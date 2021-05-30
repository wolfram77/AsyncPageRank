import os
import sys
import shlex
import subprocess as sp
from subprocess import Popen

programs = ["Barriers_Helper_Thread_Fail.cpp"]
threads = [56]

# Add more input files here along with start nodes and max nodes
# input_files = ["10.txt", "20.txt", "30.txt", "40.txt", "50.txt", "60.txt", "70.txt"]
#input_files = ["web-Stanford.txt", "web-Notre.txt", "web-BerkStan.txt","web-Google.txt","10.txt", "20.txt", "30.txt", "40.txt", "50.txt", "60.txt", "70.txt", "RMAT_21.txt","RMAT_22.txt","RMAT_23.txt"]
input_files = ["70.txt"]
start_nodes = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
max_nodes = [9000000,9000000,9000000,9000000,9000000,9000000,9000000,9000000,9000000,9000000,9000000,9000000,9000000,9000000]
iteration_threshold = 75

for program in programs :
	cmd1 = "g++ -std=c++11 -pthread " + program
	print cmd1
	cmd1 = Popen(cmd1.split(), stdin=sp.PIPE, stdout=sp.PIPE)
	cmd1.wait()

	for i in range (len(input_files)) :
		for tdf in [1, 2, 3, 4, 5, 6, 7, 8, 9, 10] :
			for count in range(0, 1) :
				cmd2 = "./a.out ../../Datasets/" + input_files[i] + " 56 " + str(start_nodes[i]) + " " + str(max_nodes[i]) + " " + str(iteration_threshold) + " " + str(tdf)
				print cmd2
				cmd2 = Popen(cmd2.split(), stdin=sp.PIPE, stdout=sp.PIPE)
				cmd2.wait()

				cmd3 = "mv ../../Output/" + program[:-4] + "_out.txt ../../Output/" + program[:-4] + "_" + input_files[i][:-4] + "_thd_" + str(56) + "_" + str(count) + "_fail_" + str(tdf) + ".txt"
				cmd3 = Popen(cmd3.split(), stdin=sp.PIPE, stdout=sp.PIPE)
				cmd3.wait()
