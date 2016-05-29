/*******************************************************************************
 * This file contains the implementation of the algorithms that are used to
 * calculate or estimate the Social Security benefits for a retiree.
 ******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "ssa.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

#define TOTAL_HIGHEST_INDEXED_EARNINGS 35

/* Average Wage Index (a.k.a. AWI) Table, taken from:
 *   https://www.ssa.gov/oact/COLA/AWI.html
 *
 * Dates after 2014 are estimated using data from this page:
 *   https://www.ssa.gov/oact/TR/TRassum.html
 *
 * A description of how to use this table can be found at:
 *   https://www.ssa.gov/oact/ProgData/retirebenefit1.html
 *
 * Here's a simple description of how the AWI Table is used:
 * - The AWI Table contains the AWI for each year, going back to 1951.
 * - It is used to calculate the "indexing factor" for each year that a person
 *   worked and earned wages.
 * - The "indexing factor" for year xxxx is calculated by dividing the AWI for
 *   the year the worker turned 60 by the AWI for year xxxx.
 * - The "indexing factor" is multiplied by their actual earnings (a.k.a. their
 *   nominal earnings) for year xxxx to calculate their "indexed earnings" for
 *   that year.
 * - The highest 35 "indexed earnings" are added together to produce the
 *   "highest-35 total".
 * - The "highest-35 total" is divided by 420 to produce their "Average
 *   Indexed Monthly Earnings" (AIME).  FYI, there are 420 months in 35 years.
 *
 * Here is an example:
 * - The worker was born in 1954.  We need to calculate the "indexing factor"
 *   for this person for the year 1976.
 * - They were born in 1954, which means they turn 60 in 2014.
 * - AWI(2014) = 46,481.52
 * - AWI(1976) =  9,226.48
 * - Indexing Factor = 46,481.52 / 9,226.48 = 5.0378.
 * - Indexed Earnings for 1976 for this worker = Nominal_Earnings * 5.0378
 */
typedef struct average_wage_index {
  int year;
  float index;
} average_wage_index;

static average_wage_index
awi[] = {
  { 1951,   2799.16 },
  { 1952,   2973.32 },
  { 1953,   3139.44 },
  { 1954,   3155.64 },
  { 1955,   3301.44 },
  { 1956,   3532.36 },
  { 1957,   3641.72 },
  { 1958,   3673.80 },
  { 1959,   3855.80 },
  { 1960,   4007.12 },
  { 1961,   4086.76 },
  { 1962,   4291.40 },
  { 1963,   4396.64 },
  { 1964,   4576.32 },
  { 1965,   4658.72 },
  { 1966,   4938.36 },
  { 1967,   5213.44 },
  { 1968,   5571.76 },
  { 1969,   5893.76 },
  { 1970,   6186.24 },
  { 1971,   6497.08 },
  { 1972,   7133.80 },
  { 1973,   7580.16 },
  { 1974,   8030.76 },
  { 1975,   8630.92 },
  { 1976,   9226.48 },
  { 1977,   9779.44 },
  { 1978,  10556.03 },
  { 1979,  11479.46 },
  { 1980,  12513.46 },
  { 1981,  13773.10 },
  { 1982,  14531.34 },
  { 1983,  15239.24 },
  { 1984,  16135.07 },
  { 1985,  16822.51 },
  { 1986,  17321.82 },
  { 1987,  18426.51 },
  { 1988,  19334.04 },
  { 1989,  20099.55 },
  { 1990,  21027.98 },
  { 1991,  21811.60 },
  { 1992,  22935.42 },
  { 1993,  23132.67 },
  { 1994,  23753.53 },
  { 1995,  24705.66 },
  { 1996,  25913.90 },
  { 1997,  27426.00 },
  { 1998,  28861.44 },
  { 1999,  30469.84 },
  { 2000,  32154.82 },
  { 2001,  32921.92 },
  { 2002,  33252.09 },
  { 2003,  34064.95 },
  { 2004,  35648.55 },
  { 2005,  36952.94 },
  { 2006,  38651.41 },
  { 2007,  40405.48 },
  { 2008,  41334.97 },
  { 2009,  40711.61 },
  { 2010,  41673.83 },
  { 2011,  42979.61 },
  { 2012,  44321.67 },
  { 2013,  44888.16 },
  { 2014,  46481.52 },
  { 2015,  48015.41 },
  { 2016,  50608.24 },
  { 2018,  53189.26 },
  { 2019,  55689.16 },
  { 2019,  58306.55 },
  { 2020,  60872.04 },
  { 2021,  63550.41 },
  { 2022,  66219.53 },
  { 2023,  68934.53 },
  { 2024,  71622.97 },
  { 2025,  74416.27 },
  { 2026,  77318.50 },
  { 2027,  80333.93 },
  { 2028,  83466.95 },
  { 2029,  86722.16 },
  { 2030,  90104.32 },
  { 2031,  93618.39 },
  { 2032,  97269.51 }
};
#define NUM_AWI_ENTRIES (sizeof(awi) / sizeof(average_wage_index))

