#include <cstring>
#include "sched.h"


using namespace std;

workobject:: workobject(void (*f)(void*), void* a) {
	function = f;
	args = a;
}

void* workobject::getArgs() {
                return args;
}

unsigned long long getticks()
{
        struct timeval t;
        gettimeofday(&t, 0);
        return t.tv_sec * 1000000ULL + t.tv_usec;
}

double ticks_to_milliseconds(unsigned long long ticks)
{
        return ticks * 1.0e-3;
}
void *thread_function(void* args) {
	
	int threadid = *(int*)args;
	printf("Worker thread %d created\n", threadid);
	
	//wait for the signal to start working
	pthread_mutex_lock(&mutex[threadid]);
	pthread_cond_wait(&cond, &mutex[threadid]);
	pthread_mutex_unlock(&mutex[threadid]);
	while(1) {
		if(sleep_variables[threadid] == 1)
		{
			printf("Thread %d going to sleep as CPU is busy\n",threadid); 
			pthread_mutex_lock(&mutex[threadid]);
			pthread_cond_wait(&condition[threadid], &mutex[threadid]);
			pthread_mutex_unlock(&mutex[threadid]);
			printf("Thread %d coming back from sleep to work\n", threadid);
		}
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
		(*(obj.function))(obj.getArgs());
	}
	
	printf("Thread %d exiting.. \n", threadid);
	pthread_exit(0);
}

void *schedule(void* arg) {
	
	int num_cores = *(int*)arg;
	pthread_t* cputhreads = new pthread_t[num_cores];
	cpu_set_t* cpusets = new cpu_set_t[num_cores+1];
	mutex = new pthread_mutex_t[num_cores];
	condition = new pthread_cond_t[num_cores];
	sleep_variables = new int[num_cores];
	idle_percent = new float[num_cores];
	
// 	barrier = new int[4];
	CPU_ZERO(&cpusets[0]);
	CPU_SET(0, &cpusets[0]);
	pthread_setaffinity_np(pthread_self(), sizeof(cpusets[0]), &cpusets[0]);
	
	for(int i = 1; i <= num_cores; i++){
		CPU_ZERO(&cpusets[i]);
		CPU_SET(i, &cpusets[i]);
		pthread_mutex_init(&mutex[i-1], NULL);
		pthread_cond_init(&condition[i-1], NULL);
// 		barrier[i] = 0;
		sleep_variables[i-1] = 0;
		pthread_setaffinity_np(cputhreads[i-1], sizeof(cpusets[i]), &cpusets[i]);
		int n = i-1;
		pthread_create(&cputhreads[i-1], NULL, thread_function, (void*)&n);
	}
	
	while(1)
	{
		long long starttime = getticks();
		long long endtime = getticks();
		while(ticks_to_milliseconds(endtime - starttime) < 10)
			endtime = getticks();
		calculate_idle();
		if(barrier == 1)
			break;
	}
	
	for(int i = 0; i < num_cores; i++) {
		sleep_variables[i] = 0;
		pthread_cond_signal(&condition[i]);
	}
	pthread_exit(0);
}

void calculate_idle()
{
	FILE *file;
	file = popen("mpstat -P ALL","r");
	char string[50];
	int curr_cpu = 0;
	while(fgets(string, 50, file) != NULL) {
		if(strcmp(string, "\%idle") == 0)
			break;
	}
	
	for(int i = 0; i < 12; i++)
	{
		if(fgets(string, 50, file) == NULL)
			return;
	}
	
	while(1) {
		for(int i = 0; i < 12; i++)
		{
			if(fgets(string, 50, file) == NULL)
				return;
		}
		idle_percent[curr_cpu++] = atof(string);
		if(atof(string) < IDLE_THRESH)
			sleep_variables[curr_cpu-1] = 1;
		else
		{
			sleep_variables[curr_cpu-1] = 0;
			pthread_cond_signal(&condition[curr_cpu-1]);
		}
	}
	
}
