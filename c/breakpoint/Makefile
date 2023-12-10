
test: breakpoint_handler.a breakpoint.o breakpoint_return.o
	gcc -c -Os -g test.c -o test.o
	gcc -Os -g test.o breakpoint_handler.a -o test -lrt

test_mmap: breakpoint_handler.a breakpoint.o breakpoint_return.o
	gcc -c -Os -g test_mmap.c -o test_mmap.o
	gcc -Os -g test_mmap.o breakpoint_handler.a -o test_mmap -lrt

breakpoint_handler.a: breakpoint.o breakpoint_return.o
	ar rcs breakpoint_handler.a breakpoint.o breakpoint_return.o

breakpoint.o: breakpoint.c
	gcc -c -Os -g breakpoint.c -o breakpoint.o

breakpoint_return.o: breakpoint_return.s
	gcc -c -Os -g breakpoint_return.s -o breakpoint_return.o

clean:
	rm -f test test_mmap *.a *.o

