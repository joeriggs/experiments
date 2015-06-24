
#include "socket_test.h"

int main(int argc, char **argv)
{
  int retval;

  printf("Socket Client\n");

  int sock = socket(AF_INET,SOCK_STREAM, 0);
  printf("socket() returned %d (%s).\n", sock, (sock >= 0) ? "PASS": "FAIL");
  if(sock >= 0)
  {

    retval = close(sock);
    printf("close() returned %d (%s).\n", retval, (retval == 0) ? "PASS" : "FAIL");
  }

  return 0;
}

