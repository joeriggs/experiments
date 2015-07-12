/* This is the main entrypoint for the calculator UI and test program.
 */

#include "common.h"
#include "test.h"
#include "ui.h"

int main(int argc, char **argv)
{
  int retcode;

#ifdef TEST
  retcode = test();
#else
  retcode = ui();
#endif

  return retcode;
}

