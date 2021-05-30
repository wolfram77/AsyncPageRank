import os
import sys
import shlex
import subprocess as sp
from subprocess import Popen
import os

#Regular configs
#threads = [7, 14, 21, 28, 32, 56] 
#input_files = ["10.txt", "20.txt", "30.txt", "40.txt", "50.txt", "60.txt", "70.txt"]
#max_nodes = [1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 7000000]
#start_nodes = [0, 0, 0, 0, 0, 0, 0]

#Karra configs
threads = [8] 
input_files = ["70.txt"]
max_nodes = [7000000]
start_nodes = [0]

#Create the _rep_out.txt
for j in range (len(input_files)) :
	rep_file_name = "../Datasets/" + input_files[j][:-4] + "_rep_map.txt"
	print(rep_file_name)
	if os.path.isfile(rep_file_name) :
		print("file exists: " + rep_file_name)
		continue
	cmd2 = "python3 ../Datasets/identical.py ../Datasets/" + input_files[j]
	print(cmd2)
	cmd2 = Popen(cmd2.split(), stdin=sp.PIPE, stdout=sp.PIPE)
	cmd2.wait()

#Gernerate the program_names structure
#program_names means the directories.
#program means the .cpp file to execute that is the algorithm variant.
program_names = []
current_dir = os.path.dirname(os.path.realpath(__file__))
dirfiles = os.listdir(current_dir)
for file in dirfiles:
	if os.path.isdir(file): program_names.append(file)
print(program_names)

#To run only one specific variant meaning files only in one specific directory then do the following:
#For example, lets say we would want to run only the programs in No_Sync directory, then do the following:
#program_names = ['No_Sync']
program_names = ['Barriers']

for i in range(len(program_names)) :
	dirname = os.path.dirname(os.path.realpath(__file__)) + "/" + program_names[i]
	print(dirname)
	files = os.listdir(dirname)
	#Furthermore if you would like to execute only one or two files in a directory
	#For example in No_Sync, you want to execute only No_Sync_Chain.cpp, then do the following
	#files = ['No_Sync_Chain.cpp']
	files = ['Barriers_Helper.cpp']
	for program in files:
		if program[-4:] != ".cpp" :
			continue
		print(program)
		cmd1 = "g++ -std=c++11 -pthread " + program
		cmd1 = Popen(cmd1.split(), stdin=sp.PIPE, stdout=sp.PIPE, cwd = dirname)
		cmd1.wait()
		for j in range (len(input_files)) :
			if program_names[i] == "Sequential" :
				for count in range(0, 3) :
					cmd2 = "./a.out " + "../../Datasets/" +input_files[j] + " " + str(start_nodes[j]) + " " + str(max_nodes[j]) + " 75"
					print(cmd2)
					cmd2 = Popen(cmd2.split(), stdin=sp.PIPE, stdout=sp.PIPE, cwd = dirname)
					cmd2.wait()

					cmd3 = "mv " + "../../Output/" + program[:-4] + "_out.txt " + "../../Output/" + program[:-4] + "_" + input_files[j][:-4] + "_pg_" + str(count) + ".txt"
					cmd3 = Popen(cmd3.split(), stdin=sp.PIPE, stdout=sp.PIPE, cwd = dirname)
					cmd3.wait()
			elif program_names[i] in ["Barriers_Edge_Opt", "No_Sync_Edge_Opt", "Sleeper_Thread"]:
				continue
			else :
				for thd_num in threads :
					for count in range(0, 3) :
						cmd2 = "./a.out " + "../../Datasets/" + input_files[j] + " " + str(thd_num) + " " + str(start_nodes[j]) + " " + str(max_nodes[j]) + " 75"
						print(cmd2)
						cmd2 = Popen(cmd2.split(), stdin=sp.PIPE, stdout=sp.PIPE, cwd = os.path.dirname(os.path.realpath(__file__)) + "/" + program_names[i])
						cmd2.wait()

						cmd3 = "mv " + "../../Output/" + program[:-4] + "_out.txt ../../Output/" + program[:-4] + "_" + input_files[j][:-4] + "_thd_" + str(thd_num) + "_" + str(count) + ".txt"
						cmd3 = Popen(cmd3.split(), stdin=sp.PIPE, stdout=sp.PIPE, cwd = os.path.dirname(os.path.realpath(__file__)) + "/" + program_names[i])
						cmd3.wait()