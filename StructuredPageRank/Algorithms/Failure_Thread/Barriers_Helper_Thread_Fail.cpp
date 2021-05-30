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
#include <signal.h>
using namespace std;

struct multi_atomic_var {
  int iteration;
  int current_node;
  double thd_error;
  double global_pg_no_out_edges_td;
};

struct multi_atomic_var1 {
  int iteration;
  double glb_error;
  bool intermediate;
  double pg_no_out_edges_val;
  vector<bool> td_check;
};

struct pr_struct {
  int iteration;
  double rank;
};

vector<int> A; 
vector<int> IA;
vector<int> JA;
vector<int> out_edges;
vector<int> no_out_edges;
// vector<int> iterations;
int nodes = 0, max_nodes = 400000;
double thershold= 0.0000000000000001,damping_factor=0.85;
int thdnum = 4;
pthread_barrier_t b;
int start_node = 0;
int iteration_threshold = 10;
int thd_fail = 0;
std::atomic<double> pg_no_out_edges;

std::atomic<int> glb_iter_count;
std::atomic<double> error;
std::atomic<std::atomic<multi_atomic_var *> *> global_coupled_variable;
std::atomic<multi_atomic_var1 *> pg_val;
std::mutex mtx;

struct pg_memory_op {
  int iteration;
  std::atomic<std::atomic<pr_struct *> *> page_rank_prev;
  std::atomic<std::atomic<pr_struct *> *> page_rank;
};

std::atomic<pg_memory_op *> glb_memory_var;
std::atomic<std::atomic<pr_struct *> *> pr;
std::atomic<std::atomic<pr_struct *> *> prev_pr;

struct timeval startwtime, endwtime;
std::atomic<bool> time_bool;

void print_vector(vector<double> vec) {
  for(int i=start_node;i<vec.size();i++) {
    cout<<vec[i]<<" ";
  }
  cout<<"\n";
}

void compute_PR(int threadId, int helperTid, multi_atomic_var1 *local_error_) {
  multi_atomic_var *old_values = global_coupled_variable[threadId].load(std::memory_order_seq_cst);
  int i = old_values->current_node;
  double local_pg_no_out = 0.0;
  local_pg_no_out = pg_val.load(std::memory_order_seq_cst)->pg_no_out_edges_val;
  
  while(i <= nodes && local_error_->iteration == old_values->iteration) {
    pr_struct *temp_ptr = prev_pr[i].load(std::memory_order_seq_cst);
    double local_prev = temp_ptr->rank;
    double pr_buffer = (1-damping_factor)/nodes;

    for(int j=IA[i-1];j<IA[i];j++) {
      pr_buffer += (prev_pr[JA[j]].load(std::memory_order_seq_cst)->rank/out_edges[JA[j]]) * (damping_factor);
    }
    pr_buffer += local_pg_no_out * (1-damping_factor);

    pr_struct *pg_old = pr[i].load(std::memory_order_seq_cst);
    if(pg_old->iteration == local_error_->iteration) {
      pr_struct *pg_new = new pr_struct();
      pg_new->iteration = local_error_->iteration + 1;
      pg_new->rank = pr_buffer;
      if(pr[i].compare_exchange_strong(pg_old, pg_new)) delete(pg_old);
      //pr[i].compare_exchange_strong(pg_old, pg_new);
    }

    pr_struct *prev_pr_old = prev_pr[i].load(std::memory_order_seq_cst);
    if(prev_pr_old->iteration == local_error_->iteration) {
      pr_struct *prev_pr_new = new pr_struct();
      prev_pr_new->iteration = local_error_->iteration + 1;
      prev_pr_new->rank = prev_pr_old->rank + 0;
      if(prev_pr[i].compare_exchange_strong(prev_pr_old, prev_pr_new)) delete(prev_pr_old);
      //prev_pr[i].compare_exchange_strong(prev_pr_old, prev_pr_new);
    }

    while(true) {
      multi_atomic_var *old_gcv = global_coupled_variable[threadId].load(std::memory_order_seq_cst);
      if(old_gcv->iteration == local_error_->iteration && old_gcv->current_node == i) {
        multi_atomic_var *new_gcv = new multi_atomic_var();
        new_gcv->iteration = local_error_->iteration;
        new_gcv->current_node = i + thdnum;
        new_gcv->thd_error = max(old_gcv->thd_error, pr_buffer - local_prev);
        if(out_edges[i] == 0) {
          new_gcv->global_pg_no_out_edges_td = old_gcv->global_pg_no_out_edges_td +  ((pr_buffer - local_prev)/nodes);
        }
        else {
          new_gcv->global_pg_no_out_edges_td = old_gcv->global_pg_no_out_edges_td + 0;
        }
        if(global_coupled_variable[threadId].compare_exchange_strong(old_gcv, new_gcv)) {
          //delete(old_gcv);
          break;
        }
      }
      else {
        break;
      }
    }

    old_values = global_coupled_variable[threadId].load(std::memory_order_seq_cst);
    i = old_values->current_node;
  }
}

