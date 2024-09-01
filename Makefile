CC=cc
CFLAGS= -Wall -g

texted: texted.o main.o 
	$(CC) $(CFLAGS) -o texted texted.o main.o

texted.o: texted.c texted.h
	$(CC) $(CFLAGS) -c texted.c

main.o: main.c texted.h
	$(CC) $(CFLAGS) -c main.c

tty_test: tty_display_test.o display.o
	$(CC) $(CFLAGS) -o tty_test tty_display_test.o display.o

tty_display_test.o: tty_display_test.c display.h
	$(CC) $(CFLAGS) -c tty_display_test.c

display.o: display.c display.h
	$(CC) $(CFLAGS) -c display.c

clean:
	rm *.o
