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
//TODO: See if you can query the h/w to get number of cores
int get_num_cores(int argc, char* argv[]){
	if (argc < 3){
		cout<<"Input format : "<<argv[0]<<" <number of cores> <input file>\n";
		return -1;
	}
	int num_cores = atoi(argv[1]);
	int total_num_cores = sysconf(_SC_NPROCESSORS_ONLN);

	if(num_cores <= 0 || num_cores > total_num_cores)
		num_cores = total_num_cores;
	
	return num_cores;
}

float** process_inputs(ifstream *input){

	string num_vertices_str;
	getline(*input, num_vertices_str);
//	cout<<"String : "<<num_vertices_str<<"\n";
	num_vertices = atoi(num_vertices_str.c_str());
//	cout<<"num_vertices = "<<num_vertices<<"\n"

	//Allocate space for the matrix
	float** matrix = NULL;
	matrix = new float*[num_vertices];
	for(int i = 0; i < num_vertices; i++){
		matrix[i] = new float[num_vertices];
		for(int j = 0; j < num_vertices;j++)
			matrix[i][j] = FLT_MAX;
	}
//	cout<<"Space allocated for matrix\n";
	
	while(1){
		
		string buffer;
		getline(*input, buffer);
//		cout<<"buffer = "<<buffer<<"\n";
		if(buffer.compare("done") == 0)
			break;
		num_edges++;
		istringstream buff(buffer);
		istream_iterator<string> beg(buff), end;
		vector<string> tokens(beg, end);

		long from_vertex = atol(tokens[0].c_str());
		long to_vertex = atol(tokens[1].c_str());
		float len = ::atof(tokens[2].c_str());
		
		cout<<"From vertex = "<<from_vertex<<"\n";
		cout<<"To vertex = "<<to_vertex<<"\n";
		cout<<"Length = "<<len<<"\n";
		cout<<"\n";
		if(from_vertex == 0 || to_vertex == 0){
			cout<<"Vertex numbering starts from 1. \n";
			return NULL;
		}
		else if(from_vertex > num_vertices || to_vertex > num_vertices){
			cout<<"Vertex number cannot be more than number of vertices\n";
			return NULL;
		}
		
		Edge *e1 = new Edge(from_vertex-1, to_vertex-1, len);
		Edge *e2 = new Edge(to_vertex-1, from_vertex-1, len);
		edge_pool.insert(*e1);
		edge_pool.insert(*e2);
		matrix[from_vertex-1][to_vertex-1] = len;
		matrix[to_vertex-1][from_vertex-1] = len;

	}

	return matrix;
}

/*
Edge* find_lightest_out_edge(set<long> cluster){
	Edge *e = NULL;
	float min_len = FLT_MAX;
	set<long>::const_iterator elem;
	for(elem = cluster.begin(); elem != cluster.end(); elem++ ){
		long vertex = *elem;
		for(int i = 0; i < num_vertices; i++){
			if(matrix[vertex][i] < min_len && matrix[vertex][i] > 0){
				min_len  = matrix[vertex][i];
				if(e == NULL)
					e = new Edge;
				e->setFromVertex(vertex);
				e->setToVertex(i);
				e->setLen(min_len);
			}
		}
	}
	return e;
}
*/


Cluster curr_cluster;
void *do_compaction(void* arg){

	int thread_id = *((int *)arg);

	//TODO: Should find a way of doing this
	num_edges--;
	
	return NULL;
}
void *do_partial_prim(void* arg){
	sleep(3);
	int thread_id = *((int *)arg);
//	pthread_mutex_lock(&print_lock);
	cout<<"Partial prim on CPU "<<thread_id<<"\n";
	cout<<"num_edges = "<<num_edges<<"\n";
//	pthread_mutex_unlock(&print_lock);

	int i = 0;
	while(1){
		while(successor[i] != -1 && i < num_vertices){
			i++;
		}
		if(i == num_vertices)
			break;
//		pthread_mutex_lock(&print_lock);
		
		cout<<thread_id<<" : Vertex "<<i<<" unassigned.\n";

//		pthread_mutex_unlock(&print_lock);
		pthread_mutex_lock(&successor_lock[i]);
		if(successor[i] == -1){
			//set successor to urself
			successor[i] = i;
			pthread_mutex_unlock(&successor_lock[i]);
		}
		else{
			//unlock and continue
			pthread_mutex_unlock(&successor_lock[i]);
			continue;
		}

		new_cluster = new Cluster;
		new_cluster->insert(i);
//		pthread_mutex_lock(&print_lock);
		cout<<thread_id<<" : Working on vertex "<<i<<"\n";
		while(new_cluster->size() < max_subgraph_size){
			//find lightest edge from within Q to
			// outside Q
			Edge e = *(new Edge);
			new_cluster->get_lightest_out_edge(&e);
			//cannot find => break
			if(e.getLen() == -1){
				cout<<"error finding min_edge\n";
//				pthread_mutex_lock(&cluster_set_lock);
//				cluster_set->insert(*new_cluster);
//				pthread_mutex_unlock(&cluster_set_lock);

				break;
			}
//			pthread_mutex_lock(&print_lock);
			cout<<thread_id<<" : min_edge at "<<e.getLen()<<"\n";
//			pthread_mutex_unlock(&print_lock);
			//erase edge from edge_pool and include edge in global msf
			pthread_mutex_lock(&edge_transfer_lock);
			set<Edge>::iterator iter = edge_pool.find(e);
			e.print();
			Edge found_edge=*iter;
			found_edge.print();
			if(iter == edge_pool.end())
				pthread_mutex_unlock(&edge_transfer_lock);
			else{
				edge_pool.erase(*iter);
				spanning_forest.insert(e);
				pthread_mutex_unlock(&edge_transfer_lock);
			}
			
			//if successor[v] not set
			//atomically set it to i
			if(successor[e.getToVertex()] == -1){
				pthread_mutex_lock(&successor_lock[e.getToVertex()]);
				if(successor[e.getToVertex()] == -1){
					successor[e.getToVertex()] = i;
					pthread_mutex_unlock(&successor_lock[e.getToVertex()]);
					new_cluster->lock();
					new_cluster->insert(e.getToVertex());
					new_cluster->unlock();
				}
				else{
					pthread_mutex_unlock(&successor_lock[e.getToVertex()]);
					pthread_mutex_lock(&successor_lock[i]);
					successor[i] = successor[e.getToVertex()];
					pthread_mutex_unlock(&successor_lock[i]);
					break;
				}
			}
			else{
				pthread_mutex_lock(&successor_lock[i]);
				successor[i] = successor[e.getToVertex()];
				pthread_mutex_unlock(&successor_lock[i]);
				cout<<"breaking\n";
				break;
			}
		}
//		pthread_mutex_unlock(&print_lock);
//		curr_cluster.print();
		cout<<"Successor array : ";
		for(int j = 0; j < num_vertices; j++){
			cout<<successor[j]<<" ";
		}
		cout<<"\n";
		
		pthread_mutex_lock(&cluster_set_lock);
		cout<<"Inserting new cluster\n";
		new_cluster->print();
		cout<<"\n";
		pair<set<Cluster>::iterator,bool> p = cluster_set->insert(*new_cluster);
		cout<<p.second<<"\n";
		cout<<cluster_set->size()<<"\n";
		pthread_mutex_unlock(&cluster_set_lock);

		delete new_cluster;
		
	}
	return NULL;
}


