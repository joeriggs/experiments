
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

/* Change this flag to enable/disable non-blocking on the socket. */
#define DO_NONBLOCK 1

#define CHANNEL      "/tmp/testing.sock"

int main(int argc, char **argv)
{
	struct sockaddr_un address;
	int  socket_fd, nbytes;
	char buffer[256];

	printf("This is the UNIX socket client.\n");

	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("socket() failed\n");
		return 1;
	}

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));
 
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, sizeof(address.sun_path), CHANNEL);

#if (DO_NONBLOCK == 1)
	int socket_flags = fcntl(socket_fd, F_GETFL, 0);
	socket_flags |= O_NONBLOCK;
	fcntl(socket_fd, F_SETFL, socket_flags);

	int retcode;
	do {
		printf("TRYING...\n");
		socklen_t address_length = sizeof(address);
		retcode = connect(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un));
		if(retcode == -1) {
			printf("connect() returned %d (%s).\n", errno, strerror(errno));
			usleep(1 * 1000 * 1000);
		}

	} while ((retcode == -1) && ((errno == ENOENT) || (errno == ECONNREFUSED)));
	if(retcode != 0)
#else
	if(connect(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0)
#endif
	{
		printf("connect() failed\n");
		return 1;
	}

	int i = 0;
	while(i < 10) {
		nbytes = snprintf(buffer, 256, "hello from a client");
		nbytes = send(socket_fd, buffer, nbytes, 0);
		printf("TX %d bytes to server (%d).\n", nbytes, errno);
 
		do {
			memset(buffer, 0, sizeof(buffer));
			nbytes = recv(socket_fd, buffer, 256, 0);
			if(nbytes >= 0) {
				printf("RX %d bytes from server: %s\n", nbytes, buffer);
				i++;
			}
			else {
				printf("recv() failed (%d : %s).\n", errno, strerror(errno));
				usleep(500 * 1000);
			}
		} while(nbytes == -1);
	}

	close(socket_fd);

	return 0;
}

