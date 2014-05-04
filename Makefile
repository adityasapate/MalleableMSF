CC=g++
CFLAGS=-c -std=c++0x
OFLAGS=-o
LIBFLAGS=-lpthread
all: MSF.o

#MSF: MSF.o
#	$(CC) $(OFLAGS) MSF MSF.o $(LIBFLAGS)
	
MSF.o: MSF.h MSF.cpp
	$(CC) $(CFLAGS) MSF.cpp

clean:
	rm *.o
#	rm MSF