void do_pma(){
	/*
	pthread_barrier_t *barrier1 = new pthread_barrier_t;
	pthread_barrier_init(barrier1, NULL, num_cores);

	pthread_barrier_t *barrier2 = new pthread_barrier_t;
	pthread_barrier_init(barrier2, NULL, num_cores);
	*/

	cout<<"do_pma : num_cores = "<<num_cores<<"\n";
	int i=0;
	while(num_edges >= 0){
		i++;
		successor = new long[num_vertices];
		successor_lock = new pthread_mutex_t[num_vertices];
		int i = 0;
		cluster_set = new set<Cluster>();
		long potential_subgraph_size = num_vertices / num_cores;
		if(potential_subgraph_size > 100)
			max_subgraph_size = potential_subgraph_size;
		else
			max_subgraph_size = 100;
		if(num_vertices < max_subgraph_size)
			max_subgraph_size = num_vertices;

		for(long i = 0; i < num_vertices; i++){
			successor[i] = -1;
			pthread_mutex_init(&successor_lock[i], NULL);
		}
		for(int i = 0; i < num_cores; i++){
			int n = i;
			pthread_create(&cputhreads[i], NULL, do_partial_prim, (void*) &n);
//			pthread_mutex_lock(&print_lock);
			cout<<"Created thread on CPU "<<n<<"\n";
//			pthread_mutex_unlock(&print_lock);

		}
		//do_partial_prim(num_vertices, num_edges, num_cores);
		//pthread_barrier_wait(barrier1);
		for(int i = 0; i < num_cores; i ++)
			pthread_join(cputhreads[i], NULL);

		cout<<"Partial prim done\n";
		
		for(int i = 0; i < num_cores; i++){
			int n = i;
			pthread_create(&cputhreads[i], NULL, do_compaction, (void*) &n);
		}
		//pthread_barrier_wait(barrier2);
		for(int i = 0; i < num_cores; i ++)
			pthread_join(cputhreads[i], NULL);

		cout<<"Iteration "<< i <<" done."<<cluster_set->size()<<" clusters created. Cluster Set : \n";
		set<Cluster>::iterator iter = cluster_set->begin();
		int j = 0;
		while(iter != cluster_set->end()){
			Cluster c = *iter;
			cout<<"Cluster "<<j<<" : ";
			set<long>::iterator it = c.get_vertices().begin();
			while(it != c.get_vertices().end()){
				cout<<*it<<" ";
				it++;
			}
			cout<<"\n";
			j++;
			iter++;
		}
		delete successor_lock;
		delete successor;
		delete cluster_set;
	}
}


int main(int argc, char* argv[]){
	printf("Welcome to this jaffa program\n");
	pthread_mutex_init(&print_lock, NULL);
	//get number of cores on which to run
	num_cores = get_num_cores(argc, argv);
	if(num_cores == -1)
		return -1;
	cout<<"Running on "<<num_cores<<" cores...\n";
	
	//initialize pthread array
	cputhreads = new pthread_t[num_cores];

	//included from MSF.h
	num_vertices = 0;
	num_edges = 0;

	//read edges from file and prepare the distance matrix
	ifstream input_file;
	input_file.open(argv[2]);
	cout<<"Parsing the input file...\n";
	matrix = process_inputs(&input_file);
	if(matrix == NULL){
		cout<<"Error parsing input file.\n";
		return -1;
	}

	//DEBUG: print the distance matrix
	cout<<"Matrix prepared:\n";
	for(int i = 0; i < num_vertices; i ++){
		for(int j = 0; j < num_vertices; j++)
			cout<<matrix[i][j]<<"\t";
		cout<<"\n";
	}

	//close the input stream
	if(input_file.is_open())
		input_file.close();

	//initialise the edge_transfer_lock
	pthread_mutex_init(&edge_transfer_lock, NULL);

	//call do_pma to find the msf concurrently
	do_pma();
	
	delete matrix;
	delete cputhreads;
	return 0;
	
	
}