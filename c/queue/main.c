
#include <stdio.h>

#include <sys/queue.h>

/* This is the data type for our TAILQ.  All objects in the TAILQ will have this
 * format.
 */
typedef struct my_data {
	TAILQ_ENTRY(my_data) tailq;
	int val;
} my_data;

/* This is the definition of the TAILQ.  The type is called "my_data_queue".
 * You can declare a my_data_queue object using that name.
 */
TAILQ_HEAD(my_data_queue, my_data);

int main(int argc, char **argv)
{
	struct my_data data[5];
	data[0].val = 2;
	data[1].val = 4;
	data[2].val = 6;
	data[3].val = 8;
	data[4].val = 10;

	/* This is a declaration of a my_data_queue TAILQ object.  We declare it,
	 * and then we initialize it.  It is then ready for use.
	 */
	struct my_data_queue q;
	TAILQ_INIT(&q);

	/* Add some elements to the q object. */
	TAILQ_INSERT_HEAD(  &q,           &data[0], tailq); // Insert at head of the queue.
	TAILQ_INSERT_AFTER( &q, &data[0], &data[1], tailq); // Insert element 1 after element 0.
	TAILQ_INSERT_BEFORE(    &data[1], &data[2], tailq); // Insert element 2 before element 1.
	TAILQ_INSERT_BEFORE(    &data[2], &data[3], tailq); // Insert element 3 before element 2.
	TAILQ_INSERT_TAIL(  &q, &data[4],           tailq); // Insert element 4 at the end.

	/* Order is now: 2, 8, 6, 4, 10 */
	struct my_data *p;
	TAILQ_FOREACH(p, &q, tailq) {
		printf("%2d ", p->val);
	}
	printf("\n");

	/* Remove one element from the middle of the TAILQ. */
	TAILQ_REMOVE(&q, &data[2], tailq);

	/* Order is now: 2, 8, 4, 10 */
	TAILQ_FOREACH(p, &q, tailq) {
		printf("%2d ", p->val);
	}
	printf("\n");

	/* Get the first entry, and then remove it.  If you're going to do this
	 * from a multi-threaded app, then you'll probably want to hold a lock
	 * during these operations.
	 */
	struct my_data *first_entry = TAILQ_FIRST(&q);
	TAILQ_REMOVE(&q, first_entry, tailq);

	/* Order is now: 8, 4, 10 */
	TAILQ_FOREACH(p, &q, tailq) {
		printf("%2d ", p->val);
	}
	printf("\n");

	while (!TAILQ_EMPTY(&q)) {
		p = TAILQ_FIRST(&q);
		TAILQ_REMOVE(&q, p, tailq);
	}

	return 0;
}