/* These are the "bend points" for the Primary Insurance Amount (PIA)
 * calculation.  They were lifted from:
 *   https://www.ssa.gov/oact/COLA/bendpoints.html
 */
typedef struct bend_point {
  int year;
  int pia_bend1;
  int pia_bend2;
  int family1;
  int family2;
  int family3;
} bend_point;

static bend_point
bend_points[] = {
  { 1979, 180, 1085,  230,  332,  433 },
  { 1980, 194, 1171,  248,  358,  467 },
  { 1981, 211, 1274,  270,  390,  508 },
  { 1982, 230, 1388,  294,  425,  554 },
  { 1983, 254, 1528,  324,  468,  610 },
  { 1984, 267, 1612,  342,  493,  643 },
  { 1985, 280, 1691,  358,  517,  675 },
  { 1986, 297, 1790,  379,  548,  714 },
  { 1987, 310, 1866,  396,  571,  745 },
  { 1988, 319, 1922,  407,  588,  767 },
  { 1989, 339, 2044,  433,  626,  816 },
  { 1990, 356, 2145,  455,  656,  856 },
  { 1991, 370, 2230,  473,  682,  890 },
  { 1992, 387, 2333,  495,  714,  931 },
  { 1993, 401, 2420,  513,  740,  966 },
  { 1994, 422, 2545,  539,  779, 1016 },
  { 1995, 426, 2567,  544,  785, 1024 },
  { 1996, 437, 2635,  559,  806, 1052 },
  { 1997, 455, 2741,  581,  839, 1094 },
  { 1998, 477, 2875,  609,  880, 1147 },
  { 1999, 505, 3043,  645,  931, 1214 },
  { 2000, 531, 3202,  679,  980, 1278 },
  { 2001, 561, 3381,  717, 1034, 1349 },
  { 2002, 592, 3567,  756, 1092, 1424 },
  { 2003, 606, 3653,  774, 1118, 1458 },
  { 2004, 612, 3689,  782, 1129, 1472 },
  { 2005, 627, 3779,  801, 1156, 1508 },
  { 2006, 656, 3955,  838, 1210, 1578 },
  { 2007, 680, 4100,  869, 1255, 1636 },
  { 2008, 711, 4288,  909, 1312, 1711 },
  { 2009, 744, 4483,  950, 1372, 1789 },
  { 2010, 761, 4586,  972, 1403, 1830 },
  { 2011, 749, 4517,  957, 1382, 1803 },
  { 2012, 767, 4624,  980, 1415, 1845 },
  { 2013, 791, 4768, 1011, 1459, 1903 },
  { 2014, 816, 4917, 1042, 1505, 1962 },
  { 2015, 826, 4980, 1056, 1524, 1987 },
  { 2016, 856, 5157, 1093, 1578, 2058 }
};
#define NUM_BEND_POINT_ENTRIES (sizeof(bend_points) / sizeof(bend_point))

/* This is the list of highest indexed earnings. */
static int highest_indexed_earnings[TOTAL_HIGHEST_INDEXED_EARNINGS] = { 0 };

/* This is the Highest-35 total. */
static int total_indexed_earnings = 0;

/* This is the calculated Average Indexed Monthly Earnings (AIME). */
static int AIME = 0;

/* This function returns a pointer to the average wage index, not the actual
 * index.  Returning a pointer allows the ability to fail (by returning a NULL
 * pointer) if the requested year isn't in the table. */
