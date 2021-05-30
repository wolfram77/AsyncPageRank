import os
import sys
import shlex
import subprocess as sp
from subprocess import Popen

#Karra configs
input_files = ["70.txt"]
max_nodes = [7000000]
start_nodes = [0]
iteration_threshold = 20
programs = ["Barriers_Helper_Thread_Fail.cpp"]
#value on server 32?
thd_num = "8"
#threads_failed = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
threads_failed = [4]

for program in programs :
	cmd1 = "g++ -std=c++11 -pthread " + program
	print (cmd1)
	cmd1 = Popen(cmd1.split(), stdin=sp.PIPE, stdout=sp.PIPE)
	cmd1.wait()

	for i in range (len(input_files)) :
		for tdf in threads_failed :
			for count in range(0, 3) :
				cmd2 = "./a.out " + "../../Datasets/" +input_files[i] + " " + thd_num+ " " + str(start_nodes[i]) + " " + str(max_nodes[i]) + " " + str(iteration_threshold) + " " + str(tdf)
				print (cmd2)
				cmd2 = Popen(cmd2.split(), stdin=sp.PIPE, stdout=sp.PIPE)
				cmd2.wait()

				cmd3 = "mv " + "../../Output/"+ program[:-4] + "_out.txt ../../Output/" + program[:-4] + "_" + input_files[i][:-4] + "_thd_" + thd_num + "_" + str(count) + "_fail_" + str(tdf) + ".txt"
				cmd3 = Popen(cmd3.split(), stdin=sp.PIPE, stdout=sp.PIPE)
				cmd3.wait()
