/*******************************************************************************
 * This file contains the implementation of the algorithms that are used to
 * calculate or estimate the Social Security benefits for a retiree.
 *
 * Here's a simple description of how it all works:
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
 *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "ssa.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

#define TOTAL_HIGHEST_INDEXED_EARNINGS 35

/* Contribution and Benefit Base Table.  Taken from:
 *   https://www.ssa.gov/oact/COLA/cbb.html
 *
 * This table represents the maximum earnings for each year.
 */
typedef struct maximum_wage {
  int year;
  int wage;
} maximum_wage;

static maximum_wage
maximum_earnings[] = {
  { 1937,   3000 },
  { 1938,   3000 },
  { 1939,   3000 },
  { 1940,   3000 },
  { 1941,   3000 },
  { 1942,   3000 },
  { 1943,   3000 },
  { 1944,   3000 },
  { 1945,   3000 },
  { 1946,   3000 },
  { 1947,   3000 },
  { 1948,   3000 },
  { 1949,   3000 },
  { 1950,   3000 },
  { 1951,   3600 },
  { 1952,   3600 },
  { 1953,   3600 },
  { 1954,   3600 },
  { 1955,   4200 },
  { 1956,   4200 },
  { 1957,   4200 },
  { 1958,   4200 },
  { 1959,   4800 },
  { 1960,   4800 },
  { 1961,   4800 },
  { 1962,   4800 },
  { 1963,   4800 },
  { 1964,   4800 },
  { 1965,   4800 },
  { 1966,   6600 },
  { 1967,   6600 },
  { 1968,   7800 },
  { 1969,   7800 },
  { 1970,   7800 },
  { 1971,   7800 },
  { 1972,   9000 },
  { 1973,  10800 },
  { 1974,  13200 },
  { 1975,  14100 },
  { 1976,  15300 },
  { 1977,  16500 },
  { 1978,  17700 },
  { 1979,  22900 },
  { 1980,  25900 },
  { 1981,  29700 },
  { 1982,  32400 },
  { 1983,  35700 },
  { 1984,  37800 },
  { 1985,  39600 },
  { 1986,  42000 },
  { 1987,  43800 },
  { 1988,  45000 },
  { 1989,  48000 },
  { 1990,  51300 },
  { 1991,  53400 },
  { 1992,  55500 },
  { 1993,  57600 },
  { 1994,  60600 },
  { 1995,  61200 },
  { 1996,  62700 },
  { 1997,  65400 },
  { 1998,  68400 },
  { 1999,  72600 },
  { 2000,  76200 },
  { 2001,  80400 },
  { 2002,  84900 },
  { 2003,  87000 },
  { 2004,  87900 },
  { 2005,  90000 },
  { 2006,  94200 },
  { 2007,  97500 },
  { 2008, 102000 },
  { 2009, 106800 },
  { 2010, 106800 },
  { 2011, 106800 },
  { 2012, 110100 },
  { 2013, 113700 },
  { 2014, 117000 },
  { 2015, 118500 },
  { 2016, 118500 }
};
#define NUM_MAXIMUM_EARNINGS_ENTRIES (sizeof(maximum_earnings) / sizeof(maximum_wage))

/* Average Wage Index (a.k.a. AWI) Table, taken from:
 *   https://www.ssa.gov/oact/COLA/AWI.html
 *
 * Dates after 2014 can be estimated using data from this page, but we aren't
 * doing that estimation.  It looks like the ssa.gov website doesn't, so we will
 * do the same as them.
 *   https://www.ssa.gov/oact/TR/TRassum.html
 *
 * A description of how to use this table can be found at:
 *   https://www.ssa.gov/oact/ProgData/retirebenefit1.html
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
  { 2014,  46481.52 }
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
#define NUM_BEND_POINTS_ENTRIES (sizeof(bend_points) / sizeof(bend_point))

/* This is the list of highest indexed earnings. */
static int highest_indexed_earnings[TOTAL_HIGHEST_INDEXED_EARNINGS] = { 0 };

/* This is the Highest-35 total. */
static int total_indexed_earnings = 0;

/* This is the calculated Average Indexed Monthly Earnings (AIME). */
static int AIME = 0;

/* This function returns the maximum earnings for the specified year.  If the
 * year doesn't exist in the table, then the maximum earnings for the last year
 * in the table is returned.  The requested year is probably later than any of
 * the data in the table, so it makes sense to keep using the most recent year.
 */
static int
maximum_earnings_get(int year)
{
  /* Default to the last entry in the table. */
  int result = maximum_earnings[NUM_MAXIMUM_EARNINGS_ENTRIES - 1].wage;

  int i;
  for(i = 0; i < NUM_MAXIMUM_EARNINGS_ENTRIES; i++) {
    maximum_wage *w = &maximum_earnings[i];
    if(w->year == year) {
      result = w->wage;
      break;
    }
  }

  //printf("Maximum earnings for %d is $%d\n", year, result);
  return result;
}

/* This function returns the average wage index for the specified year.  If
 * the year isn't present in the table, then it returns the AWI for the final
 * year in the table.  This covers situations where a younger person is using
 * the tool. */
static float
average_wage_index_get(int year)
{
  float result = awi[NUM_AWI_ENTRIES - 1].index;

  int i;
  for(i = 0; i < NUM_AWI_ENTRIES; i++) {
    average_wage_index *a = &awi[i];
    if(a->year == year) {
      result = a->index;
      break;
    }
  }

  return result;
}

