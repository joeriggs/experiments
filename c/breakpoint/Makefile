
OBJ := breakpoint.o

TARGET := breakpoint

%.o: %.c
	gcc -O0 -g -Wall -Werror -c -o $@ $<

$(TARGET): $(OBJ)
	gcc -o $(TARGET) $(OBJ) -lrt

clean:
	rm -f $(OBJ) $(TARGET)

