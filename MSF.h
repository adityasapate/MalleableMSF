#include<pthread.h>
#include<set>
#include<algorithm>

using namespace std;

pthread_t *cputhreads;
pthread_mutex_t* successor_lock;

float **matrix;
long *successor;
long max_subgraph_size;
long num_vertices;
long num_edges;
int num_cores;

class Edge{
	long from;
	long to;
	float len;
	pthreead_mutex_t lock;

public:
	Edge(){
		from = -1;
		to = -1;
		len = -1;
		pthread_mutex_init(&lock, NULL);
	}
	Edge(long a, long b){
		from = a;
		to = b;
		len = -1;
		pthread_mutex_init(&lock, NULL);
	}

	Edge(long a, long b, float d){
		from = a;
		to = b;
		len = d;
		pthread_mutex_init(&lock, NULL);
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
	bool operator<( const Edge other ) const{
		if(from != other.from)
			return (from < other.from);
		else if(to != other.to)
			return (to < other.to);
		else
			return (len < other.len);
    }
    bool operator=( const Edge& other ) const{
		from = other.from;
		to = other.to;
		len = other.len;

		return true;
	}
};

class Cluster{
	set<long> vertices;
	set<Edge> out_edges;

public:

	void update_out_edges(){
		for(long i = 0; i < vertices.size(); i++){
			long vertex = vertices[i];
			for(long j = 0; j < num_vertices; j++){
				if(matrix[vertex][j] != 0){
					Edge e = new Edge(vertex, j, matrix[vertex][j]);
					this->out_edges.insert(e);
				}
			}
		}
	}
	Edge *get_lightest_out_edge(){
		return min_element<Edge>(out_edges, out_edges + out_edges.size() + 1);
	}
	long size(){
		return vertices.size();
	}
	Cluster(){
		//default constructor
	}
	Cluster(set<long> vertexSet){
		vertices = vertexSet;
		update_out_edges();
	}
	
	
};
set<Cluster> *cluster_set;

set<Edge> spanning_forest;
set<Edge> edge_pool;

pthreead_mutex_t edge_pool_lock;