/* This function returns the bend points for the year the person turns 62.  The
 * bend points are used in the PIA calculation.
 *
 * If there aren't bend points for a person born in the specified year, then we
 * return the bend points for the last year in the table.  This will cover folks
 * who are too young to retire yet.
 */
static int
bend_points_get(int dob,
                int *bend1,
                int *bend2)
{
  int retcode = 1;

  bend_point *p = &bend_points[NUM_BEND_POINTS_ENTRIES - 1];
  *bend1 = p->pia_bend1;
  *bend2 = p->pia_bend2;

  int age_62 = dob + 62;
  int i;
  for(i = 0; i < NUM_BEND_POINTS_ENTRIES; i++) {
    p = &bend_points[i];
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
    float awi_60 = average_wage_index_get(age_60);
    float awi    = average_wage_index_get(year);
    indexing_factor = awi_60 / awi;
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

  /* If the person earned more than the maximum allowed, then adjust their
   * wage down to the maximum allowed amount for that year. */
  int maximum_earnings = maximum_earnings_get(year);
  int allowed_wage = min(wage, maximum_earnings);
  //printf("allowed_wage for %d is %d.\n", year, allowed_wage);

  float indexing_factor = calc_indexing_factor(dob, year);
  int indexed_earnings = allowed_wage * indexing_factor;

  /* Do we need to round indexed_earnings up? */
  float ie = allowed_wage * indexing_factor;
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
    //printf("highest_indexed_earnings[%d] = %7d\n", i, highest_indexed_earnings[i]);
    total_indexed_earnings += highest_indexed_earnings[i];
  }
  AIME = total_indexed_earnings / (TOTAL_HIGHEST_INDEXED_EARNINGS * 12);
  //printf("total_indexed_earnings = %8d.  AIME = %6d.\n", total_indexed_earnings, AIME);

  int bend1 = 0;
  int bend2 = 0;
  bend_points_get(dob, &bend1, &bend2);
  //printf("bend1 = %d.  bend2 = %d.\n", bend1, bend2);

  /* Calculate the Primary Insurance Amount (PIA) (i.e. the Social Security benefit. */
  int temp_AIME = AIME;

  int bend1_amt = min(bend1, temp_AIME);
  float bend1_benefit_float = bend1_amt * 0.90;
  int bend1_benefit = bend1_benefit_float;
  //printf("bend1_amt = %d.  bend1_benefit =  %d.\n", bend1_amt, bend1_benefit);
  temp_AIME -= bend1_amt;

  int bend2_amt = min((bend2 - bend1), temp_AIME);
  float bend2_benefit_float = bend2_amt * 0.32;
  int bend2_benefit = bend2_benefit_float;
  //printf("bend2_amt = %d.  bend2_benefit =  %d.\n", bend2_amt, bend2_benefit);
  temp_AIME -= bend2_amt;

  int more_amt = temp_AIME;
  float more_benefit_float = 0.15 * more_amt;
  int more_benefit = more_benefit_float;
  //printf("more_amt = %d.  more_benefit = %d.\n", more_amt, more_benefit);

  *PIA = bend1_benefit + bend2_benefit + more_benefit;
  //printf("PIA %d.\n", *PIA);

  {
    /* For illustrative purposes only, show the PIA for different ages (early
     * and late retirement ages).  This data assumes the person has a full
     * retirement age of 67.
     *
     * According to the information at:
     *    https://www.ssa.gov/oact/quickcalc/early_late.html#late
     *
     * In the case of early retirement, a benefit is reduced 5/9 of one percent
     * for each month before normal retirement age, up to 36 months. If the
     * number of months exceeds 36, then the benefit is further reduced 5/12 of
     * one percent per month.
     *
     * For example, if the number of reduction months is 60 (the maximum number
     * for retirement at 62 when normal retirement age is 67), then the benefit
     * is reduced by 30 percent. This maximum reduction is calculated as 36
     * months times 5/9 of 1 percent plus 24 months times 5/12 of 1 percent. */

    /* Comment added by me: For late retirement, an additional 8% is added per
     * year. */
    float PIA_f = *PIA;

    int PIA_67 = *PIA;

    PIA_f *= 0.9333333333333;
    int PIA_66 = PIA_f;

    PIA_f *= 0.9333333333333;
    int PIA_65 = PIA_f;

    PIA_f *= 0.9333333333333;
    int PIA_64 = PIA_f;

    PIA_f *= 0.9583333333333;
    int PIA_63 = PIA_f;

    PIA_f *= 0.9583333333333;
    int PIA_62 = PIA_f;

    PIA_f = *PIA;
    PIA_f *= 1.08;
    int PIA_68 = PIA_f;
    PIA_f *= 1.08;
    int PIA_69 = PIA_f;
    PIA_f *= 1.08;
    int PIA_70 = PIA_f;

    printf("PIA Age 62 = $%d\n", PIA_62);
    printf("PIA Age 63 = $%d\n", PIA_63);
    printf("PIA Age 64 = $%d\n", PIA_64);
    printf("PIA Age 65 = $%d\n", PIA_65);
    printf("PIA Age 66 = $%d\n", PIA_66);
    printf("PIA Age 67 = $%d\n", PIA_67);
    printf("PIA Age 68 = $%d\n", PIA_68);
    printf("PIA Age 69 = $%d\n", PIA_69);
    printf("PIA Age 70 = $%d\n", PIA_70);
  }

  return retcode;
}

