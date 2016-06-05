/*******************************************************************************
 * This file contains the implementation of a method that drives real life data
 * through the ssa engine.
 ******************************************************************************/

#include <stdio.h>

#include "ssa.h"
#include "ssa_data.h"

/* Actual wage history. */
typedef struct wage_history {
  int year;
  int ssa_wage; /* Social Security wages. */
  int med_wage; /* Medicare wages. */
} wage_history;
static wage_history
wages[] = {
  {    0,      0,      0 }
};
static int dob = 1950;

int
ssa_data_run(void)
{
  int retcode = 0;

  ssa_init();

  int i;
  for(i = 0; wages[i].year != 0; i++) {
    wage_history *w = &wages[i];
    ssa_add_wage(dob, w->year, w->ssa_wage);
  }

  int PIA;
  ssa_calc_benefit(dob, &PIA);
  printf("PIA = %d\n", PIA);

  return retcode;
}

