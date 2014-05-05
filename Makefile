CC=g++
CFLAGS=-c -std=c++0x
OFLAGS=-o
LIBFLAGS=-lpthread
all: MSF

MSF: sched.o MSF.o
	$(CC) $(OFLAGS) MSF MSF.o sched.o $(LIBFLAGS)
	
MSF.o: MSF.h sched.h MSF.cpp
	$(CC) $(CFLAGS) MSF.cpp

sched.o: sched.h sched.cpp
	$(CC) $(CFLAGS) sched.cpp
clean:
	rm *.o
	rm MSF
