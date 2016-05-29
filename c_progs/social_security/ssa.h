/*******************************************************************************
 * This defines the API to a C implementation of the algorithms used to
 * calculate or estimate the Social Security benefits for a retiree.
 ******************************************************************************/

extern float calc_indexing_factor(int dob, int year);

extern int ssa_init(void);
extern int ssa_add_wage(int dob, int year, int wage);
extern int ssa_calc_benefit(int dob, int *PIA);
