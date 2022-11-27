#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

using namespace std;




vector<int> IA;
vector<int> JA;
vector<int> degrees;
vector<int> iterations;
vector<double> pr;

int FIRST_NODE  = 1;
int LAST_NODE   = 0;
int MAX_NODES   = 10000000;
int NUM_THREADS = 12;
int MAX_ITERATIONS    = 500;
double TOLERANCE      = 1E-10;
double DAMPING_FACTOR = 0.85;
vector<double> th_error;
pthread_barrier_t barrier;




void* thread_pagerank(void *data) {
  int th_id = *((int*) data);
  double err = 1;
  while (err > TOLERANCE) {
    err = 0;
    for (int i=th_id+FIRST_NODE; i<=LAST_NODE; i+=NUM_THREADS) {
      double temp = (1-DAMPING_FACTOR) / (LAST_NODE+1);
      double prev = pr[i];
      for (int j=IA[i-1]; j<IA[i]; j++)
        temp += pr[JA[j]] / degrees[JA[j]] * DAMPING_FACTOR;
      pr[i] = temp;
      err  = max(err, fabs(pr[i] - prev));
    }
    iterations[th_id]++;
    th_error  [th_id] = err;
    for (int i=0; i<NUM_THREADS; i++)
      err = max(err, th_error[i]);
  }
  return NULL;
}


void compute_pagerank() {
  int        th_ids[NUM_THREADS];
  pthread_t threads[NUM_THREADS];
  pthread_barrier_init(&barrier, 0, NUM_THREADS);
  for (int i=0; i<NUM_THREADS; ++i) {
    th_ids[i] = i;
    pthread_create(&threads[i], NULL, thread_pagerank, (void*) &th_ids[i]);
  }
  for (int i=0; i<NUM_THREADS; ++i)
    pthread_join(threads[i], NULL);
  pthread_barrier_destroy(&barrier);
}


int main(int argc, char **argv) {
  string line, filename = argv[1];
  fstream file;
  file.open(filename.c_str());
  IA        .resize(MAX_NODES, 0);
  degrees   .resize(MAX_NODES, 0);
  iterations.resize(NUM_THREADS, 0);
  th_error  .resize(NUM_THREADS, 0);
  cout << "Loading " << filename << " ...\n";
  while (getline(file, line)) {
    istringstream iss(line);
    int u = 0, v = 0;
    iss >> v >> u;
    JA.push_back(u);
    IA[v]++;
    degrees[u]++;
    LAST_NODE = max(LAST_NODE, v);
    LAST_NODE = max(LAST_NODE, u);
  }
  file.close();
  for (int i=1; i<IA.size(); i++)
    IA[i] = IA[i] + IA[i-1];
  pr.resize(LAST_NODE+1, 1.0/(LAST_NODE+1));
  struct timeval startwtime, endwtime;
  gettimeofday(&startwtime, NULL);
  compute_pagerank();
  gettimeofday(&endwtime,   NULL);
  double time = double((endwtime.tv_usec - startwtime.tv_usec)*1E-3 + (endwtime.tv_sec - startwtime.tv_sec)*1E+3);
  int avg_itr = 0;
  for (int i=0; i<NUM_THREADS; i++)
    avg_itr = avg_itr + iterations[i];
  avg_itr = avg_itr / NUM_THREADS;
  cout << "Time: " << time << " ms ";
  cout << "Iterations: " << avg_itr << "\n\n";
}
