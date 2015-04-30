
/******************************************************************************
 * This program will execute the system() function to run a command.
 *****************************************************************************/

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
  printf("Testing system().\n");

  int rc = system("ls -l main.c");
  printf("The command returned %d.\n", rc);

  return rc;
}

