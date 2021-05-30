import os
import numpy as np
from datetime import datetime

#input_files = ["10.txt", "20.txt", "30.txt", "40.txt", "50.txt", "60.txt", "70.txt"]
#input_files = ["web-Stanford.txt", "web-Notre.txt", "web-BerkStan.txt","web-Google.txt","soc-LiveJournal1.txt", "soc-Epinions1.txt", "Slashdot0811.txt", "Slashdot0902.txt", "road-italy-osm.txt","great-britain_osm.txt","asia_osm.txt","germany_osm.txt"]
input_files = ["70.txt"]
#The actual thread configurations we have data for
#threads = [7, 14, 21, 28, 32,56]
threads = [8]

current_dir = os.path.dirname(os.path.realpath(__file__))
print(current_dir)
programs = []
program_names = []
headers = ["Program", "Input", "Iterations", "Speed-up", "Threads", "L1-norm", "Parallel-Time"]
csv_data = [headers]

dirs = os.listdir(current_dir)

for d in dirs :
	if os.path.isdir(d):
		files = os.listdir(current_dir + "/" + d)
		#files = ["Barriers_Helper.cpp"]
		for program in files:
			if program[-3:] == "cpp":
				if program in ["Barriers_Chain.cpp", "Barriers_Opt.cpp", "Barriers_Opt_Identical.cpp", "Barriers_Edge_Chain.cpp", "Barriers_Edge_Opt.cpp", "No_Sync_Chain.cpp", "Sequential.cpp", "No_Sync_Edge_Opt.cpp",  "No_Sync_Edge.cpp", "Barriers_Helper_Sleeper.cpp","Barriers_Sleeper.cpp","No_Sync_Sleeper.cpp", "Barriers_Helper_Thread_Fail.cpp"]:
					continue

				programs.append(program)
				program_names.append(program[:-4])
				
				for inpt in input_files :
					seq_time = 0.0
					for count in range(0, 3) :
						first_line=open("../Output/Sequential_" + inpt[:-4] + "_pg_" + str(count) + ".txt").readline().rstrip()
						seq_time += float(first_line)
					seq_time = float(seq_time)/3
					seq_data = [float(x.strip()) for x in open("../Output/Sequential_" + inpt[:-4] + "_pg_" + str(0) + ".txt").readlines()]
					seq_data = seq_data[2:]

					for thread_num in threads :
						counter = 0
						time_val = 0.0
						iter_val = 0
						l1_norm = 0.0
						speed_up = 0.0
						for count in range(0, 3) :
								file_name = "../Output/" + program[:-4] + "_" + inpt[:-4] + "_thd_"+ str(thread_num) + "_" + str(count) + ".txt"
								print(program[:-4] + "_" + inpt[:-4] + "_thd_"+ str(thread_num) + "_" + str(count) + ".txt")
								lines = open("../Output/" + program[:-4] + "_" + inpt[:-4] + "_thd_"+  str(thread_num) + "_" + str(count) + ".txt").readlines()[0:2]
								
								first_line = lines[0].rstrip()
								second_line = lines[1].rstrip()
								time_val += float(first_line)
								iter_val += float(second_line)
						
								par_data = [float(x.strip()) for x in open("../Output/" + program[:-4] + "_" + inpt[:-4] + "_thd_"+ str(thread_num) + "_" + str(count) + ".txt").readlines()]
								par_data = par_data[2:]
								sum_ = 0.0
								l1_norm += np.linalg.norm(np.asarray(seq_data) - np.asarray(par_data), ord=1)
								counter += 1

						if counter != 0 and float(time_val) != 0:
							print("Counter and time_val are not 0")
							print("time_val : ", time_val)
							time_val = time_val / counter
							iter_val = iter_val / counter
							l2_norm = l1_norm / counter
							speed_up = seq_time / float(time_val)

						print("Seq time : ", seq_time)
						print("Parallel time : ", time_val)
						print("Speedup : ", speed_up)
						print("Iterations : ", iter_val)
						print("L1 norm : ", l1_norm)

						csv_data.append([program, inpt, iter_val, speed_up, thread_num, l1_norm, time_val])

print("programs:", programs)
#Write to the csv file, create the filename using timestamp
now = datetime.now()
date_str = now.strftime("%B_%d_%H_%M")
csv_file = date_str + '_analysis_result_data.csv'
np.savetxt(csv_file, csv_data, delimiter=',', fmt="%s")
