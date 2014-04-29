#include<pthread.h>
#include<set>
#include<algorithm>
#include<cfloat>

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
	pthread_mutex_t edge_lock;

	
public:

	Edge(){
		from = -1;
		to = -1;
		len = -1;
		pthread_mutex_init(&(this->edge_lock), NULL);
	}
	Edge(long a, long b){
		from = a;
		to = b;
		len = -1;
		pthread_mutex_init(&this->edge_lock, NULL);
	}

	Edge(long a, long b, float d){
		from = a;
		to = b;
		len = d;
		pthread_mutex_init(&this->edge_lock, NULL);
	}
	long getToVertex(){
		return to;
	}
	long getFromVertex(){
		return from;
	}
	float getLen(){
		return len;
	}
	void setToVertex(long a){
		to = a;
	}
	void setFromVertex(long a){
		from = a;
	}
	void setLen(float a){
		len = a;
	}
	void lock(){
		pthread_mutex_lock(&this->edge_lock);
	}
	void unlock(){
		pthread_mutex_unlock(&this->edge_lock);
	}
	bool operator<( const Edge other ) const{
		if(from != other.from)
			return (from < other.from);
		else if(to != other.to)
			return (to < other.to);
		else
			return (len < other.len);
    }
    bool operator=( const Edge other ) {
		from = other.from;
		to = other.to;
		len = other.len;
		return true;
	}
	bool operator==(const Edge other ) const{
		if(from == other.from)
			if(to == other.to)
				if(len == other.len)
					return true;
		return false;
	}
};

class Cluster{
	set<long> vertices;
	set<Edge> out_edges;
	pthread_mutex_t cluster_lock;

public:

	void update_out_edges(){
		set<long>::iterator iter = vertices.begin();
		while(iter != vertices.end()){
			long vertex = *iter;
			for(long j = 0; j < num_vertices; j++){
				if(matrix[vertex][j] != 0){
					if(vertices.find(j) == vertices.end()){
						Edge *e = new Edge(vertex, j, matrix[vertex][j]);
						out_edges.insert(*e);
					}
				}
			}
			iter++;
		}
	}

	set<Edge>::iterator get_lightest_out_edge(){
		float min_len = FLT_MAX;
		set<Edge>::iterator min;
		set<Edge>::iterator iter = out_edges.begin();
		while(iter != out_edges.end()){
			Edge e = *iter;
			if(e.getLen() < min_len){
				min_len = e.getLen();
				min = iter;
			}
			iter++;
		}
		if(iter == out_edges.end())
			return iter;
		
		return min;
	}
	set<Edge> get_out_edges(){
		return out_edges;
	}
	long size(){
		return vertices.size();
	}
	void insert(long vertex){
		vertices.insert(vertex);
		update_out_edges();
		return;
	}
	void lock(){
		pthread_mutex_lock(&cluster_lock);
	}
	void unlock(){
		pthread_mutex_unlock(&cluster_lock);
	}
	Cluster(){
		pthread_mutex_init(&cluster_lock, NULL);
		//default constructor
	}
	Cluster(set<long> vertexSet){
		vertices = vertexSet;
		update_out_edges();
		pthread_mutex_init(&cluster_lock, NULL);
	}
	
	
};
set<Cluster> *cluster_set;

set<Edge> spanning_forest;
set<Edge> edge_pool;

pthread_mutex_t edge_transfer_lock;
pthread_mutex_t cluster_set_lock;