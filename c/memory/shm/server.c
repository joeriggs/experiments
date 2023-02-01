
#include <curses.h>
#include <locale.h>
#include <string.h>

#include "test.h"

static shmIoctlMailbox *mailbox = NULL;

static int exitProgram = 0;

static pthread_mutex_t statsMutex;

static pthread_t    tidList[NUM_SERVER_THREADS];
static int          tidCounter[NUM_SERVER_THREADS];
static __thread int tidStatsIndex = -1;

static pid_t        pidList[NUM_CLIENT_PROCESSES];
static int          pidCounter[NUM_CLIENT_PROCESSES];

/*******************************************************************************
 *******************************************************************************
 ************ Display some fancy statistics on the server's console ************
 *******************************************************************************
 ******************************************************************************/

static WINDOW *mainWin;
static pthread_t statsThreadID = 0;
static void *statsThread(void *arg)
{
	int numThreads = 0;
	int numProcesses = 0;
	int clearScreen = 1;

	setlocale(LC_NUMERIC, "en_US");

	mainWin = initscr();
	cbreak(); // Don't wait for <CR> to process characters.
	noecho(); // Don't echo input characters.
	nodelay(mainWin, true);

	while(!exitProgram) {
		if (clearScreen)
			clear();
		clearScreen = 0;

		char str[1024];
		int row = 1;

		// Print some stuff;
		attron(A_BOLD);
		mvaddstr(row, 1, "Key: ");
		attroff(A_BOLD);
		if (mailbox)
			sprintf(str, "%x", mailbox->mailboxKey);
		else
			sprintf(str, "N/A");
		mvaddstr(row, 6, str);
		row++;

		attron(A_BOLD);
		mvaddstr(row, 1, "shmid: ");
		attroff(A_BOLD);
		if (mailbox)
			sprintf(str, "%d", mailbox->mailboxShmid);
		else
			sprintf(str, "N/A");
		mvaddstr(row, 8, str);
		row += 2;

		// *************************************************************
		// *************************************************************
		// Display stats for each thread.
		// *************************************************************
		// *************************************************************

		// Print a column header.
		attron(A_BOLD);
		sprintf(str, "Number                 ID       Callbacks");
		mvaddstr(row++, 1, str);
		sprintf(str, "Thread             Thread       Number of");
		mvaddstr(row++, 1, str);
		attroff(A_BOLD);

		int i;
		int total;
		for(i = 0, total = 0; i < NUM_SERVER_THREADS; i++) {
			if (tidList[i]) {
#ifdef __FREEBSD__
				sprintf(str, "%6d   %16p      %'10d", i, tidList[i], tidCounter[i]);
#else
				sprintf(str, "%6d   %16lx      %'10d", i, tidList[i], tidCounter[i]);
#endif
				mvaddstr(row++, 1, str);

				total += tidCounter[i];
			}
		}
		attron(A_BOLD);
		mvaddstr(row++, 30, "____________");
		mvaddstr(row,   20, "Total");
		attroff(A_BOLD);

		sprintf(str, "%'12d\n", total);
		mvaddstr(row++, 30, str);

		if (numThreads != i)
			clearScreen = 1;

		// *************************************************************
		// *************************************************************
		// Display stats for client processes (max NUM_CLIENT_PROCESSES
		// processes).
		// *************************************************************
		// *************************************************************
		row+=2;

		// Print a column header.
		attron(A_BOLD);
		sprintf(str, "Client             CLient       Number of");
		mvaddstr(row++, 1, str);
		sprintf(str, "Process               PID       Callbacks");
		mvaddstr(row++, 1, str);
		attroff(A_BOLD);

		for(i = 0, total = 0; i < NUM_CLIENT_PROCESSES; i++) {
			if (pidList[i] != 0) {
				sprintf(str, "%6d   %16d      %'10d", i, pidList[i], pidCounter[i]);
				mvaddstr(row++, 1, str);
				total += pidCounter[i];
			}
		}
		attron(A_BOLD);
		mvaddstr(row++, 30, "____________");
		mvaddstr(row,   20, "Total");
		attroff(A_BOLD);

		sprintf(str, "%'12d\n", total);
		mvaddstr(row++, 30, str);

		if (numProcesses != i)
			clearScreen = 1;

		refresh();
		sleep(3);

		int ch = wgetch(mainWin);
		if((ch == 'x') || (ch == 'X')) {
			exitProgram = 1;
		}
	}

	pthread_join(statsThreadID, NULL);

	endwin();
	return NULL;
}

