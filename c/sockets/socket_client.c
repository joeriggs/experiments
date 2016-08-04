
#include "socket_test.h"

int main(int argc, char **argv)
{
  int retval = -1;

  printf("Socket Client\n");

  do
  {
    int ret;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    printf("socket() returned %d (%s).\n", sock, (sock >= 0) ? "PASS": "FAIL");
    if(sock < 0) break;

    struct sockaddr_in connect_info;
    connect_info.sin_family = AF_INET;
    connect_info.sin_port   = htons(50000);
    inet_aton("192.168.1.184", &connect_info.sin_addr);
    ret = connect(sock, (struct sockaddr *) &connect_info, sizeof(connect_info));
    printf("connect() returned %d (%s).\n", ret, (ret == 0) ? "PASS": "FAIL");
    if(ret != 0) { printf("%s\n", strerror(errno)); break; }

    char read_buf[1024];
    ret = read(sock, read_buf, sizeof(read_buf));
    printf("read() returned %d (%s).\n", ret, (ret >= 0) ? "PASS" : "FAIL");

    ret = close(sock);
    printf("close() returned %d (%s).\n", ret, (ret == 0) ? "PASS" : "FAIL");
    if(ret != 0) break;

    /* Success.  */
    retval = 0;

  } while(0);

  return retval;
}

