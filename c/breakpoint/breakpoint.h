
#ifndef __BREAKPOINT_H__
#define __BREAKPOINT_H__

typedef int (*pre_processor_callback)
	(int arg_01, int arg_02, int arg_03, int arg_04, int arg_05,
	 int arg_06, int arg_07, int arg_08, int arg_09, int arg_10);

typedef int (*post_processor_callback) (int retcode);

int breakpoint_handler_init(void);

int breakpoint_handler_set(void *addr,
                           pre_processor_callback pre_cb,
                           post_processor_callback post_cb);

#endif

