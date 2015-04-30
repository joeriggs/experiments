
/******************************************************************************
 * This program will sit in a loop and read a file over and over again.  It's a
 * simple read/seek/read/seek/read/seek test.
 *****************************************************************************/

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FILENAME "./file.txt"
static unsigned char buf[1024 * 1024];

int
main(int argc, char **argv)
{
  int rc;
  char *result;

  printf("    fopen(): ");
  FILE *fp = fopen(FILENAME, "r");
  rc     = (fp != 0) ?      0 :      1;
  result = (rc == 0) ? "PASS" : "FAIL";
  printf("%s.\n", result);

  if(fp != 0) {
    /* Get file size.  Then allocate a buffer to be used for reading. */
    printf("    fseek(): ");
    rc = fseek(fp, 0L, SEEK_END);
    result = (rc == 0) ? "PASS" : "FAIL";
    printf("%s.\n", result);

    printf("    ftell(): ");
    int file_size = ftell(fp);
    rc     = (file_size != -1) ?      0 :      1;
    result = (rc == 0)         ? "PASS" : "FAIL";
    if(rc == 0) {
      printf("%s.  File is %d bytes.\n", result, file_size);
    }
    else {
      printf("%s.\n", result);
    }

    while(rc == 0) {
      printf("    fseek(): ");
      rc = fseek(fp, 0, SEEK_SET);
      result = (rc == 0) ? "PASS" : "FAIL";
      printf("%s.\n", result);

      if(rc == 0) {
        printf("    fread(): ");
        int read_size = (file_size < sizeof(buf)) ? file_size : sizeof(buf);
        int j = fread(buf, read_size, 1, fp);
        printf("fread(%d) returned %d.\n", read_size, j);
        if(j != 1) {
          rc = 1;
        }
        result = (rc == 0) ? "PASS" : "FAIL";
        printf("%s.\n", result);
      }
    }

    /* We shouldn't drop out of the loop, but just in case... */
    if(fp != (FILE *) 0) {
      printf("    fclose(): ");
      rc = fclose(fp);
      result = (rc == 0) ? "PASS" : "FAIL";
      printf("%s.\n", result);
    }
  }

  return rc;
}

