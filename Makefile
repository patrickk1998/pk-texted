F=frontend
B=backend
T=tests

CC=cc
CFLAGS= -Wall -fsanitize=address -g

all: texted tests

OBJS = $F/texted.o main.o $F/display.o $F/input.o $B/span.o $F/codepoint.o utf8.o

texted: $(OBJS) 
	$(CC) $(CFLAGS) -o texted $(OBJS)

tests: tty_test input_test codepoint_test

tty_test: $T/tty_display_test.o $F/display.o $F/codepoint.o utf8.o
	$(CC) $(CFLAGS) -o tty_test $T/tty_display_test.o $F/display.o $F/codepoint.o utf8.o


input_test: $T/input_test.o $F/input.o
	$(CC) $(CFLAGS) -o input_test $T/input_test.o $F/input.o

codepoint_test: $T/codepoint_test.o $F/codepoint.o utf8.o
	$(CC) $(CFLAGS) -o codepoint_test $T/codepoint_test.o $F/codepoint.o utf8.o

clean:
	rm *.o $F/*.o $B/*.o $T/*.o 
