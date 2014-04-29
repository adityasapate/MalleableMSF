#include "sched.h"

using namespace std;

void thread_function(void *args) {
	
	int threadid = *(int*)args;
	
	//wait for the signal to start working
	pthread_mutex_lock(&mutex[threadid]);
	pthread_cond_wait(&cond, &mutex[threadid]);
	pthread_mutex_unlock(&mutex[threadid]);
	while(1) {
		if(queue_work.empty())
		{
			// If barrier is 1, means work is over and thread has to end
			if(barrier == 1)
				break;
			else
				continue;
		}
		
		//Get the work object from the queue
		workobject obj = queue_work.front();
		queue_work.pop();
		
		//The function to be executed by this thread which is taken from the queue
		*(obj.function)(obj.args);
	}
	
	pthread_exit(0);
}

void schedule(int num_cores) {
	
	cputhreads = new pthread_t[num_cores];
	cpu_set_t* cpusets = new cpu_set_t[num_cores];
	mutex = new pthread_mutex_t[num_cores];
	barrier = new int[4];
	for(int i = 0; i < num_cores; i++){
		CPU_ZERO(&cpusets[i]);
		CPU_SET(i, &cpusets[i]);
		mutex[i] = PTHREAD_MUTEX_INITIALIZER;
		barrier[i] = 0;
		pthread_setaffinity_np(cputhreads[i], sizeof(cpusets[i]), cpusets[i]);
		int n = i;
		pthread_create(&cputhreads[i], NULL, thread_function, (void*)n);
	}
	
}