static void statsInit(void)
{
	// Initialize the counter storage.
	int i;
	for(i = 0; i < NUM_SERVER_THREADS; i++) {
		tidList[i] = 0;
		tidCounter[i] = 0;
	}
	for(i = 0; i < NUM_CLIENT_PROCESSES; i++) {
		pidList[i] = 0;
		pidCounter[i] = 0;
	}

	pthread_mutexattr_t mutexAttr;
        pthread_mutexattr_init(&mutexAttr);
        pthread_mutex_init(&statsMutex, &mutexAttr);

	// Start the stats thread.
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&statsThreadID, &attr, statsThread, NULL);
}

/*******************************************************************************
 *******************************************************************************
 *********************** End of the fancy statistics code **********************
 *******************************************************************************
 ******************************************************************************/

/* Update the stats each time our callback function is called.
 */
static void statsCalc(int clientPID)
{
	// Keep some stats for each callback thread.
	if (tidStatsIndex == -1) {
		// Find an unused slot in the TID stats arrays.  Save the index
		// in tidStatsIndex.
		int i;
		pthread_mutex_lock(&statsMutex);
		for(i = 0; i < NUM_SERVER_THREADS; i++) {
			if (tidList[i] == 0) {
				tidList[i] = pthread_self();
				tidStatsIndex = i;
				break;
			}
		}
		pthread_mutex_unlock(&statsMutex);
	}
	if (tidStatsIndex != -1) {
		tidCounter[tidStatsIndex]++;
	}

	// Keep stats for each client process.
	int i;
	pthread_mutex_lock(&statsMutex);
	for(i = 0; i < NUM_CLIENT_PROCESSES; i++) {
		if (pidList[i] == clientPID) {
			break;
		}
	}
	if (i >= NUM_CLIENT_PROCESSES) {
		for(i = 0; i < NUM_CLIENT_PROCESSES; i++) {
			if (pidList[i] == 0) {
				pidList[i] = clientPID;
				break;
			}
		}
	}

	if (i < NUM_CLIENT_PROCESSES) {
		pidCounter[i]++;
	}
	pthread_mutex_unlock(&statsMutex);
}

/* This is the callback function that processes the client's ioctl calls.
 *
 * Input:
 *   ioctl   = The ioctl code.
 *   msg     = The message data.
 *   msgSize = The size of the msg.
 *   clientPID    = The PID of the caller.
 *
 * Return:
 *   msg     = The data returned to the client.
 *   msgSize = The size of the reply.
 *   result  = The return code.
 */
static int serverCallback(int ioctl, unsigned char *msg, size_t *msgSize, int clientPID)
{
	int result = -1;

	// Keep some stats.
	statsCalc(clientPID);

	// Create a reply.
	char *msgBuf = (char *) msg;
	sprintf(msgBuf, "This is the reply from server [%d].", getpid());
	*msgSize = strlen(msgBuf) + 1;

	// Success.
	result = 0;

	return result;
}

int main(int argc, char **argv)
{
	// Initialize some code that will display statistics during the test.
	statsInit();

	// Create a mailbox object.  It'll recv calls from the clients, and it will
	// pass the calls to the specified callback function.
	mailbox = shmIoctlMailboxOpen(SHM_PATH, 1, NUM_SERVER_THREADS, serverCallback);
	if (!mailbox) {
		printf("shmIoctlMailboxOpen() failed.\n");
		return 1;
	}

	while (!exitProgram) {
		sleep(1);
	}

	shmIoctlMailboxClose(mailbox);

	return 0;
}

