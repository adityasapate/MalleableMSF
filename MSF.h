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
int num_vert_finished;
class Cluster;

class Edge{
	long from;
	long to;
	float len;
//	Cluster from_cluster;
//	Cluster to_cluster;
	pthread_mutex_t edge_lock;

	
public:

	Edge();
	Edge(long, long, float);
	void print();

	//Accessor functions
	
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

	//Operators
	bool operator<(const Edge) const;
	bool operator=(const Edge);
	bool operator==(const Edge) const;

};

class Cluster{
	long cluster_id;
	long root;
	set<long> vertices;
	set<Edge> out_edges;
	pthread_mutex_t cluster_lock;
	int status;

public:
	Cluster();
	Cluster(long);
	Cluster(set<long>, long);
	
	bool contains(long vertex);
	void print_out_edges();
	void add_edge(Edge);
	void print();

	//Operators

	bool operator==(const Cluster) const;
	bool operator<(const Cluster) const;
	bool operator=(const Cluster);

	/*
	void update_out_edges(){
		out_edges.clear();
		cout<<"Outedges cleared..."<<out_edges.size()<<"\n";
		set<long>::iterator iter = vertices.begin();
		while(iter != vertices.end()){
			long vertex = *iter;
			for(long j = 0; j < num_vertices; j++){
				if(matrix[vertex][j] != 0){
					if(vertices.find(j) == vertices.end()){
						Edge *e = new Edge(vertex, j, matrix[vertex][j]);
						if(e->getLen() != FLT_MAX)
							out_edges.insert(*e);
					}
				}
			}
			iter++;
		}
	}

	void get_lightest_out_edge(Edge *min){
		cout<<"here\n";
		float min_len = FLT_MAX;
		set<Edge>::iterator iter = out_edges.begin();
		int i = 0;
		print_out_edges();
		while(iter != out_edges.end()){
			Edge e = *iter;
			i++;
			e.print();
			if(e.getLen() < min_len){
				cout<<"get_lightest_out_edge: min at posn "<<i<<"\n";
				e.print();
				min_len = e.getLen();
				*min = *iter;
			}
			iter++;
		}
		return;
	}
*/
	set<Edge> get_out_edges(){
		return out_edges;
	}
	set<long> get_vertices(){
		return vertices;
	}
	long get_root(){
		return root;
	}
	void set_root(long r){
		root = r;
	}
	long size(){
		return vertices.size();
	}
	void insert(long vertex){
		vertices.insert(vertex);
//		update_out_edges();
		print_out_edges();
		return;
	}
	void lock(){
		pthread_mutex_lock(&cluster_lock);
	}
	void unlock(){
		pthread_mutex_unlock(&cluster_lock);
	}
	int getStatus(){
		return status;
	}
	void setStatus(int i){
		status = i;
	}
/*
	bool merge_into(Cluster other){
		set<long>::iterator iter = vertices.begin();
		while(iter != vertices.end()){
			other.get_vertices().insert(*iter);
			iter++;
		}
		set<Edge>::iterator iter2 = out_edges.begin();
		while(iter2 != out_edges.end()){
			Edge *e = *(iter2);
			if(!other.contains(e->getToVertex())){
				long tmp = e->getFromVertex();
				e->setFromVertex(e->getToVertex());
				e->setToVertex(tmp);
			}
			if(!other.contains(e->getToVertex()))
				other.get_out_edges().insert(e);
			iter2++;
		}
	}
*/
};

//Global data
Cluster **cluster_set;
set<Edge> spanning_forest;
bool *status;

//Locks
pthread_mutex_t edge_transfer_lock;
pthread_mutex_t *cluster_set_lock;
pthread_mutex_t print_lock;
pthread_mutex_t *status_lock;

//work packet functions
void extend_cluster(void*);
void merge_clusters(void*);
//void check_cycles(void*);

//initialisation
void init(int, char**);
