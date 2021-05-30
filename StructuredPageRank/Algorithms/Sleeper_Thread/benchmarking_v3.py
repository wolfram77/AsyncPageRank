import os
import sys
import shlex
import time
import subprocess as sp
from subprocess import Popen

programs = ["Barriers_Sleeper.cpp", "No_Sync_Sleeper.cpp", "Barriers_Helper_Sleeper.cpp"]

#Karra configs
thd_num = '8' 
input_files = ["70.txt"]
max_nodes = [7000000]
start_nodes = [0]
iteration_threshold = 20
#sleep_times = [100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000 ]
sleep_times = [100, 150]

for program in programs :
	cmd1 = "g++ -std=c++11 -pthread " + program
	print (cmd1)
	cmd1 = Popen(cmd1.split(), stdin=sp.PIPE, stdout=sp.PIPE)
	cmd1.wait()

	for i in range (len(input_files)) :
		for sz in sleep_times :
			for count in range(0, 1) :
				cmd2 = "./a.out " + "../../Datasets/" + input_files[i] + " " + thd_num + " " + str(start_nodes[i]) + " " + str(max_nodes[i]) + " " + str(iteration_threshold) + " " + str(sz)
				print (cmd2)
				cmd2 = Popen(cmd2.split(), stdin=sp.PIPE, stdout=sp.PIPE)
				cmd2.wait()

				cmd3 = "mv " + "../../Output/" + program[:-4] + "_out.txt ../../Output/" + program[:-4] + "_" + input_files[i][:-4] + "_thd_" + thd_num + "_" + str(count) + "_sleep_" + str(sz) + ".txt"
				cmd3 = Popen(cmd3.split(), stdin=sp.PIPE, stdout=sp.PIPE)
				cmd3.wait()
