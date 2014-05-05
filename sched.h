#ifndef _SCHED_H_
#define _SCHED_H_ 

#include <cstdio>
#include <cstdlib>
#include <queue>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#define PROBE_TIME 10	//milliseconds
#define IDLE_THRESH 90	//threshold for making the thread run

class workobject {
	void* args;
public:
	void (*function)(void*);
	workobject(void (*f)(void*), void* a); 
	
	void* getArgs();
}; 

std::queue <workobject> queue_work;

void *schedule(void*);
void *thread_function(void*);
void calculate_idle();
float *idle_percent;
pthread_mutex_t *mutex;
pthread_cond_t cond;
pthread_cond_t *condition;
int barrier;
int *sleep_variables;

unsigned long long getticks();

double ticks_to_milliseconds(unsigned long long ticks);

#endif
