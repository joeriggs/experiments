
#include "socket_test.h"

//#define HOSTNAME "example-go.local3.deisapp.com"
//#define HOSTNAME "localhost.localdomain"
#define HOSTNAME "google.com"

static void
print_addrinfo(struct addrinfo *i)
{
  char flags[1024] = { 0 };
  if(i->ai_flags & AI_ADDRCONFIG) strcat(flags, "AI_ADDRCONFIG ");
  if(i->ai_flags & AI_PASSIVE)    strcat(flags, "AI_PASSIVE ");
  if(i->ai_flags & AI_V4MAPPED)   strcat(flags, "AI_V4MAPPED ");
  printf("flags (%s): ", flags);

  const char *family;
  if(     i->ai_family == AF_INET)   family = (const char *) "AF_INET";
  else if(i->ai_family == AF_INET6)  family = (const char *) "AF_INET6";
  else if(i->ai_family == AF_UNSPEC) family = (const char *) "AF_UNSPEC";
  else                               family = (const char *) "UNKNOWN";
  printf("%s: ", family);

  const char *socktype;
  if(     i->ai_socktype == SOCK_STREAM) socktype = (const char *) "SOCK_STREAM";
  else if(i->ai_socktype == SOCK_DGRAM)  socktype = (const char *) "SOCK_DGRAM";
  else if(i->ai_socktype == SOCK_RAW)    socktype = (const char *) "SOCK_RAW";
  else { static char buf[16]; sprintf(buf, "%d", i->ai_socktype); socktype = buf; }
  printf("%s: ", socktype);

  const char *protocol;
  if(     i->ai_protocol == IPPROTO_IP)  protocol = (const char *) "IP";
  else if(i->ai_protocol == IPPROTO_TCP) protocol = (const char *) "TCP";
  else if(i->ai_protocol == IPPROTO_UDP) protocol = (const char *) "UDP";
  else { static char buf[16]; sprintf(buf, "%d", i->ai_protocol); protocol = buf; }
  printf("%s: ", protocol);

  char hostname[NI_MAXHOST];
  getnameinfo(i->ai_addr, i->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0); 
  if(*hostname != '\0')
    printf(" %s", hostname);

  if(i->ai_family == AF_INET)
  {
    struct sockaddr_in *sin = (struct sockaddr_in *) i->ai_addr;
    printf(" (%s) Port %d", inet_ntoa(sin->sin_addr), ntohs(sin->sin_port));
  }
  else
  {
    struct sockaddr_in6 *sin = (struct sockaddr_in6 *) i->ai_addr;
    char i6_addr[INET6_ADDRSTRLEN];
    printf(" (%s) Port %d", inet_ntop(AF_INET6, &sin->sin6_addr, i6_addr, sizeof(*sin)), ntohs(sin->sin6_port));
  }


  if(i->ai_canonname != (char *) 0) printf(": %s", i->ai_canonname);

  printf(".\n");
}

int main(int argc, char **argv)
{
  int retval = -1;

  printf("Socket Server\n");

  do
  {
    int ret;

    struct addrinfo *info;
    ret = getaddrinfo(HOSTNAME, "80", NULL, &info);
    printf("getaddrinfo() returned %d (%s).\n", ret, (ret >= 0) ? "PASS": "FAIL");
    if(ret != 0) break;
    struct addrinfo *i = info;
    while(i != (struct addrinfo *) 0)
    {
      print_addrinfo(i);
      i = i->ai_next;
    }
 
    int sock = socket(AF_INET,SOCK_STREAM, 0);
    printf("socket() returned %d (%s).\n", sock, (sock >= 0) ? "PASS": "FAIL");
    if(sock < 0) break;

    ret = bind(sock, info->ai_addr, sizeof(*info->ai_addr));
    printf("bind() returned %d (%s).\n", ret, (ret == 0) ? "PASS": "FAIL");
    if(ret != 0) break;

    ret = listen(sock, 10);
    printf("listen() returned %d (%s).\n", ret, (ret == 0) ? "PASS": "FAIL");
    if(ret != 0) break;

    ret = close(sock);
    printf("close() returned %d (%s).\n", ret, (ret == 0) ? "PASS" : "FAIL");
    if(ret != 0) break;

    freeaddrinfo(info);

    /* Success.  */
    retval = 0;

  } while(0);

  return retval;
}

