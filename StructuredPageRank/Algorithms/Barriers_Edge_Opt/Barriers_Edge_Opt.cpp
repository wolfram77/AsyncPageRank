// g++ Barriers_Edge_Opt.cpp -lpthread -o d.out
// ./d.out

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

/***********************************
* Global Variable Declration
************************************/

vector<double> pageRankPrev, pageRank;
int totalNodes = 0, maxNodes = 400000, maxInDegree = 0;
double error = 1.000, thershold = 0.0000000000000001, dampingFactor = 0.85;
int numOfThreads = 4;

pthread_barrier_t b;
int startNode = 0;
vector<double> threadError, threadSum;
int iterations = 0;
mutex mtx;
int iterationThreshold = 10;

vector<vector<int>> vertexInLinks;
vector<vector<int>> vertexOutLinks;
vector<double> contributionList;
vector<int> offsetList;
vector<int> binIndexList;
int binCount = 1;
struct nodeInfo 
{
	int start;
	int end;
};
vector<nodeInfo *> inLinks, outLinks;

/**********************************
* 
* Helper Print Functions
* 
***********************************/

template <typename T>
void printVector(const vector<T>& vec, int startNode)
{
	for (T i = startNode; i < vec.size(); i++)
	{
		cout << "\n" << i << ": " << vec[i] << " ";
	}
	cout << "\n";
}


void printLinks(const vector<nodeInfo>& vec, int startNode)
{
	for (int i = startNode; i < vec.size(); i++)
	{
		cout << "\n" << i << ": " << "start " << vec[i].start << " " << "end " << vec[i].end;
	}
	cout << "\n";
}

void printVectorOfVector(const vector<vector<int>>& matrix, int startNode)
{
	for (int i = startNode; i < matrix.size(); i++)
	{
		cout << i << ": ";
		for (int j = 0; j < matrix.at(i).size(); j++)
		{
			cout << matrix[i][j] << ", ";
		}
		cout << endl;
	}
	cout << endl;
}


/****************************
* Method Implementations
*****************************/

void* threadPageRank(void* thdnumber) 
{
	int* thdid = (int*)thdnumber;
	while(error>thershold) {
	//while (iterations < iterationThreshold){
		for (int u = *thdid + startNode; u <= totalNodes; u = u + numOfThreads)
        {
            if (vertexOutLinks[u].size() == 0)
				continue;
            double contribution = pageRank[u] / vertexOutLinks[u].size();
			double prevContribution = pageRankPrev[u] / vertexOutLinks[u].size();

			//cout << endl << " contribution = " << contribution << " prevContribution = " << prevContribution << endl;

			if (contribution == prevContribution)
				continue;

			double logValue = log2(pow(10, -11)) - log2(fabs(contribution - prevContribution));

			int binNo = fabs(logValue) / 2;
			cout << "\n For Vertex: " << u; 
			cout << "\n Bin No = " << binNo;
			int outListIndex = binIndexList[(u * binCount) + binNo];
			cout << "\n outListIndex = " << outListIndex;

            for (unsigned v = outListIndex; v <= outLinks[u]->end; v++)
            {
				contributionList[offsetList[v]] = contribution;
            }
        }
		pthread_barrier_wait(&b); 
		threadError[*thdid] = 0;
		for (int u = *thdid + startNode; u <= totalNodes; u = u + numOfThreads) 
		{
			double sum = 0;
			for (unsigned v = inLinks[u]->start; v <= inLinks[u]->end; v++) {
				sum = sum + contributionList[v];
			}
			pageRank[u] = ((1 - dampingFactor) / (totalNodes+1)) + ((dampingFactor) * sum);
			threadError[*thdid] = max(threadError[*thdid], fabs(pageRank[u] - pageRankPrev[u]));
		}
		pthread_barrier_wait(&b);

		if (*thdid == 0) {
			iterations++;
			error = 0;
			for (int i = 0; i < numOfThreads; i++)
				error = max(error, threadError[i]);
			pageRankPrev = pageRank;
		}
		pthread_barrier_wait(&b);
	}
	return NULL;
};

