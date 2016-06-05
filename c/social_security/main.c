
#include <stdio.h>

#include "ssa.h"

#ifdef TEST
#include "ssa_test.h"
#else
#include "ssa_data.h"
#endif

int main(int argc, char **argv)
{
  int retcode = 0;

#ifdef TEST
  retcode = ssa_test();
#else
  retcode = ssa_data_run();
#endif

  return retcode;
}

