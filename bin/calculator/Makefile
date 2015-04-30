
OBJ := calculator.o main.o raw_stdin.o stack.o

TARGET := calculator

%.o: %.c
	gcc -c -o $@ $<

$(TARGET): calculator.o main.o raw_stdin.o stack.o

clean:
	rm -f $(OBJ) $(TARGET)

