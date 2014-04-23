#include<pthread.h>

pthread_t *cputhreads;
pthread_mutex_t* successor_lock;
float **matrix;
long *successor;
long max_subgraph_size;

class edge{
	long from;
	long to;
	float len;

public:
	edge(long a, long b){
		from = a;
		to = b;
	}

	edge(long a, long b, float d){
		from = a;
		to = b;
		len = d;
	}
};

class argPacket1{
	long num_vertices;
	long num_edges;
	int num_cores;

public:
	argPacket1(long a, long b, int c){
		num_vertices = a;
		num_edges = b;
		num_cores = c;
	}

	long getVertices(){
		return num_vertices;
	}
	long getEdges(){
		return num_edges;
	}
	int getCores(){
		return num_cores;
	}
};

class argPacket2{
	long *num_vertices;
	long *num_edges;
	int num_cores;

public:
	argPacket2(long *a, long *b, int c){
		num_vertices = a;
		num_edges = b;
		num_cores = c;
	}

	long *getVertices(){
		return num_vertices;
	}
	long *getEdges(){
		return num_edges;
	}
	int getCores(){
		return num_cores;
	}
};