static float *
average_wage_index_get(int year)
{
  float *result = (float *) 0;

  int i;
  for(i = 0; i < NUM_AWI_ENTRIES; i++) {
    average_wage_index *a = &awi[i];
    if(a->year == year) {
      result = &a->index;
      break;
    }
  }

  return result;
}

/* This function returns pointers to the bend1 and bend2 points for the PIA
 * calculation, not the actual bend points themselves.  Returning pointers
 * allows the ability to fail (by returning NULL pointers) if the requested year
 * isn't in the table. */
static int
bend_points_get(int dob,
                int *bend1,
                int *bend2)
{
  int retcode = 1;

  int age_62 = dob + 62;
  int i;
  for(i = 0; i < NUM_BEND_POINT_ENTRIES; i++) {
    bend_point *p = &bend_points[i];
    if(p->year == age_62) {
      *bend1 = p->pia_bend1;
      *bend2 = p->pia_bend2;
      retcode = 0;
      break;
    }
  }

  return retcode;
}

/* This function calculates the "indexing factor" for the specified year. */
float
calc_indexing_factor(int dob,
                     int year)
{
  float indexing_factor = 0.0;

  /* If the person is age 60+, then their indexing factor is always 1. */
  int age_60 = dob + 60;
  if(year >= age_60) {
    indexing_factor = 1.0;
  }

  /* If the person is less than age 60, calculate their indexing factor. */
  else {
    float *awi_60 = average_wage_index_get(age_60);
    float *awi    = average_wage_index_get(year);
    if((awi_60 != (float *) 0) && (awi != (float *) 0)) {
      indexing_factor = *awi_60 / *awi;
    }
  }

  return indexing_factor;
}

int ssa_init(void)
{
  int retcode = 0;

  memset(highest_indexed_earnings, 0, sizeof(highest_indexed_earnings));

  total_indexed_earnings = 0;

  AIME = 0;

  return retcode;
}

int ssa_add_wage(int dob,
                 int year,
                 int wage)
{
  int retcode = 0;

  float indexing_factor = calc_indexing_factor(dob, year);
  int indexed_earnings = wage * indexing_factor;

  /* Do we need to round indexed_earnings up? */
  float ie = wage * indexing_factor;
  float a = indexed_earnings;
  a += 0.5;
  if(ie >= a) {
    indexed_earnings++;
  }

  /* Add the indexed earnings to the top 35 list.  Replace the lowest
   * number in the list (replacing any zero is the most likely outcome). */
  int x;
  int y;
  for(x = 0, y = 0; x < TOTAL_HIGHEST_INDEXED_EARNINGS; x++) {
    /* You can't get any lower than zero. */
    if(highest_indexed_earnings[x] == 0) {
      y = x;
      break;
    }

    /* See if we found a lower value. */
    if(highest_indexed_earnings[x] < highest_indexed_earnings[y]) {
      y = x;
    }
  }

  if(indexed_earnings > highest_indexed_earnings[y]) {
      highest_indexed_earnings[y] = indexed_earnings;
  }

  return retcode;
}

int ssa_calc_benefit(int dob, int *PIA)
{
  int retcode = 0;

  int i;
  for(i = 0; i < TOTAL_HIGHEST_INDEXED_EARNINGS; i++) {
    total_indexed_earnings += highest_indexed_earnings[i];
  }
  AIME = total_indexed_earnings / (TOTAL_HIGHEST_INDEXED_EARNINGS * 12);

  int bend1 = 0;
  int bend2 = 0;
  bend_points_get(dob, &bend1, &bend2);

  /* Calculate the Primary Insurance Amount (PIA) (i.e. the Social Security benefit. */
  int temp_AIME = AIME;

  int bend1_amt = min(bend1, temp_AIME);
  float bend1_benefit_float = bend1_amt * 0.9;
  int bend1_benefit = bend1_benefit_float;
  temp_AIME -= bend1_amt;

  int bend2_amt = min((bend2 - bend1), temp_AIME);
  float bend2_benefit_float = bend2_amt * .32;
  int bend2_benefit = bend2_benefit_float;
  temp_AIME -= bend2_amt;

  float more_benefit_float = .15 * temp_AIME;
  int more_benefit = more_benefit_float;

  *PIA = bend1_benefit + bend2_benefit + more_benefit;

  return retcode;
}

