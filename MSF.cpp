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
	for(int i = 0; i < num_vertices; i++)
		matrix[i] = new float[num_vertices];
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

		matrix[from_vertex][to_vertex] = len;

	}

	return matrix;
}

void do_compaction(long *num_vertices, long* num_edges, int num_cores){
	(*num_edges)--;
	return;
}
void *do_partial_prim(void* arg){
	argPacket1 *my_arg = (argPacket1 *)arg;

	long num_vertices = my_arg->getVertices();
	long num_edges = my_arg->getEdges();
	int num_cores = my_arg->getCores();
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
			pthread_mutex_unlock(&successor_lock[i]);
		}
		else{
			//unlock and continue
			pthread_mutex_unlock(&successor_lock[i]);
			continue;
		}
		long set_verts[] = {i};
		set<long> Q(set_verts, set_verts + 1);
		while(Q.size() < max_subgraph_size){
			//find lightest edge from within Q to
			// outside Q

			//cannot find => break

			//include edge in global msf

			//if successor[v] not set
			//atomically set it
			//Q = QUV

			//else
			//successor[i] = successor[v];
			//break;
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
		argPacket1 *arg = new argPacket1(num_vertices, num_edges, num_cores);
		for(int i = 0; i < num_cores; i++){
			pthread_create(&cputhreads[i], NULL, do_partial_prim, (void*) &arg);
		}
		//do_partial_prim(num_vertices, num_edges, num_cores);
		//pthread_barrier_wait(barrier1);
		for(int i = 0; i < num_cores; i ++)
			pthread_join(cputhreads[i], NULL);
		do_compaction(&num_vertices, &num_edges, num_cores);
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
	cout<<"Running on "<<num_cores<<" cores...\n";
	if(num_cores == -1)
		return -1;
	cputhreads = new pthread_t[num_cores];
	cpu_set_t* cpusets = new cpu_set_t[num_cores];

	for(int i = 0; i < num_cores; i++){
		CPU_ZERO(&cpusets[i]);
		CPU_SET(i, &cpusets[i]);
	}
	long num_vertices = 0, num_edges = 0;
	ifstream input_file;
	input_file.open(argv[2]);
	cout<<"Parsing the input file...\n";
	matrix = process_inputs(&input_file, &num_vertices, &num_edges);
	cout<<"Matrix prepared\n";
	if(input_file.is_open())
		input_file.close();
	
	do_pma(num_vertices, num_edges, num_cores);
	
	delete matrix;
	delete cputhreads;
	return 0;
}