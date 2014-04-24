#include<pthread.h>
#include<set>

using namespace std;

pthread_t *cputhreads;
pthread_mutex_t* successor_lock;
float **matrix;
long *successor;
long max_subgraph_size;

class Edge{
	long from;
	long to;
	float len;

public:
	Edge(){
		from = -1;
		to = -1;
		len = -1;
	}
	Edge(long a, long b){
		from = a;
		to = b;
		len = -1;
	}

	Edge(long a, long b, float d){
		from = a;
		to = b;
		len = d;
	}
	inline long getToVertex(){
		return to;
	}
	inline void setToVertex(long a){
		to = a;
	}
	inline void setFromVertex(long a){
		from = a;
	}
	inline void setLen(float a){
		len = a;
	}
	bool operator<( const Edge& other ) const{
    	return (from < other.from) ;
    }
};

class ArgPacket1{
	long num_vertices;
	long num_edges;
	int num_cores;
	int cpu_id;
public:
	ArgPacket1(long a, long b, int c, int d){
		num_vertices = a;
		num_edges = b;
		num_cores = c;
		cpu_id = d;
	}

	inline long getVertices(){
		return num_vertices;
	}
	inline long getEdges(){
		return num_edges;
	}
	inline int getCores(){
		return num_cores;
	}
	inline int getCpuId(){
		return cpu_id;
	}
};

class ArgPacket2{
	long *num_vertices;
	long *num_edges;
	int num_cores;
	int cpu_id;

public:
	ArgPacket2(long *a, long *b, int c, int d){
		num_vertices = a;
		num_edges = b;
		num_cores = c;
		cpu_id = d;
	}

	inline long *getVertices(){
		return num_vertices;
	}
	inline long *getEdges(){
		return num_edges;
	}
	inline int getCores(){
		return num_cores;
	}
	inline int getCpuId(){
		return cpu_id;
	}
};

set<Edge> spanning_forest;