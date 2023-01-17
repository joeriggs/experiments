
#include <curses.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"

static int maxOps = 1000000;

/*******************************************************************************
 *******************************************************************************
 ************ Display some fancy statistics on the server's console ************
 *******************************************************************************
 ******************************************************************************/
static WINDOW *mainWin;

static void statsCalc(int counter)
{
	if ((counter % 10000) == 0) {
		char str[1024];
		int row = 3;
		attron(A_BOLD);
		mvaddstr(row, 1, "Number of requests:");
		attroff(A_BOLD);

		sprintf(str, "%'15d", counter);
		mvaddstr(row, 30, str);

		refresh();
	}
}

static void statsInit(void)
{
	char str[64];

	setlocale(LC_NUMERIC, "en_US");

	mainWin = initscr();
	cbreak(); // Don't wait for <CR> to process characters.
	noecho(); // Don't echo input characters.
	nodelay(mainWin, true);

	attron(A_BOLD);
	const char *clientPIDHdr = "Client PID: ";
	mvaddstr(1, 1, clientPIDHdr);
	attroff(A_BOLD);

	sprintf(str, "%d", getpid());
	mvaddstr(1, strlen(clientPIDHdr) + 2, str);

	attron(A_BOLD);
	mvaddstr(2, 1, "Test iterations:");
	attroff(A_BOLD);

	sprintf(str, "%'15d", maxOps);
	mvaddstr(2, 30, str);
}

static void statsSummary(int counter)
{
	char str[1024];

	if (counter > maxOps)
		sprintf(str, "%s", "Test completed successfully.");
	else
		sprintf(str, "Test failed.  %d test iterations ran.", counter);

	attron(A_BOLD);
	mvaddstr(4, 1, str);
	attroff(A_BOLD);
	refresh();
	sleep(10);

	endwin();

	printf("\n%s\n", str);
}

/*******************************************************************************
 *******************************************************************************
 *********************** End of the fancy statistics code **********************
 *******************************************************************************
 ******************************************************************************/

int main(int argc, char **argv)
{
	// The user can specify the number of operations to perform during this test.
	if (argc > 1)
		maxOps = atoi(argv[1]);

	// We'll display some fancy stats on the screen during the test.
	// Initialize the statistics processor.
	statsInit();

	// Get a pointer to the server's mailbox.
	shmIoctlMailbox *mailbox = shmIoctlMailboxOpen(SHM_PATH, 0, 0, NULL);
	if (!mailbox) {
		printf("shmIoctlMailboxOpen() failed.\n");
		return 1;
	}

	// Allocate a msg object.  We use it to pass ioctl calls to the server.
	// We use our own PID to create the "key" that is used for the requests
	// that we send to the server.
	key_t shmKey = (key_t) getpid();
	int shmid;
	shmIoctlMsg *msg = shmIoctlMsgAllocate(shmKey, 100, 1, &shmid);
	if (!msg) {
		printf("shmIoctlMsgAllocate() failed.\n");
		return 1;
	}

	int counter;
	for(counter = 1; counter <= maxOps; counter++) {

		// Update our test program stats.
		statsCalc(counter);

		// =============================================================
		// PREPARE THE IOCTL MESSAGE.  STORE THE DATA AT:
		//   msg->ioctl   = The ioctl code.
		//   msg->msg     = The message.
		//   msg->msgSize = The size of the message.
		// =============================================================
		msg->ioctl = 67890;
		int *intPtr = (int *) &msg->msg[0];
		*intPtr = counter;
		msg->msgSize = sizeof(int);

		// Send the ioctl call to the server.  Block until it either completes
		// or times out.
		if (shmIoctlMsgSend(mailbox, msg, shmid) == -1) {
			// An error means there was a system failure (probably a
			// timeout).  It has nothing to do with whether the ioctl
			// was successful.
			printf("shmIoctlMsgSend(%p %d) failed.\n", mailbox, shmid);
			break;
		}

		// =============================================================
		// THE CALL RETURNED.  THE RESULTS ARE STORED AT:
		//   msg->result  = The return code.
		//   msg->msg     = Any data returned from the server.
		//   msg->msgSize = The size of the reply.
		// =============================================================

	}

	statsSummary(counter);

	// Delete the message.
	shmIoctlMsgDelete(msg, shmid);

	return 0;
}

