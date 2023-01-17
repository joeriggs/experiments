
#ifndef __SHM_IOCTL_H__
#define __SHM_IOCTL_H__

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include <sys/shm.h>
#include <sys/types.h>

typedef struct shmIoctlMsg {
	sem_t msgSemCmdCmplt;
	int ioctl;
	int result;
	size_t msgSize;
	unsigned char msg[1];
} shmIoctlMsg;

typedef int shmIoctlMailboxCallback(int ioctl, unsigned char *msg, size_t *msgSize, int clientPID);

typedef struct shmIoctlMailbox {
	int mailboxShmid;
	shmIoctlMailboxCallback *cb;

	pthread_mutex_t msgLock;
	sem_t msgSemCmdStart;
	int msgShmid;
} shmIoctlMailbox;

extern shmIoctlMailbox *shmIoctlMailboxOpen(const char *path, int owner, int numThreads, shmIoctlMailboxCallback *cb);
extern void shmIoctlMailboxClose(shmIoctlMailbox *mailbox);

extern shmIoctlMsg *shmIoctlMsgAllocate(key_t msgKey, size_t msgSize, int owner, int *msgShmid);
extern shmIoctlMsg *shmIoctlMsgRecv(shmIoctlMailbox *mailbox, int *msgShmid, int *clientPID);
extern int shmIoctlMsgSend(shmIoctlMailbox *mailbox, shmIoctlMsg *msg, int msgShmid);
extern int shmIoctlMsgReply(shmIoctlMsg *msg, int msgShmid);
extern void shmIoctlMsgDelete(shmIoctlMsg *msg, int msgShmid);

#endif // __SHM_IOCTL_H__
