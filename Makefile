CC=gcc
CFLAGS= -Wall

texted: texted.o main.o 
	$(CC) $(CFLAGS) -o texted texted.o main.o


texted.o: texted.c texted.h
	$(CC) $(CFLAGS) -c texted.c

main.o: main.c texted.h
	$(CC) $(CFLAGS) -c  main.c

clean:
	rm *.o