void computePageRank() 
{
	int iterations = 0;

	pthread_t threads[numOfThreads];
	int thdid[numOfThreads];
	pthread_barrier_init(&b, 0, numOfThreads);
	threadError.resize(numOfThreads, 0);

	for (int i = 0; i < numOfThreads; ++i) {
		thdid[i] = i;
		pthread_create(&threads[i], NULL, threadPageRank, (void*)&thdid[i]);
	}

	for (int i = 0; i < numOfThreads; ++i) {
		pthread_join(threads[i], NULL);
	}
	pthread_barrier_destroy(&b);
}

bool comparePair(pair<double, double> i1, pair<double, double> i2)
{
	return (i1.first > i2.first);
}

void buildLinks()
{
	int counter1 = 0;
	int counter2 = 0;
	int counter3 = 0;
	int counter4 = 0;

	inLinks.resize(totalNodes + 1);
	outLinks.resize(totalNodes + 1);
	for (int u = startNode; u <= totalNodes; u++)
	{
		counter2 = counter2 + vertexInLinks[u].size();
		nodeInfo *tempNode1 = new nodeInfo();
		tempNode1->start = counter1;
		tempNode1->end = counter2 - 1;
		inLinks[u] = tempNode1;
		counter1 = counter2;

		counter4 = counter4 + vertexOutLinks[u].size();
		nodeInfo *tempNode2 = new nodeInfo();
		tempNode2->start = counter3;
		tempNode2->end = counter4 - 1;
		outLinks[u] = tempNode2;
		counter3 = counter4;
	}
}

//Need to fix this.
void sortVertexOutLinks()
{
	for (int u = startNode; u <= totalNodes; u++)
	{
		vector<int> outLinks, inDegree;
		outLinks = vertexOutLinks[u];
		for (int v : outLinks)
			inDegree.push_back(vertexInLinks[v].size());
		//cout << "\nFor Vertex: " << u << endl;
		//cout << "\nBefore InDegree:\n";
		//printVector(inDegree, startNode); 

		//cout << "\nBefore OutLinks:\n";
		//printVector(outLinks, startNode);

		for (int i = 1; i < outLinks.size(); i++)
		{
			int tempInDegree = inDegree[i];
			int tempOutLink = outLinks[i];
			//cout << endl;
			//cout << " tempInDegree: " << tempInDegree << " tempOutLink: " << tempOutLink << endl;
			int j = i - 1;
			while (j >= 0 && inDegree[j] > tempInDegree)
			{
				inDegree[j + 1] = inDegree[j];
				outLinks[j + 1] = outLinks[j];
				j--;
			}
			inDegree[j + 1] = tempInDegree;
			outLinks[j + 1] = tempOutLink;				
		}
		vertexOutLinks[u] = outLinks;

		//cout << "\nAfter InDegree\n";
		//printVector(inDegree, startNode);

		//cout << "\nAfter OutLinks\n";
		//printVector(outLinks, startNode);
	}
}

void* threadOffSetList(void* thdnumber)
{
	int* thdid = (int*)thdnumber;
	for (int u = *thdid + startNode; u <= totalNodes; u = u + numOfThreads)
	{
		int counter1 = 0;
		for (int v : vertexInLinks[u])
		{
			int counter2 = 0;
			for (int p : vertexOutLinks[v])
			{
				if (p == u)
				{
					int indexPos = outLinks[v]->start + counter2;
					offsetList[indexPos] = inLinks[u]->start + counter1;
					break;
				}
				counter2++;
			}
			counter1++;
		}
	}
	return NULL;
}

void buildOffSetList() 
{
	pthread_t threads[numOfThreads];
	int thdid[numOfThreads];

	for (int i = 0; i < numOfThreads; ++i)
	{
		thdid[i] = i;
		pthread_create(&threads[i], NULL, threadOffSetList, (void*)&thdid[i]);
	}

	for (int i = 0; i < numOfThreads; ++i)
	{
		pthread_join(threads[i], NULL);
	}
}

