CC=g++
CFLAGS=-c
OFLAGS=-o
LIBFLAGS=-lpthread
all: MSF

MSF: MSF.o
	$(CC) $(OFLAGS) MSF MSF.o $(LIBFLAGS)
	
MSF.o: MSF.cpp MSF.h
	$(CC) $(CFLAGS) MSF.cpp

clean:
	rm *.o
	rm MSF
