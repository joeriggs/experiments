/*******************************************************************************
 * This is a very simple ODP/OFP app that proxies all data between a client and
 * server.
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	int so = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("socket() returned %d.\n", so);

	struct sockaddr_in listener_addr;
	memset(&listener_addr, 0, sizeof(listener_addr));
	listener_addr.sin_family = AF_INET;
	listener_addr.sin_port = htons(760);
	listener_addr.sin_addr.s_addr = inet_addr("172.17.0.1");
	int rc = bind(so, (struct sockaddr *) &listener_addr, sizeof(listener_addr));
	printf("bind() returned %d.\n", rc);

	while(1) {
		usleep(10000);

		struct sockaddr_in dst_addr;
		memset(&dst_addr, 0, sizeof(dst_addr));
		dst_addr.sin_family = AF_INET;
		dst_addr.sin_port = htons(2048);
		dst_addr.sin_addr.s_addr = inet_addr("172.17.0.3");

		const char *send_buf = "Hello world";
		ssize_t s = sendto(so, send_buf, strlen(send_buf), 0, (struct sockaddr *) &dst_addr, sizeof(dst_addr));
		printf("sendto() returned %ld.\n", s);

		char recv_buf[1024];
		struct sockaddr_in src_addr;
		socklen_t src_addr_len;
		src_addr_len = sizeof(src_addr);
		ssize_t r = recvfrom(so, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *) &src_addr, &src_addr_len);
		printf("recvfrom() returned %ld.\n", r);
		if(r > 0) {
			recv_buf[r] = 0;
			printf("Received '%s'.\n", recv_buf);
		}
	}

	return rc;
}

