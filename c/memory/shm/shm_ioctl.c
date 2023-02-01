
#include <stdio.h>
#include <stdlib.h>

#include "shm_ioctl.h"

/* This is a thread function.  The server can request that more than one thread be
 * started.  Each thread runs this function.
 */
static void *shmIoctlMailboxThreadFunc(void *arg)
{
	shmIoctlMailbox *mailbox = (shmIoctlMailbox *) arg;
	shmIoctlMailboxCallback *cb = mailbox->cb;

	while(1) {
		// Get the next message from the mailbox.
		int clientPID;
		shmIoctlMsg *msg = shmIoctlMsgRecv(mailbox, &clientPID);

		// Are we closing?
		if (mailbox->doClose)
			break;

		// Pass the message to the server's callback function.
		msg->result = (*cb)(msg->opcode, msg->msg, &msg->msgSize, clientPID);

		// Send the reply back to the client.
		if (shmIoctlMsgReply(msg) == -1) {
			printf("shmIoctlMsgReply(%p) failed.\n", msg);
		}
	}

	return NULL;
}

/* Create a shared memory mailbox that allows a client to pass messages to a
 * server
 */
shmIoctlMailbox *shmIoctlMailboxOpen(const char *shmPath, int owner, int numThreads, shmIoctlMailboxCallback *cb)
{
	shmIoctlMailbox *mailbox = NULL;

	key_t my_key = ftok(shmPath, 'W');

	int shmFlag = 0666 | (owner ? IPC_CREAT : 0);

	int shmid = shmget(my_key, sizeof(shmIoctlMailbox), shmFlag);

	if (shmid == -1) {
#ifdef __FREEBSD__
		printf("shmget(%lx, %ld, %o) failed (%m).\n", my_key, sizeof(shmIoctlMailbox), shmFlag);
#else
		printf("shmget(%x, %ld, %o) failed (%m).\n", my_key, sizeof(shmIoctlMailbox), shmFlag);
#endif
		return NULL;
	}

	mailbox = (shmIoctlMailbox *) shmat(shmid, 0, 0);
	if (mailbox == (shmIoctlMailbox *) -1) {
		printf("shmat(%d, 0, 0) failed (%m).\n", shmid);
		return NULL;
	}

	// Store some necessary stuff.
	mailbox->mailboxShmid = shmid;
	mailbox->mailboxKey = my_key;
	mailbox->cb = cb;

	/* PTHREAD_MUTEX_INITIALIZER */
	pthread_mutexattr_t attr;
	if (pthread_mutexattr_init(&attr)) {
		printf("STAGE 1: Unable to initialize the Lock attributes.");
		return NULL;
	} else if(pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) {
		printf("STAGE 2: Unable to initialize the Lock attributes.");
		return NULL;
	} else if(pthread_mutex_init(&mailbox->msgLock, &attr)) {
		printf("STAGE 3: Unable to initialize the Lock attributes.");
		return NULL;
	}

	if (owner) {
		sem_t *semAddr = &mailbox->msgSemCmdStart;
		if(sem_init(semAddr, 1, 0) == -1) {
			printf("%s(): sem_init(%p, 1, 0) failed (%m).", __FUNCTION__, semAddr);
			return NULL;
		}

		// Save all of the threadIDs.  We want to make sure they're all shutdown
		// before we delete the mailbox.
		mailbox->numThreads = numThreads;
		mailbox->threadIDs = (pthread_t *) malloc(sizeof(pthread_t) * numThreads);

		// Start all of the requested threads.
		int i;
		for(i = 0; i < numThreads; i++) {
			pthread_attr_t attr;
			if(pthread_attr_init(&attr)) {
				printf("%s(): pthread_attr_init() failed (%m).", __FUNCTION__);
				return NULL;
			}

			if(pthread_create(&mailbox->threadIDs[i], &attr, shmIoctlMailboxThreadFunc, mailbox) != 0) {
				printf("%s(): pthread_create() failed (%m).", __FUNCTION__);
				return NULL;
			}
		}
	}

	return mailbox;
}

/* Delete the shared memory mailbox.
 */
void shmIoctlMailboxClose(shmIoctlMailbox *mailbox)
{
	int shmid = mailbox->mailboxShmid;

	// Get the statistics for shmid.  We'll extract the creator's PID from there.
	struct shmid_ds buf;
	int retcode = shmctl(shmid, IPC_STAT, &buf);
	if (retcode == -1) {
		printf("%s(): shmctl(%d, IPC_STAT, %p) failed (%m).\n", __FUNCTION__, shmid, &buf);
		return;
	}
	int creatorPID = buf.shm_cpid;

	if (getpid() == creatorPID) {
		mailbox->doClose = 1;

		int i;
		for(i = 0; i < mailbox->numThreads; i++) {
			sem_post(&mailbox->msgSemCmdStart);
		}
		for(i = 0; i < mailbox->numThreads; i++) {
			pthread_join(mailbox->threadIDs[i], NULL);
		}
	}

	if (shmdt(mailbox) == -1) {
		printf("shmdt(%p) failed (%m).\n", mailbox);
	}

	if (getpid() == creatorPID) {
		if (shmctl(shmid, IPC_RMID, NULL) == -1) {
			printf("%s(): shmctl(%d, IPC_RMID, NULL) failed (%m).\n", __FUNCTION__, shmid);
		}
	}
}

