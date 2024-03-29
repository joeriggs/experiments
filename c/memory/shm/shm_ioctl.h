
#ifndef __SHM_IOCTL_H__
#define __SHM_IOCTL_H__

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include <sys/shm.h>
#include <sys/types.h>

// Each client creates an shmIoctlMsg object.  They use the shmIoctlMsg object to
// pass their ioctl() calls to their server(s) via the server's shmIoctlMailbox
// object.
typedef struct shmIoctlMsg {
	int msgShmid;
	sem_t msgSemCmdCmplt;
	int opcode;
	int result;
	size_t msgSize;
	unsigned char msg[1];
} shmIoctlMsg;

// Each server has a callback function that will receive incoming ioctl() calls.
typedef int shmIoctlMailboxCallback(int opcode, unsigned char *msg, size_t *msgSize, int clientPID);

typedef struct shmIoctlMailbox {
	int initialized;

	int mailboxShmid;
	key_t mailboxKey;
	shmIoctlMailboxCallback *cb;

	int doClose;
	int numThreads;
	pthread_t *threadIDs;

	pthread_mutex_t msgLock;
	sem_t msgSemCmdStart;
	int msgShmid;
} shmIoctlMailbox;

extern shmIoctlMailbox *shmIoctlMailboxOpen(const char *path, int owner, int numThreads, shmIoctlMailboxCallback *cb);
extern void shmIoctlMailboxClose(shmIoctlMailbox *mailbox);

extern shmIoctlMsg *shmIoctlMsgAllocate(key_t msgKey, size_t msgSize, int owner);
extern shmIoctlMsg *shmIoctlMsgRecv(shmIoctlMailbox *mailbox, int *clientPID);
extern int shmIoctlMsgSend(shmIoctlMailbox *mailbox, shmIoctlMsg *msg);
extern int shmIoctlMsgReply(shmIoctlMsg *msg);
extern void shmIoctlMsgDelete(shmIoctlMsg *msg);

#endif // __SHM_IOCTL_H__
