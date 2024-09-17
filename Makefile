CC=cc
CFLAGS= -Wall -g

texted: texted.o main.o display.o text.o input.o
	$(CC) $(CFLAGS) -o texted texted.o main.o display.o text.o input.o

texted.o: texted.c texted.h display.h
	$(CC) $(CFLAGS) -c texted.c

main.o: main.c texted.h display.h
	$(CC) $(CFLAGS) -c main.c

tty_test: tty_display_test.o display.o
	$(CC) $(CFLAGS) -o tty_test tty_display_test.o display.o

tty_display_test.o: tty_display_test.c display.h
	$(CC) $(CFLAGS) -c tty_display_test.c

input_test: input_test.o input.o
	$(CC) $(CFLAGS) -o input_test input_test.o input.o

input_test.o: input_test.c input.h
	$(CC) $(CFLAGS) -c input_test.c

display.o: display.c display.h
	$(CC) $(CFLAGS) -c display.c

input.o: input.c input.h
	$(CC) $(CFLAGS) -c input.c

text_test: text_test.o text.o
	$(CC) $(CFLAGS) -o text_test text_test.o text.o

text_test.o: text_test.c text.h
	$(CC) $(CFLAGS) -c text_test.c

text.o: text.c text.h
	$(CC) $(CFLAGS) -c text.c


clean:
	rm *.o