void buildBinIndexList()
{
	for (int u = startNode; u <= totalNodes; u++)
	{
		int start            = outLinks[u]->start;
		int end              = outLinks[u]->end;
		int binStart         = u * binCount;
		vector<int> outLinks = vertexOutLinks[u];
		vector<int> inDegree;

		for (int v : outLinks)
			inDegree.push_back(vertexInLinks[v].size());

		binIndexList[binStart] = start;
		int counter1 = 1;

		for (int i = 0; i < outLinks.size(); i++)
		{
			if (inDegree[i] <  pow(2,counter1) )
				continue;
			else
			{
				if ((start + i + 1) < offsetList.size())
					binIndexList[binStart + counter1] = start + i + 1;
				else
					binIndexList[binStart + counter1] = start + i;
				counter1++;
			}
		}

		for (int i = counter1; i < binCount; i++)
			binIndexList[binStart + i] = binIndexList[binStart + counter1 - 1];

	}

}

/******************************************************************************
* 
* Input Parameter Definitions
* string filename        - input filename that has the graph dataset.
* int numOfThreads       - number of threads to use. Use 1 for sequntial run.
* int startNode          - the first vertex number.
* int maxNodes           - the number of vertices in the data set. 
* int iterationThreshold - threshold an ideal value can be between 15 to 20.
* 
******************************************************************************/
int main(int argc, char** argv)
{
	srand((unsigned)time(NULL));
	string filename = "", line;
	if (argc == 6) 
	{
		filename = argv[1];
		numOfThreads = atoi(argv[2]);
		startNode = atoi(argv[3]);
		maxNodes = atoi(argv[4]);
		iterationThreshold = atoi(argv[5]);
	}
	else 
	{
		cout << "Check the arguments\n";
		return 0;
	}

	fstream file;
	file.open(filename.c_str());

	vertexInLinks.resize(maxNodes);
	vertexOutLinks.resize(maxNodes);

	int edgesCount = 0;
	while (getline(file, line)) 
	{
		istringstream iss(line);
		vector<int> tokens{ istream_iterator<int>{iss}, istream_iterator<int>{} };
		vertexInLinks[tokens[0]].push_back(tokens[1]);
		vertexOutLinks[tokens[1]].push_back(tokens[0]);

		totalNodes = max(totalNodes, tokens[0]);
		totalNodes = max(totalNodes, tokens[1]);

		int inDegree = vertexInLinks[tokens[0]].size();
		maxInDegree = max(maxInDegree, inDegree );

		edgesCount++;
	}
	file.close();

	buildLinks();

	//cout << "\nVertex OutLink before Sorting \n";
	//printVectorOfVector(vertexOutLinks, startNode);

	sortVertexOutLinks();

	//cout << "\nVertex OutLinks after Sorting \n";
	//printVectorOfVector(vertexOutLinks, startNode);

	contributionList.resize(edgesCount, 0);

	offsetList.resize(edgesCount, 0);
	buildOffSetList();

	binCount = ceil(log2(maxInDegree));
	cout << "\nMaxInDegree = " << maxInDegree << " Bincount = " << binCount << " Totalnodes= " << totalNodes+1 << " TotalEdges= " << edgesCount << endl;
	binIndexList.resize((totalNodes + 1) * binCount);
	buildBinIndexList(); 
	//cout << "\nBin Index List \n";
	//printVector(binIndexList, startNode);

	pageRankPrev.resize(totalNodes + 1, 1.0000 / (totalNodes + 1));
	pageRank.resize(totalNodes + 1, 0);

	struct timeval startwtime, endwtime;
	gettimeofday(&startwtime, NULL);

	computePageRank();

	gettimeofday(&endwtime, NULL);

	double time = (double)((endwtime.tv_usec - startwtime.tv_usec) / 1.0e6
		+ endwtime.tv_sec - startwtime.tv_sec);

	string outFileName = __FILE__;
	size_t lastindex = outFileName.find_last_of(".");
	size_t firstindex = outFileName.find_last_of("/\\");
	outFileName = outFileName.substr(firstindex + 1, lastindex);
	outFileName += "_out.txt";
	outFileName = "../../Output/" + outFileName;

	cout << outFileName << " Num of threads: " << to_string(numOfThreads) << " Input File: " << filename << endl;
	cout << "Time : " << time << "\n";
	cout << "Iterations : " << iterations << "\n";

	std::ofstream outFile(outFileName);

	outFile << time << "\n";
	outFile << iterations << "\n";
	for (int i = startNode; i <= totalNodes; i++)
	{
		outFile << pageRankPrev[i] << "\n";
	}
}
