#include <stdio.h>
#include <sys/queue.h>

typedef struct MyDataStruct {
	// #define LIST_ENTRY(type)
	// LIST_ENTRY(MyDataStruct) list;
	struct {
		struct MyDataStruct *le_next;
		struct MyDataStruct **le_prev;
	} list;

	int myData;
} MyDataStruct;

static MyDataStruct s1 = { .myData = 1 };
static MyDataStruct s2 = { .myData = 2 };
static MyDataStruct s3 = { .myData = 3 };

// #define LIST_HEAD(name, type)
// typedef LIST_HEAD(myList, MyDataStruct) myDataStruct;
struct myList {
	struct MyDataStruct *lh_first;
} myList;

int main(int argc, char **argv)
{
	// #define LIST_INIT(head)
	// LIST_INIT(&myList);
	do {
		(&myList)->lh_first = NULL;
	} while (0);

	// #define LIST_INSERT_HEAD(head, elm, field)
	// LIST_INSERT_HEAD(&myList, &s1, list);
	do {
		if (((&s1)->list.le_next = (&myList)->lh_first) != NULL)
			(&myList)->lh_first->list.le_prev = &(&s1)->list.le_next;
		(&myList)->lh_first = (&s1);
		(&s1)->list.le_prev = &(&myList)->lh_first;
	} while (0);

	LIST_INSERT_HEAD(&myList, &s2, list);
	LIST_INSERT_HEAD(&myList, &s3, list);

	// #define LIST_FIRST(head)
	// LIST_FIRST(&myList);
	MyDataStruct *entry = ((&myList)->lh_first);
	while(entry != NULL) {
		printf("%d\n", entry->myData);

		// #define LIST_NEXT(elm, field)
		// LIST_NEXT(entry, list);
		entry = ((entry)->list.le_next);
	}
	
	return 0;
}

