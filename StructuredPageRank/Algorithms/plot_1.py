import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties
import datetime

input_files = ["10.txt", "20.txt", "30.txt", "40.txt", "50.txt", "60.txt", "70.txt"]
#input_files = ["germany_osm.txt"]
#input_files = ["asia_osm.txt"]

current_dir = os.path.dirname(os.path.realpath(__file__))
print(current_dir)
programs = []
program_names = []
thread_config = "_thd_14_"

dirs = os.listdir(current_dir)

plot_data = []
plot_iter_data = []
plot_error_data = []
plot_parallel_data = []
for d in dirs :
	if os.path.isdir(d):
		files = os.listdir(current_dir + "/" + d)
		for program in files:
			if program[-3:] == "cpp":
				if program in ["Barriers_Edge_Opt.cpp", "No_Sync_Edge_Opt.cpp", "Sequential.cpp"]:
					continue

				if program != "Sequential.cpp":
					programs.append(program)
					program_names.append(program[:-4])

				times = []
				iterations = []
				manhattan_vals = []
				parallel_vals = []
				seq_time = 0.0

				for inpt in input_files :
					for count in range(0, 3) :
						first_line=open("../Output/Sequential_" + inpt[:-4] + "_pg_" + str(count) + ".txt").readline().rstrip()
						seq_time += float(first_line)
					seq_time = float(seq_time)/3

					seq_data = [float(x.strip()) for x in open("../Output/Sequential_" + inpt[:-4] + "_pg_" + str(0) + ".txt").readlines()]
					seq_data = seq_data[2:]

					time_val = 0.0
					iter_val = 0
					l1_norm = 0.0
					for count in range(0, 3) :
						file = "../Output/" + program[:-4] + "_" + inpt[:-4] + thread_config + str(count) + ".txt"
						print(program[:-4] + "_" + inpt[:-4] + thread_config + str(count) + ".txt")
						lines = open("../Output/" + program[:-4] + "_" + inpt[:-4] + thread_config + str(count) + ".txt").readlines()[0:2]
						first_line = lines[0].rstrip()
						second_line = lines[1].rstrip()
						time_val += float(first_line)
						iter_val += float(second_line)
						
						par_data = [float(x.strip()) for x in open("../Output/" + program[:-4] + "_" + inpt[:-4] + thread_config + str(count) + ".txt").readlines()]
						par_data = par_data[2:]
						sum_ = 0.0
						l1_norm += np.linalg.norm(np.asarray(seq_data) - np.asarray(par_data), ord=1)

					time_val = time_val / 3
					iter_val = iter_val / 3
					l1_norm = l1_norm / 3

					print("Seq time : ", seq_time)
					print("Parallel time : ", time_val)
					print("Speedup : ", seq_time / float(time_val))
					print("Iterations : ", iter_val)
					print("L1 norm : ", l1_norm)

					times.append(seq_time / float(time_val))
					iterations.append(iter_val)
					manhattan_vals.append(l1_norm)
					parallel_vals.append(time_val)
				plot_data.append(times)
				plot_iter_data.append(iterations)
				plot_error_data.append(manhattan_vals)
				plot_parallel_data.append(parallel_vals)

dic_data = {}
for d in range(len(plot_data)) :
    dic_data[program_names[d]] = plot_data[d]

dic_iter_data = {}
for d in range(len(plot_iter_data)) :
    dic_iter_data[program_names[d]] = plot_iter_data[d]

dic_error_data = {}
for d in range(len(plot_error_data)) :
    dic_error_data[program_names[d]] = plot_error_data[d]

dic_parallel_data = {}
for d in range(len(plot_parallel_data)) :
    dic_parallel_data[program_names[d]] = plot_parallel_data[d]

def bar_plot(ax, data, y_axis_label, colors=None, total_width=0.8, single_width=1, legend=True):
    # Check if colors where provided, otherwhise use the default color cycle
    if colors is None:
        colors = plt.rcParams['axes.prop_cycle'].by_key()['color']

    # Number of bars per group
    n_bars = len(data)

    # The width of a single bar
    bar_width = total_width / n_bars

    # List containing handles for the drawn bars, used for the legend
    bars = []

    # Iterate over all data
    for i, (name, values) in enumerate(data.items()):
        # The offset in x direction of that bar
        x_offset = (i - n_bars / 2) * bar_width + bar_width / 2

        # Draw a bar for every value of that type
        for x, y in enumerate(values):
            bar = ax.bar(x + x_offset, y, width=bar_width * single_width, color=colors[i % len(colors)])

        # Add a handle to the last drawn bar, which we'll need for the legend
        bars.append(bar[0])

    ax.set_title('PageRank Speedup Graph(Synthetic Datasets)', fontsize=30)
    #ax.set_title('PageRank Speedup Graph', fontsize=20)
    #ax.set_xlabel('Benchmarks', fontsize=18)
    #ax.set_ylabel('Speedup (w.r.t Seq)', fontsize=25)
    ax.set_ylabel( y_axis_label, fontsize=25)
    ax.set_xticks(np.arange(len(input_files)))
    labels = ax.set_xticklabels([x[:-4] for x in input_files], rotation=45)
    ax.tick_params(axis="x", labelsize=16)
    ax.tick_params(axis="y", labelsize=25)
    #for i, label in enumerate(labels) :
    #    label.set_y(label.get_position()[1] - (i % 2) * 0.05)
    # Draw legend if we need
    if legend:
        box = ax.get_position()
        ax.set_position([box.x0, box.y0 + box.height * 0.1, box.width, box.height * 0.9])
        ax.legend(bars, data.keys(), loc="upper center", bbox_to_anchor=(0.5, -0.19), fancybox=True, shadow=True, ncol=6, prop={"size":12})
        

fig, ax = plt.subplots(figsize=(15,10))
bar_plot(ax, dic_data, 'Speedup (w.r.t Seq)', total_width=.8, single_width=.9)
#plt.savefig("plot_1_IdenticalNode.png")
#plt.savefig("plot_1_RWG.png")
plt.savefig("plot_1.png")
bar_plot(ax, dic_iter_data, 'Iterations (w.r.t Seq)', total_width=.8, single_width=.9)
plt.savefig("plot_iter.png")

#Debug code
#print("Program names", program_names)
#print("Input files", input_files)
#print("Iterration data", dic_iter_data)
#print("Speedup data", dic_data)
#print("Error Data", dic_error_data)
#print("Parallel Times", dic_parallel_data)

headers = ["Program", "Input", "Iterations", "Speed-up", "Threads", "L1-norm", "Parallel-Time"]
csv_data = [headers]

for i in range(len(program_names)):
	for j in range(len(input_files)):
		program = program_names[i]
		csv_data.append([program, input_files[j], str(dic_iter_data[program][j]), str(dic_data[program][j]), "8", str(dic_error_data[program][j]), str(dic_parallel_data[program][j])])

date_str = datetime.date.today().strftime("%B_%d")
csv_file = date_str + '_result_data.csv'
np.savetxt(csv_file, csv_data, delimiter=',', fmt="%s")