#include <cstdio>
#include <cstdlib>
#include <queue>
#include <pthread.h>

class workobject {
	void* function;
	void* args;
public:
	workobject(int f, int a) {
		function = f;
		args = a;
	}
	
	int getThreadid() {
		return threadid;
	}
	
	int getFlag() {
		return flag;
	}
	
}; 

std::queue <workobject> queue_work;

void schedule(int);
void thread_function(void*);
pthread_mutex_t *mutex;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int barrier = 0;
