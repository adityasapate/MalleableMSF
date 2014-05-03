#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <string>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>
#include <iterator>
#include <sched.h>
#include <unistd.h>
#include <set>
#include <cfloat>

#include "MSF.h"

using namespace std;

//validate the input and return number of cores as input by user
void set_num_cores(int argc, char* argv[]){
	if (argc < 3){
		cout<<"Input format : "<<argv[0]<<" <number of cores> <input file>\n";
		return -1;
	}
	num_cores = atoi(argv[1]);
	
	//query the h/w to get the actual number of logical CPUs
	int total_num_cores = sysconf(_SC_NPROCESSORS_ONLN);

	//doesn't really make sense if num_cores <=0 or >total_num_cores
	//return total_num_cores in this case
	if(num_cores <= 0 || num_cores > total_num_cores)
		num_cores = total_num_cores;
	
	return;
}

/************************** Functions of Edge Class *************************/

Edge::Edge(){

	from = -1;
	to = -1;
	len = -1;
	from_cluster = -1;
	to_cluster = -1;
	pthread_mutex_init(&(this->edge_lock), NULL);

}


Edge::Edge(long a, long b, float d){
	from = a;
	to = b;
	len = d;
	from_cluster = cluster_set[from];
	to_cluster = cluster_set[to];
	pthread_mutex_init(&this->edge_lock, NULL);
}

Edge::print(){
	cout<<from<<"->"<<to<<" : "<<len<<"\n";
	return;
}

bool Edge::operator<( const Edge other ) const{
	if(from == other.from && to == other.to)
		return len < other.len;
	if(from == other.to && to == other.from)
		return len < other.len;
	if(from != other.from)
		return (from < other.from);
	else
		return (to < other.to);
}
bool Edge::operator=( const Edge other ) {
		from = other.from;
		to = other.to;
		len = other.len;
		return true;
}
bool Edge::operator==(const Edge other ) const{
	if(from == other.from)
		if(to == other.to)
			if(len == other.len)
				return true;
	if(from == other.to)
		if(to == other.from)
			if(len == other.len)
				return true;
	return false;
}

bool Cluster::contains(long vertex){
	set<long>::iterator iter = vertices.find(vertex);
	if(iter == vertices.end())
		return false;
	return true;
}

Cluster::Cluster(){
	pthread_mutex_init(&cluster_lock, NULL);
	//default constructor
	root=-1;
	cluster_id = -1;
}

Cluster::Cluster(long i){
	root = i;
	cluster_id = i;
	vertices.insert(i);
	pthread_mutex_init(&cluster_lock);
}

Cluster::Cluster(set<long> vertexSet, long vertex){
	root = vertex;
	vertices = vertexSet;
//	update_out_edges();
	pthread_mutex_init(&cluster_lock, NULL);
}

void Cluster::add_edge(Edge e){
  out_edges.insert(e);
}

void Cluster::print_out_edges(){
	cout<<"Out edges of cluster: \n";
	set<Edge>::iterator iter = out_edges.begin();
	while(iter != out_edges.end()){
		Edge e = *iter;
		cout<<e.getFromVertex() <<"->"<<e.getToVertex()<<" : "<<e.getLen()<<"\n";
		iter++;
	}
}

void Cluster::print(){
	cout<<root<<" : {";
	set<long>::iterator iter = vertices.begin();
	while(iter!=vertices.end()){
		cout<<*iter<<" ";
		iter++;
	}
	cout<<"}";
}

bool Cluster::operator==(const Cluster other ) const{
	return root == other.root;
}

bool Cluster::operator<(const Cluster other) const{
	return (root < other.root);
}

bool Cluster::operator=(const Cluster other) {
	set<long>::iterator vertex_iter = other.vertices.begin();
	while(vertex_iter != other.vertices.end()){
		vertices.insert(*vertex_iter);
		vertex_iter++;
	}
	set<Edge>::iterator edge_iter = other.out_edges.begin();
	while(edge_iter != other.out_edges.end()){
		out_edges.insert(*edge_iter);
		edge_iter++;
	}

	root = other.root;
	pthread_mutex_init(&cluster_lock, NULL);

	return true;
}

void init(int num_args, char** args){
	set_num_cores(num_args, args);
	ifstream input_file;
	input_file.open(args[2]);
	cout<<"Parsing the input file...\n";
	
	
}
