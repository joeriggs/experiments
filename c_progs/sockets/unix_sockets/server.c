
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

/* Change this flag to enable/disable non-blocking on the socket. */
#define DO_NONBLOCK 1

#define CHANNEL      "/tmp/testing.sock"

int main(int argc, char **argv)
{
	printf("This is the UNIX socket server.\n");

	/* Delete the old socket if it existed. */
	unlink(CHANNEL);

	int socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("socket() failed\n");
		return 1;
	} 

	/* start with a clean address structure */
	struct sockaddr_un address;
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, sizeof(address.sun_path), CHANNEL);

	if(bind(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0)
	{
		printf("bind() failed\n");
		return 1;
	}

	if(listen(socket_fd, 5) != 0)
	{
		printf("listen() failed\n");
		return 1;
	}

#if 1
	/* Enable this code to test the ability of the agent to block when it
	 * it calls connect().  It'll block until the server calls accept(). */
	{
		int counter;
		for(counter = 0; counter < 10; counter++) {
			printf("Sleeping ... %3d\n", counter);
			sleep(1);
		}
	}
#endif

	int connection_fd;
#if (DO_NONBLOCK == 1)
	int socket_flags = fcntl(socket_fd, F_GETFL, 0);
	socket_flags |= O_NONBLOCK;
	fcntl(socket_fd, F_SETFL, socket_flags);

	do {
		usleep(3 * 1000 * 1000);
		printf("TRYING...\n");
		socklen_t address_length = sizeof(address);
		connection_fd = accept(socket_fd, (struct sockaddr *) &address, &address_length);

		if(connection_fd > -1) {
			socket_flags = fcntl(connection_fd, F_GETFL, 0);
			socket_flags |= O_NONBLOCK;
			fcntl(connection_fd, F_SETFL, socket_flags);
		}
	} while ((connection_fd == -1) && (errno == EWOULDBLOCK));
#else
	socklen_t address_length = sizeof(address);
	connection_fd = accept(socket_fd, (struct sockaddr *) &address, &address_length);
#endif
	if(connection_fd > -1)
	{
		printf("signal returned %d.\n", signal(SIGPIPE, SIG_IGN));

		while(1) {
			/* now inside newly created connection handling process */
			int nbytes;
			char buffer[256];

			memset(buffer, 0, sizeof(buffer));
			nbytes = recv(connection_fd, buffer, 256, 0);
			printf("RX %d bytes from agent (%d).\n", nbytes, errno);
			if(nbytes <= 0) {
				printf("RX error %d (%s).\n", errno, strerror(errno));
				sleep(1);
			}

			printf("MESSAGE FROM CLIENT (%d): %s\n", nbytes, buffer);
			nbytes = snprintf(buffer, 256, "hello from the server");
			nbytes = send(connection_fd, buffer, nbytes, 0);
			printf("TX %d bytes to agent (%d).\n", nbytes, errno);
		}
	}

	close(socket_fd);
	unlink(CHANNEL);
	return 0;
}