void update_shared_variables(int threadId, int helperTid, multi_atomic_var1 *local_error_) {
  while(true) {
    multi_atomic_var1 *old_pg_val = pg_val.load(std::memory_order_seq_cst);
    if(old_pg_val->iteration != local_error_->iteration || old_pg_val->td_check[threadId] == true) {
      break;
    }
    multi_atomic_var1 *new_pg_val = new multi_atomic_var1();
    new_pg_val->iteration = local_error_->iteration;
    new_pg_val->td_check = old_pg_val->td_check;
    new_pg_val->td_check[threadId] = true;
    new_pg_val->intermediate = true;
    new_pg_val->pg_no_out_edges_val = old_pg_val->pg_no_out_edges_val + global_coupled_variable[threadId].load(std::memory_order_seq_cst)->global_pg_no_out_edges_td;
    new_pg_val->glb_error = max(old_pg_val->glb_error, global_coupled_variable[threadId].load(std::memory_order_seq_cst)->thd_error);

    if(pg_val.compare_exchange_strong(old_pg_val, new_pg_val)) {
      break;
    }
  }

  while(true) {
    multi_atomic_var *old_values = global_coupled_variable[threadId].load(std::memory_order_seq_cst);
    if(old_values->iteration == local_error_->iteration) {
      multi_atomic_var *new_values = new multi_atomic_var();
      new_values->iteration = local_error_->iteration + 1;
      new_values->current_node = threadId + start_node;
      new_values->global_pg_no_out_edges_td = 0.0;
      new_values->thd_error = 0.0;
      if(global_coupled_variable[threadId].compare_exchange_strong(old_values, new_values))
      {
        //delete(old_values);
        break;
      }
        
    }
    else {
      break;
    }
  }

}

void *thread_page_rank(void* thdnumber) {
  int *thdid = (int *)thdnumber;
  double iteration_error = 10000;
  multi_atomic_var1 *local_pg_val = pg_val.load(std::memory_order_seq_cst);

  int counter = 0;
  while(iteration_error > thershold || local_pg_val->intermediate == true) {
  //while(local_pg_val->iteration < iteration_threshold || local_pg_val->intermediate == true) {
    // cout<<"Err: "<<iteration_error<<"\n";
    multi_atomic_var1 *local_error_ = pg_val.load(std::memory_order_seq_cst);

    compute_PR(*thdid, *thdid, local_error_);
    for(int i = 0; i < thdnum; i++) {
      if(*thdid != i) {
        compute_PR(i, *thdid, local_error_);
      }
    }

    multi_atomic_var1 *pre_temp = pg_val.load(std::memory_order_seq_cst);
    if(pre_temp->iteration == local_error_->iteration) {
      multi_atomic_var1 *new_values = new multi_atomic_var1();
      new_values->iteration = local_error_->iteration;
      new_values->pg_no_out_edges_val = pre_temp->pg_no_out_edges_val + 0;
      new_values->glb_error = 0.0;
      new_values->td_check.resize(thdnum, false);
      new_values->intermediate = true;
      pg_val.compare_exchange_strong(pre_temp, new_values);
    }

    update_shared_variables(*thdid, *thdid, local_error_);
    for(int i = 0; i < thdnum; i++) {
      if(*thdid != i) {
        update_shared_variables(i, *thdid, local_error_);
      }
    }

    multi_atomic_var1 *temp1 = pg_val.load(std::memory_order_seq_cst);
    if(temp1->iteration == local_error_->iteration) {
      multi_atomic_var1 *new_values = new multi_atomic_var1();
      new_values->iteration = local_error_->iteration + 1;
      new_values->pg_no_out_edges_val = temp1->pg_no_out_edges_val + 0;
      new_values->td_check.resize(thdnum, false);
      new_values->glb_error = temp1->glb_error;
      new_values->intermediate = false;
      pg_val.compare_exchange_strong(temp1, new_values);
    }
    
    local_pg_val = pg_val.load(std::memory_order_seq_cst);
    iteration_error = temp1->glb_error;

    pg_memory_op *temp3 = glb_memory_var.load(std::memory_order_seq_cst);
    if(temp3->iteration == local_error_->iteration) {
      pg_memory_op *temp_struct2 = new pg_memory_op();
      temp_struct2->page_rank_prev.store(pr, std::memory_order_seq_cst);
      temp_struct2->page_rank.store(prev_pr, std::memory_order_seq_cst);
      temp_struct2->iteration = local_error_->iteration + 1;
      glb_memory_var.compare_exchange_strong(temp3, temp_struct2);
    }

    prev_pr.store(glb_memory_var.load(std::memory_order_seq_cst)->page_rank_prev.load(std::memory_order_seq_cst));
    pr.store(glb_memory_var.load(std::memory_order_seq_cst)->page_rank.load(std::memory_order_seq_cst));

    // float rand_val = (float) rand()/RAND_MAX;
    // if(rand_val > 0.25) {
    //   usleep(5000000);
    // }
    
    if(*thdid < thd_fail) {
      pthread_exit(NULL);
    }
  
  }
  bool temp = false;
  if(time_bool.compare_exchange_strong(temp,true)) gettimeofday (&endwtime, NULL);
};

