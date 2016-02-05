
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define CHANNEL      "/var/run/testing.sock"

int main(int argc, char **argv)
{
	struct sockaddr_un address;
	int socket_fd, connection_fd;
	socklen_t address_length;
	pid_t child;
 
	printf("This is the tf_sync server.\n");

	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("socket() failed\n");
		return 1;
	} 

	unlink(CHANNEL);

	/* start with a clean address structure */
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

	if((connection_fd = accept(socket_fd, (struct sockaddr *) &address, &address_length)) > -1)
	{
		while(1) {
			/* now inside newly created connection handling process */
			int nbytes;
			char buffer[256];

			nbytes = read(connection_fd, buffer, 256);
			buffer[nbytes] = 0;

			printf("MESSAGE FROM CLIENT: %s\n", buffer);
			nbytes = snprintf(buffer, 256, "hello from the server");
			nbytes = write(connection_fd, buffer, nbytes);
		}
	}

	close(socket_fd);
	unlink(CHANNEL);
	return 0;
}

