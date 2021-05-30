#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

#include <bitset>
#include <cstdlib>
#include <iostream>
#include <atomic>
#include <mutex>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <cmath> 
#include <sys/time.h>
#include <unistd.h>
using namespace std;

vector<int> A; 
vector<int> IA;
vector<int> JA;
vector<int> out_edges;
vector<int> no_out_edges;
vector<int> iterations;
vector<double> page_rank_prev, page_rank;
int nodes = 0, max_nodes = 400000;
double thershold= 0.0000000000000001,damping_factor=0.85;
int thdnum = 4;
pthread_barrier_t b;
int start_node = 0;
vector<double> th_sum;
int iteration_threshold = 10;
std::atomic<double> error;
vector<double> th_error;
int thd_sleep = 0;

void print_vector(vector<double> vec) {
	for(int i=start_node;i<vec.size();i++) {
		cout<<vec[i]<<" ";
	}
	cout<<"\n";
}

void *thread_page_rank(void* thdnumber) {
	int *thdid = (int *)thdnumber;
	double local_error = 1;

	local_error = error.load(std::memory_order_seq_cst);
	// while(local_error > thershold) {
	while(iterations[*thdid] < iteration_threshold) {
    	local_error = 0;
		for(int i=*thdid+start_node;i<=nodes;i=i+thdnum) {
			double temp = (1 - damping_factor)/(nodes +1), prev = page_rank[i];
			
			for(int j=IA[i-1];j<IA[i];j++) {
				temp = temp + page_rank[JA[j]]/out_edges[JA[j]] * (damping_factor);
			}
			page_rank[i] = temp;
			local_error = max(local_error,fabs(page_rank[i]-prev));
			
		}
		iterations[*thdid]++;
		th_error[*thdid] = local_error;
		for(int i=0;i<thdnum;i++) {
			local_error = max(local_error,th_error[i]);
		}

		float rand_val = (float) rand()/RAND_MAX;
	    if(*thdid < (thdnum/4) && rand_val > 0.25) {
			usleep(1000 * thd_sleep);
		}
	}

};

void compute_page_rank() {
	int iterations = 0;

	pthread_t threads[thdnum];
	int thdid[thdnum];
	pthread_barrier_init(&b, 0, thdnum);

	for (int i = 0; i < thdnum; ++i) {
		thdid[i] = i;
		pthread_create(&threads[i], NULL, thread_page_rank, (void *)&thdid[i]);
	}

	for (int i = 0; i < thdnum; ++i) {
		pthread_join(threads[i], NULL);
	}
	pthread_barrier_destroy(&b);
}

bool comparePair(pair<double,double> i1, pair<double,double> i2) 
{ 
    return (i1.first > i2.first); 
} 


int main(int argc, char** argv)
//int main()
{
	//string filename = "Notre.txt", line;
	string filename = "", line;
	if(argc == 7) {
		filename = argv[1];
		thdnum = atoi(argv[2]);
		start_node = atoi(argv[3]);
		max_nodes = atoi(argv[4]);
		iteration_threshold = atoi(argv[5]);
		thd_sleep = atoi(argv[6]);
	}
	else {
		cout << "Check the arguments\n";
		return 0;
	}
	
	fstream file;
	file.open(filename.c_str());
	IA.resize(max_nodes,0);
	out_edges.resize(max_nodes,0);
	iterations.resize(thdnum,0);
	error.store(1, std::memory_order_seq_cst);
	th_error.resize(thdnum,0);

	while(getline(file, line)){
	istringstream iss(line);
	        vector<int> tokens{istream_iterator<int>{iss}, istream_iterator<int>{}};
	        A.push_back(1);
	        JA.push_back(tokens[1]);
	        IA[tokens[0]]++;
	        out_edges[tokens[1]]++;
	        nodes = max(nodes,tokens[0]);
	        nodes = max(nodes,tokens[1]);
	}
	file.close();

	for(int i=1;i<IA.size();i++) {
		IA[i] = IA[i] + IA[i-1];
	}

	page_rank.resize(nodes+1,1.0000/(nodes+1));

	struct timeval startwtime, endwtime;
    gettimeofday (&startwtime, NULL);

	compute_page_rank();

	gettimeofday (&endwtime, NULL);

	double time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
              + endwtime.tv_sec - startwtime.tv_sec);
	//cout<<"time: "<<time<<endl;
	//cout<<"Iterations: "<<iteration_threshold<<"\n";

	// double sum = 0;
	// for(int i=start_node;i<=nodes;i++) sum = sum + page_rank[i];
	int avg_itr = 0;
	for(int i=0;i<thdnum;i++) 
		avg_itr = avg_itr + iterations[i];
	avg_itr = avg_itr/thdnum;

	string outFileName = __FILE__;
	size_t lastindex = outFileName.find_last_of(".");
	size_t firstindex = outFileName.find_last_of("/\\");
	outFileName = outFileName.substr(firstindex + 1, lastindex);
	outFileName += "_out.txt";
	outFileName = "../../Output/" + outFileName;

	cout << outFileName << " Num of threads: " << to_string(thdnum) << " Input File: " << filename << endl;
	cout << "Time : " << time << "\n";
	cout << "Iterations : " << avg_itr << "\n";

	std::ofstream outFile(outFileName);

	outFile << time << "\n";
	outFile << avg_itr << "\n";

	// outFile << sum << "\n";
	for (int i = start_node; i <= nodes; i++) {
		outFile << page_rank[i] << "\n";
	}
};
