// g++ Barriers.cpp -lpthread
// ./a.out

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
#include <fstream>
#include <unistd.h>

using namespace std;

vector<int> A;
vector<int> IA;
vector<int> JA;
vector<int> out_edges;
vector<int> no_out_edges;
vector<double> page_rank_prev, page_rank;
int nodes = 0, max_nodes = 400000;
double error = 1.000, thershold = 0.0000000000000001, damping_factor = 0.85;
int start_node = 0;
int iterations = 0;
int iterationThreshold = 10;

void print_vector(vector<double> vec) {
	for (int i = start_node; i < vec.size(); i++) {
		cout << vec[i] << ",";
	}
	cout << "\n";
}

void compute_page_rank() {
    double error = 1;
	while(error > thershold) {
	//while (iterations < iterationThreshold) {
        error = 0;
		for (int i = start_node; i <= nodes; i++) {
			page_rank[i] = (1 - damping_factor) / (nodes+1);
			for (int j = IA[i - 1]; j < IA[i]; j++) {
				page_rank[i] += (page_rank_prev[JA[j]] / out_edges[JA[j]]) * (damping_factor);
			}
			error = max(error, fabs(page_rank_prev[i] - page_rank[i]));
		}
		//cout << "iterations: " << iterations << "\n";
		//print_vector(page_rank);
		iterations++;
		page_rank_prev = page_rank;
	}
}

bool comparePair(pair<double, double> i1, pair<double, double> i2)
{
	return (i1.first > i2.first);
}


int main(int argc, char** argv)
{
	srand((unsigned)time(NULL));
	string filename = "", line;
	if (argc == 5) {
		filename = argv[1];
		start_node = atoi(argv[2]);
		max_nodes = atoi(argv[3]);
		iterationThreshold = atoi(argv[4]);
	}
	else {
		cout << "Check the arguments\n";
		return 0;
	}

	fstream file;
	file.open(filename.c_str());
	IA.resize(max_nodes, 0);
	out_edges.resize(max_nodes, 0);

	while (getline(file, line)) {
		istringstream iss(line);
		vector<int> tokens{ istream_iterator<int>{iss}, istream_iterator<int>{} };
		A.push_back(1);
		JA.push_back(tokens[1]);
		IA[tokens[0]]++;
		out_edges[tokens[1]]++;
		nodes = max(nodes, tokens[0]);
		nodes = max(nodes, tokens[1]);
	}
	file.close();

	for (int i = 1; i < IA.size(); i++) {
		IA[i] = IA[i] + IA[i - 1];
	}
        

	page_rank_prev.resize(nodes + 1, 1.0000 / (nodes + 1));
	page_rank.resize(nodes + 1, 0);


	struct timeval startwtime, endwtime;
	gettimeofday(&startwtime, NULL);

	//print_vector(page_rank);
	compute_page_rank();

	gettimeofday(&endwtime, NULL);

	double time = (double)((endwtime.tv_usec - startwtime.tv_usec) / 1.0e6
		+ endwtime.tv_sec - startwtime.tv_sec);

	// double sum = 0;
	// for(int i=start_node;i<=nodes;i++) sum = sum + page_rank[i];

	string outFileName = __FILE__;
	size_t lastindex = outFileName.find_last_of(".");
	size_t firstindex = outFileName.find_last_of("/\\");
	outFileName = outFileName.substr(firstindex + 1, lastindex);
	outFileName += "_out.txt";
	outFileName = "../../Output/" + outFileName;

	cout << outFileName << " Num of threads: 1 " << " Input File: " << filename << endl;
	cout << "Time : " << time << "\n";
	cout << "Iterations : " << iterations << "\n";

	std::ofstream outFile(outFileName);

	outFile << time << "\n";
	outFile << iterations << "\n";

	// outFile << sum << "\n";
	for (int i = start_node; i <= nodes; i++) {
		outFile << page_rank[i] << "\n";
	}
}
