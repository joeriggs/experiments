/*******************************************************************************
 *
 * This program calculates an RSA Public and Private key pair.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>

/*******************************************************************************
 * Given p, q, and e, calculate d.
 *
 * Returns 0 if success.
 * Returns 1 if failure.
 ******************************************************************************/
static int64_t
calculate_d(int64_t p, int64_t q, int64_t e)
{
	printf("Testing:  p = %jd, q = %jd, e = %jd.\n", p, q, e);
	/* n = (p * q). */
	int64_t n = (p * q);

	/* Calculate phi (p - 1) * (q - 1). */
	int64_t phi = (p - 1) * (q - 1);
	printf("          phi = %jd.\n", phi);

	/* Make sure e doesn't share a factor with phi. */
	if((phi % e) == 0) {
		printf("e won't work (%jd / %jd = %jd).\n", phi, e, (phi / e));
		return 1;
	}

	/* Calculate d using the Extended Euclidian Algorithm. */
	int64_t d = 0;
	{
		int64_t val1a = phi;
		int64_t val1b = e;

		int64_t val2a = phi;
		int64_t val2b = 1;

		do {
			int64_t x = val1a / val1b;

			int64_t val1c = val1a - (x * val1b);
			int64_t val2c = val2a - (x * val2b);

			while(val1c < 0) {
				val1c += phi;
			}
			while(val2c < 0) {
				val2c += phi;
			}

			val1a = val1b;
			val1b = val1c;

			val2a = val2b;
			val2b = val2c;

		} while(val1b != 1);

		d = val2b;
		printf("          d = %jd.\n", d);
	}

	/* Test e and d.  (e * d) mod phi = 1. */
	if(((e * d) % phi) != 1) {
		printf("e and d don't work (%jd * %jd) / %jd = %jd.\n",
		        e, d, phi, ((e * d) / phi));
		return 1;
	}

	printf("Done: p %jd: q %jd: n %jd: phi %jd: e %jd: d %jd.\n",
	         p, q, n, phi, e, d);
	return 0;
}

/*******************************************************************************
 *
 * Here are a couple sets of test data:
 *
 * p =       5, q =      11, e =  7, d =             23
 * p =      61, q =      53, e = 17, d =          2,753
 * p =     113, q =      91, e = 17, d =            593
 * p = 170,497, p = 170,503, e =  5, d = 11,627,963,597
 * p = 170,497, p = 170,503, e = 11, d = 18,499,032,995
 *
 ******************************************************************************/
int main(int argc, char **argv)
{

	calculate_d(    5,      11,  7);
	calculate_d(   61,      53, 17);
	calculate_d(   113,     91, 17);
	calculate_d(170497, 170503,  5);
	calculate_d(170497, 170503, 11);

	return 0;
}

