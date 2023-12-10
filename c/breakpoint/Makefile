
test:
	gcc -Os -g test.c breakpoint.c breakpoint_return.s -o test -lrt

test_mmap:
	gcc -Os -g test_mmap.c breakpoint.c breakpoint_return.s -o test_mmap -lrt

clean:
	rm -f test test_mmap

