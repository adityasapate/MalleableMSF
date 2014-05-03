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

/************************* Functions of Cluster class ***********************/

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

/************************* Algo functions *********************************/

void init(int num_args, char** args){
	set_num_cores(num_args, args);
	ifstream input_file;
	input_file.open(args[2]);
	cout<<"Parsing the input file...\n";

	//read number of vertices
	string num_vertices_str;
	getline(input_file, num_vertices_str);
	num_vertices = atoi(num_vertices_str.c_str());

	//allocate initial clusters and belongs_to array
	cluster_set = new Cluster*[num_vertices];
	cluster_set_lock = new pthread_mutex_t[num_vertices];
	
	for(long i = 0; i < num_vertices; i++){
		pthread_mutex_init(&cluster_set_lock[i], NULL);
		Cluster *c = new Cluster(i);
		cluster_set[i] = c;
	}

	while(1){

		string buffer;
		getline(input_file, buffer);
		if(buffer.compare("done") == 0)
			break;
		num_edges++;
		istringstream buff(buffer);
		istream_iterator<string> beg(buff), end;
		vector<string> tokens(beg, end);

		long from_vertex = atol(tokens[0].c_str());
		long to_vertex = atol(tokens[1].c_str());
		float len = ::atof(tokens[2].c_str());

		if(from_vertex >= num_vertices || to_vertex >= num_vertices){
			cout<<"Vertex number cannot be more than or equal to number of vertices\n";
			return NULL;
		}

		Edge *e1 = new Edge(from_vertex, to_vertex, len);
		cluster_set[from_vertex]->add_edge(*e1);
		
		Edge *e2 = new Edge(to_vertex, from_vertex, len);
		cluster_set[to_vertex]->add_edge(*e2);
	
	}
}

void extend_cluster(void* args){

	pair<Cluster*, int*> extend_args = *((pair<Cluster *, int *> *)args);
	Cluster *target = extend_args.first;
	int *result = extend_args.second;
	
	float min_len = FLT_MAX;
	int flag = 0;
	Edge *next_edge;
	set<Edge>::iterator iter = target->get_out_edges().begin();
	while(iter != target->get_out_edges().end()){
		Edge e = *iter;
		if(e.getLen() < min_len){
			flag = 1;
			min_len = e.getLen();
			next_edge = &e;
		}
	}
	if(flag == 0){
		*result = -1;
		return;
	}

//	spanning_forest.insert(*next_edge);

	//prepare arg for merging
	bool merge_result;
	tuple<Cluster*, Cluster*, Edge*, bool*> merge_args;
	get<0>(merge_args) = target;
	get<1> (merge_args) = cluster_set[next_edge->getToVertex()];
	get<2> (merge_args) = next_edge;
	get<3> (merge_args) = &merge_result;
	//TODO : @Priya, @Niranjan
	//You will need to create a pkt here and push it into the work queue
	merge_clusters((void *) &merge_args);

	if(merge_result == false){
		//merge was unsuccessful
		*result = -1;
		return;
	}
	
	//merge was successful
	*result = 1;
	return;
}

void merge_clusters(void* args){

	//parse the input argument
	tuple<Cluster*, Cluster*, Edge*, bool*> merge_args;
	merge_args = *(pair<Cluster*, Cluster*, Edge*> *)args;
	Cluster *winner = get<0>(merge_args);
	Cluster *loser = get<1>(merge_args);
	Edge* cross_edge = get<2> (merge_args);
	bool* merge_success = get<3> (merge_args);

	//winner = cluster with more vertices
	if(winner->get_vertices().size() < loser->get_vertices().size()){
		Cluster *tmp = winner;
		winner = loser;
		loser = tmp;
	}

	//create arg for check_cycles
	bool check_result;
	tuple<Cluster*, Cluster*, Edge*, bool*> check_args;
	get<0>(check_args) = winner;
	get<1>(check_args) = loser;
	get<2>(check_args) = cross_edge;
	get<3>(check_args) = &check_result;

	//check if cycles will be formed if we merge
	check_cycles((void *)&check_args);

	if(check_result == false){
		//no risk : do the merge
		//swap direction of cross edge so that it is from winner to loser
		if(cluster_set[cross_edge->getFromVertex()] != winner){
			long tmp = cross_edge->getFromVertex();
			cross_edge->setFromVertex(cross_edge->getToVertex());
			cross_edge->setToVertex(tmp);
		}
		//TODO: Any better way to do this than lockng out both clusters?
		winner->lock();
		loser->lock();
		//merge edges
		set<Edge>::iterator it = loser->get_out_edges().begin();

		//add necessary loser edges to winner
		while(it != loser->get_out_edges().end()){
			Edge e = *it;
			if(cluster_set[e.getToVertex()] != winner)
				winner->get_out_edges().insert(e);
			it++;
		}

		//erase unnecessary edges from winner
		it = winner->get_out_edges().begin();
		while(it != winner->get_out_edges().end()){
			Edge e = *it;
			if(cluster_set[e.getToVertex()] == loser)
				winner->get_out_edges().erase(e);
			it++;
		}
		//merge vertices
		set<long>::iterator iter = loser->get_vertices().begin();
		while(iter != loser->get_out_edges().end()){
			long vertex = *iter;
			pthread_mutex_lock(&cluster_set_lock[vertex]);
			if(cluster_set[vertex] == loser){
				//update cluster_set
				cluster_set[vertex] = winner;
				//add vertex to winner's vertices
				winner->get_vertices().insert(vertex);
			}
			pthread_mutex_unlock(&cluster_set_lock[vertex]);
			iter++;
		}
		loser->unlock();
		winner->unlock();
		merge_success = true;
		return;
	}
	else{
		*merge_success = false;
		return;
	}
	return;
}
	

	
	

}

