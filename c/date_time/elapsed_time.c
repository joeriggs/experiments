/*******************************************************************************
 *
 * This program does a little bit of work, and then calculates the amount of
 * elapsed time.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

/*******************************************************************************
 * Given 2 struct timeval data structures, calculate the elapsed time.  Return
 * a string back to the caller that contains the elapsed time.  Note that this
 * function is not thread safe.  It uses a static buffer to hold the result.
 ******************************************************************************/
static const char *
calc_elapsed_time(struct timeval *bef, struct timeval *aft)
{
	static char res[128];

	int64_t elapsed_tvsec  = (aft->tv_sec  - bef->tv_sec);
	int64_t elapsed_tvusec = (aft->tv_usec - bef->tv_usec);

	int64_t elapsed_usec = (elapsed_tvsec * 1000000) + elapsed_tvusec;

	int64_t sec = elapsed_usec / 1000000;
	int64_t usec = elapsed_usec % 1000000;

	snprintf(res, sizeof(res) - 1, "%3jd.%06jd", sec, usec);
	return res;
}

/*******************************************************************************
 * Given p, try to determine if it's prime by trying all possible factors.  This
 * is a brute force test.  It can easily burn some CPU cycles.
 *
 * Returns 0 if p is prime.
 * Returns 1 if p is NOT prime.
 ******************************************************************************/
static int
do_some_work(int64_t p)
{
	int64_t i = 2;
	while(i < p) {
		if((p % i++) == 0) {
			return 1;
		}
	}

	return 0;
}

/*******************************************************************************
 * Run a few tests.  Display the elapsed time for each test.
 ******************************************************************************/
int main(int argc, char **argv)
{
	int64_t p;
	for(p = 1; p < 1000000000000ll; p *= 11) {
		/* Get a starting timestamp. */
		struct timeval bef;
		gettimeofday(&bef, 0);

		int res = do_some_work(++p);

		/* Get the ending timestamp. */
		struct timeval aft;
		gettimeofday(&aft, 0);

		printf("%s seconds: %12jd %s prime.\n",
		        calc_elapsed_time(&bef, &aft), p,
		        (res == 0) ? "is" : "is not");
	}

	return 0;
}

