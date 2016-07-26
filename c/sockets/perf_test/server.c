
#include "socket_test.h"

int main(int argc, char **argv)
{
  int retval = -1;

  printf("Socket Server\n");

  int ret;

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  printf("socket() returned %d (%s).\n", sock, (sock >= 0) ? "PASS": "FAIL");
  if(sock < 0) return retval;

  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  inet_aton("192.168.1.184", &sin.sin_addr);
  sin.sin_port = htons(50000);
  ret = bind(sock, (struct sockaddr *) &sin, sizeof(sin));
  printf("bind(%d) returned %d (%s).\n", ntohs(sin.sin_port), ret, (ret == 0) ? "PASS": "FAIL");

  ret = listen(sock, 10);
  printf("listen() returned %d (%s).\n", ret, (ret == 0) ? "PASS": "FAIL");
  if(ret != 0) return retval;

  struct sockaddr accept_addr;
  socklen_t       accept_addr_len = sizeof(accept_addr);
  int accept_sock = accept(sock, &accept_addr, &accept_addr_len);
  printf("accept() returned %d (%s).\n", accept_sock, (accept_sock != -1) ? "PASS": "FAIL");
  if(accept_sock == -1) return retval;

  int i;
  for(i = 0; i < 100000; i++) {
    printf("%8d\n", i);

    const char *write_msg = "This is a message from the socket_server.";

    ret = send(accept_sock, write_msg, strlen(write_msg), 0);
    printf("send() returned %d (%s).\n", ret, (ret == strlen(write_msg)) ? "PASS": "FAIL");
    if(ret != strlen(write_msg)) break;

    char read_buf[1024];
    ret = recv(accept_sock, read_buf, sizeof(read_buf), 0);
    printf("recv() returned %d.\n", ret);
    if(ret <= 0) break;
  }

  retval = close(sock);
  printf("close() returned %d (%s).\n", retval, (retval == 0) ? "PASS" : "FAIL");

  return retval;
}