void compute_page_rank() {

  pthread_t threads[thdnum];
  int thdid[thdnum];
  
  for (int i = 0; i < thdnum; ++i) {
    thdid[i] = i;
    pthread_create(&threads[i], NULL, thread_page_rank, (void *)&thdid[i]);
  }

  for (int i = 0; i < thdnum; ++i) {
    pthread_join(threads[i], NULL);
  }
}

bool comparePair(pair<double,double> i1, pair<double,double> i2) 
{ 
    return (i1.first > i2.first); 
} 

void initialize_global_struct() {
  double local_pg_no_out = 0.0;
  double temp_var = (1.0000/(nodes+1))/nodes;

  for(int i = 0; i < thdnum; i++) {
    for(int j=i+start_node;j<=nodes;j=j+thdnum) {
      pr_struct *temp1 = new pr_struct();
      temp1->iteration = 1;
      temp1->rank = 0;
      pr[j].store(temp1, std::memory_order_seq_cst);

      pr_struct *temp2 = new pr_struct();
      temp2->iteration = 1;
      temp2->rank = 1.0000/(nodes+1);
      prev_pr[j].store(temp2, std::memory_order_seq_cst);

      if(out_edges[j] == 0) {
        local_pg_no_out += temp_var;
      }
    }
    multi_atomic_var *temp1 = new multi_atomic_var();
    temp1->iteration = 1;
    temp1->current_node = i + start_node;
    temp1->thd_error = 0.0;
    temp1->global_pg_no_out_edges_td = 0.0;
    global_coupled_variable[i].store(temp1, std::memory_order_seq_cst);

  }

  pg_no_out_edges.store(local_pg_no_out, std::memory_order_seq_cst);
  
  multi_atomic_var1 * temp2 = new multi_atomic_var1();
  temp2->iteration = 1;
  temp2->pg_no_out_edges_val = local_pg_no_out;
  temp2->td_check.resize(thdnum, false);
  temp2->glb_error = 0;
  temp2->intermediate = false;
  pg_val.store(temp2, std::memory_order_seq_cst);
}


int main(int argc, char** argv)
{
  srand( (unsigned)time( NULL ) );

  string filename = "", line;
  if(argc == 7) {
    filename = argv[1];
    thdnum = atoi(argv[2]);
    start_node = atoi(argv[3]);
    max_nodes = atoi(argv[4]);
    iteration_threshold = atoi(argv[5]);
    thd_fail = atoi(argv[6]);
  }
  else {
    cout << "Check the arguments\n";
    return 0;
  }

  fstream file;
  file.open(filename.c_str());
  IA.resize(max_nodes,0);
  out_edges.resize(max_nodes,0);
  // iterations.resize(thdnum,0);
  // error.store(100000, std::memory_order_seq_cst);
  glb_iter_count.store(1, std::memory_order_seq_cst);

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

  global_coupled_variable = new std::atomic<multi_atomic_var *>[thdnum];

  prev_pr = new std::atomic<pr_struct *>[nodes + 1];
  pr = new std::atomic<pr_struct *>[nodes + 1];

  pg_memory_op *temp_struct = new pg_memory_op();
  temp_struct->page_rank_prev.store(prev_pr, std::memory_order_seq_cst);
  temp_struct->page_rank.store(pr, std::memory_order_seq_cst);
  temp_struct->iteration = 1;

  glb_memory_var.store(temp_struct, std::memory_order_seq_cst);

  time_bool.store(false,std::memory_order_seq_cst);

  initialize_global_struct();

  gettimeofday (&startwtime, NULL);

  compute_page_rank();

  double time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
              + endwtime.tv_sec - startwtime.tv_sec);
  //cout<<"time: "<<time<<endl;
  //cout<<"Iterations: "<<iteration_threshold<<"\n";

  double sum = 0;
  for(int i=start_node;i<=nodes;i++) sum = sum + prev_pr[i].load(std::memory_order_seq_cst)->rank;
  int avg_itr = glb_memory_var.load(std::memory_order_seq_cst)->iteration - 1;
  // for(int i=0;i<thdnum;i++) avg_itr = avg_itr + iterations[i];
  // avg_itr = avg_itr/thdnum;
  
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
      outFile << prev_pr[i].load(std::memory_order_seq_cst)->rank << "\n";
  }
};
