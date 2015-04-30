
/******************************************************************************
 * This program will create, write, and close a file.  Nothing fancy.  Just a
 * useful tool for testing/experimenting with file streams.
 *****************************************************************************/

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FILENAME "./file.test"

int
main(int argc, char **argv)
{
  int rc = 0;
  const char *result;
  
  const char *filename = FILENAME;
  printf("  Creating %s.\n", filename);

  printf("    fopen(): ");
  FILE *fp = fopen(filename, "w");
  rc     = (fp != 0) ?      0 :      1;
  result = (rc == 0) ? "PASS" : "FAIL";
  printf("%s.\n", result);

  if(rc == 0) {
    int count;
    for(count = 0; count < 1024; count++) {
      char buf[64];
      snprintf(buf, sizeof(buf), "%06d: %06d\n", count, count);
      printf("    fwrite(): %6d: ", count);
      if(fwrite(buf, strlen(buf), 1, fp) != 1) {
        rc = 1;
      }
      result = (rc == 0) ? "PASS" : "FAIL";
      printf("%s.\n", result);
    }
  }

  if(fp != (FILE *) 0) {
    printf("    fclose(): ");
    rc = fclose(fp);
    result = (rc == 0) ? "PASS" : "FAIL";
    printf("%s.\n", result);
  }

  return rc;
}

