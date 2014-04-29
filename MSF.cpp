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

float** process_inputs(ifstream *input, long *num_vert, long *num_edge){

	string num_vertices_str;
	getline(*input, num_vertices_str);
//	cout<<"String : "<<num_vertices_str<<"\n";
	int num_vertices = atoi(num_vertices_str.c_str());
//	cout<<"num_vertices = "<<num_vertices<<"\n";
 *num_vert = num_vertices;

	//Allocate space for the matrix
	float** matrix = NULL;
	matrix = new float*[num_vertices];
	for(int i = 0; i < num_vertices; i++){
		matrix[i] = new float[num_vertices];
		for(int j = 0; j < num_vertices;j++)
			matrix[i][j] = 0.0;
	}
//	cout<<"Space allocated for matrix\n";
	
	while(1){
		
		string buffer;
		getline(*input, buffer);
//		cout<<"buffer = "<<buffer<<"\n";
		if(buffer.compare("done") == 0)
			break;
		(*num_edge)++;
		istringstream buff(buffer);
		istream_iterator<string> beg(buff), end;
		vector<string> tokens(beg, end);

		long from_vertex = atol(tokens[0].c_str());
		long to_vertex = atol(tokens[1].c_str());
		float len = ::atof(tokens[2].c_str());
		
//		cout<<"From vertex = "<<from_vertex<<"\n";
//		cout<<"To vertex = "<<to_vertex<<"\n";
//		cout<<"Length = "<<len<<"\n";
		if(from_vertex == 0 || to_vertex == 0){
			cout<<"Vertex numbering starts from 1. \n";
			return NULL;
		}
		else if(from_vertex > num_vertices || to_vertex > num_vertices){
			cout<<"Vertex number cannot be more than number of vertices\n";
			return NULL;
		}
		
		Edge e = new Edge(from_vertex-1, to_vertex-1, len);
		edge_pool.insert(e);
		matrix[from_vertex-1][to_vertex-1] = len;

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
void *do_compaction(void* arg){

	int thread_id = *((int *)arg);

	//TODO: Should find a way of doing this
	(*num_edges)--;
	return NULL;
}
void *do_partial_prim(void* arg){
	int thread_id = *((int *)arg);
	
	cout<<"Partial prim on CPU "<<thread_id<<"\n";
	
	int i = 0;
	while(1){
		while(successor[i] != -1 && i < num_vertices){
			i++;
		}
		if(i == num_vertices)
			break;
		//Just for seeing termination
		break;

		pthread_mutex_lock(&successor_lock[i]);
		if(successor[i] == -1){
			//set successor to urself
			successor[i] = i;
			set<long> curr_cluster;
			pthread_mutex_unlock(&successor_lock[i]);
		}
		else{
			//unlock and continue
			pthread_mutex_unlock(&successor_lock[i]);
			continue;
		}
		long set_verts[] = {i};
		Cluster curr_cluster;
		
		while(curr_cluster.size() < max_subgraph_size){
			//find lightest edge from within Q to
			// outside Q
			Edge *e = curr_cluster.get_lightest_out_edge();
			//cannot find => break
			if(e == NULL)
				break;
			//erase edge from edge_pool and include edge in global msf
			
			spanning_forest.insert(*e);
			//if successor[v] not set
			//atomically set it to i
			if(successor[e->getToVertex()] == -1){

				pthread_mutex_lock(&successor_lock[e->getToVertex()]);
				if(successor[e->getToVertex()] == -1){
					successor[e->getToVertex()] = i;
					pthread_mutex_unlock(&successor_lock[e->getToVertex()]);
					//Q = QUV
					curr_cluster.insert(e->getToVertex());
				}
				else{
					pthread_mutex_unlock(&successor_lock[e->getToVertex()]);
					pthread_mutex_lock(&successor_lock[i]);
					successor[i] = successor[e->getToVertex()];
					pthread_mutex_unlock(&successor_lock[i]);
					break;
				}
			}
			else{
				pthread_mutex_lock(&successor_lock[i]);
				successor[i] = successor[e->getToVertex()];
				pthread_mutex_unlock(&successor_lock[i]);
				break;
			}
		}
	}
	return NULL;
}

void do_pma(long num_vertices, long num_edges, int num_cores){
	
	pthread_barrier_t *barrier1 = new pthread_barrier_t;
	pthread_barrier_init(barrier1, NULL, num_cores);

	pthread_barrier_t *barrier2 = new pthread_barrier_t;
	pthread_barrier_init(barrier2, NULL, num_cores);

	while(num_edges){
		successor = new long[num_vertices];
		successor_lock = new pthread_mutex_t[num_vertices];
		long potential_subgraph_size = num_vertices / num_cores;
		if(potential_subgraph_size > 100)
			max_subgraph_size = potential_subgraph_size;
		else
			max_subgraph_size = 100;
		for(long i = 0; i < num_vertices; i++){
			successor[i] = -1;
			pthread_mutex_init(&successor_lock[i], NULL);
		}
		for(int i = 0; i < num_cores; i++){
			int n = i;
			pthread_create(&cputhreads[i], NULL, do_partial_prim, (void*) &n);
		}
		//do_partial_prim(num_vertices, num_edges, num_cores);
		//pthread_barrier_wait(barrier1);
		for(int i = 0; i < num_cores; i ++)
			pthread_join(cputhreads[i], NULL);
		for(int i = 0; i < num_cores; i++){
			int n = i;
			pthread_create(&cputhreads[i], NULL, do_compaction, (void*) &n);
		}
		//pthread_barrier_wait(barrier2);
		for(int i = 0; i < num_cores; i ++)
			pthread_join(cputhreads[i], NULL);

		delete successor_lock;
		delete successor;
	}
}
int main(int argc, char* argv[]){
	printf("Welcome to this jaffa program\n");

	int num_cores = get_num_cores(argc, argv);
	if(num_cores == -1)
		return -1;
	cout<<"Running on "<<num_cores<<" cores...\n";
// 	cputhreads = new pthread_t[num_cores];
// 	cpu_set_t* cpusets = new cpu_set_t[num_cores];
// 
// 	for(int i = 0; i < num_cores; i++){
// 		CPU_ZERO(&cpusets[i]);
// 		CPU_SET(i, &cpusets[i]);
// 	}

	long num_vertices = 0, num_edges = 0;
	ifstream input_file;
	input_file.open(argv[2]);
	cout<<"Parsing the input file...\n";
	matrix = process_inputs(&input_file, &num_vertices, &num_edges);
	if(matrix == NULL){
		cout<<"Error parsing input file.\n";
		return -1;
	}
	cout<<"Matrix prepared:\n";
	for(int i = 0; i < num_vertices; i ++){
		for(int j = 0; j < num_vertices; j++)
			cout<<matrix[i][j]<<"\t";
		cout<<"\n";
	}
	if(input_file.is_open())
		input_file.close();
	
	do_pma(num_vertices, num_edges, num_cores);
	
	delete matrix;
	delete cputhreads;
	return 0;
	
}