/* Create a message that can be passed from the client to the server via the
 * server's mailbox.
 */
shmIoctlMsg *shmIoctlMsgAllocate(key_t msgKey, size_t size, int owner)
{
	shmIoctlMsg *msg = NULL;

	int shmFlag = 0666 | (owner ? IPC_CREAT : 0);
	size_t msgSize = sizeof(shmIoctlMsg) + size;

	int shmid = shmget(msgKey, msgSize, shmFlag);
	if (shmid == -1) {
#ifdef __FREEBSD__
		printf("shmget(%lx, %ld, %o) failed (%m).\n", msgKey, msgSize, shmFlag);
#else
		printf("shmget(%x, %ld, %o) failed (%m).\n", msgKey, msgSize, shmFlag);
#endif
		return NULL;
	}

	msg = (shmIoctlMsg *) shmat(shmid, 0, 0);
	if (msg == (shmIoctlMsg *) -1) {
		printf("shmat(%d, 0, 0) failed (%m).\n", shmid);
		msg = NULL;
	}

	if (msg) {
		sem_t *semAddr = &msg->msgSemCmdCmplt;
		if(sem_init(semAddr, 1, 0) == -1) {
			printf("sem_init(%p, 1, 0) failed (%m).", semAddr);
		}
	}

	msg->msgShmid = shmid;
	msg->msgSize = msgSize;

	return msg;
}

/* Wait for a new message to arrive in the specified mailbox.  Then grab the
 * message.
 */
shmIoctlMsg *shmIoctlMsgRecv(shmIoctlMailbox *mailbox, int *clientPID)
{
	shmIoctlMsg *msg = NULL;

	// Wait for a client to send us a message.
	if (sem_wait(&mailbox->msgSemCmdStart) == -1) {
		printf("%s(): sem_wait(%p) failed (%m).\n", __FUNCTION__, &mailbox->msgSemCmdStart);
		return msg;
	}

	// If we're shutting down, return immediately.
	if (mailbox->doClose)
		return msg;

	// Get the message ID.  Remove it from the mailbox ASAP.
	__sync_synchronize();
	int shmid = mailbox->msgShmid;

	// We removed the message ID from the mailbox.  Release the
	// mutex so the mailbox can receive another message.
	pthread_mutex_unlock(&mailbox->msgLock);

	// Get the statistics for shmid.  We'll extract the caller's PID from there.
	struct shmid_ds buf;
	int retcode = shmctl(shmid, IPC_STAT, &buf);
	if (retcode == -1) {
		printf("%s(): shmctl(%d, IPC_STAT, %p) failed (%m).\n", __FUNCTION__, shmid, &buf);
		*clientPID = -1;
	}
	else
		*clientPID = buf.shm_cpid;

	// Get a pointer to the client's message.
	msg = (shmIoctlMsg *) shmat(shmid, 0, 0);
	if (msg == (shmIoctlMsg *) -1) {
		printf("%s(): shmat(%d, 0, 0) failed (%m).\n", __FUNCTION__, shmid);
		return msg;
	}

	return msg;
}

int shmIoctlMsgSend(shmIoctlMailbox *mailbox, shmIoctlMsg *msg)
{
	int retcode = -1;

	// Grab the server's mailbox mutex.  Then place our message
	// into the server's mailbox.
	pthread_mutex_lock(&mailbox->msgLock);
	mailbox->msgShmid = msg->msgShmid;
	__sync_synchronize();

	// Tell the server to process our message.
	if (sem_post(&mailbox->msgSemCmdStart) == -1) {
		printf("sem_post(%p) failed (%m).\n", &mailbox->msgSemCmdStart);
		return retcode;
	}

	// Wait for the server to finish our message.
	struct timespec timer;
	clock_gettime(CLOCK_REALTIME, &timer);
	timer.tv_sec += 60;
	if (sem_timedwait(&msg->msgSemCmdCmplt, &timer) == -1) {
		printf("sem_timedwait(%p, %p) failed (%m).\n", &msg->msgSemCmdCmplt, &timer);
		return retcode;
	}

	retcode = msg->result;

	return retcode;
}

int shmIoctlMsgReply(shmIoctlMsg *msg)
{
	int retcode = -1;

	// Wake up the client so it can process the result.
	if (sem_post(&msg->msgSemCmdCmplt) == -1) {
		printf("%s(): sem_post(%p failed (%m).\n", __FUNCTION__, &msg->msgSemCmdCmplt);
		return retcode;
	}

	// Delete our handle to the message.
	shmIoctlMsgDelete(msg);

	retcode = 0;

	return retcode;
}

/* Delete the message.
 */
void shmIoctlMsgDelete(shmIoctlMsg *msg)
{
	pid_t my_pid = getpid();

	int shmid = msg->msgShmid;
	struct shmid_ds buf;
	if (shmctl(shmid, IPC_STAT, &buf) == -1) {
		printf("shmctl(%d, IPC_STAT, %p) failed (%m).\n", shmid, &buf);
		return;
	}

	if (shmdt(msg) == -1) {
		printf("shmdt(%p) failed (%m).\n", msg);
	}

	if (buf.shm_cpid == my_pid) {
		if (shmctl(shmid, IPC_RMID, NULL) == -1) {
			printf("%s(): shmctl(%d, IPC_RMID, NULL) failed (%m).\n", __FUNCTION__, shmid);
		}
	}
}

