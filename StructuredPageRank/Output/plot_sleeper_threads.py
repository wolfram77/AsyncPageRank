import os
import numpy as np
import matplotlib.pyplot as plt
from datetime import datetime

programs = ["Barriers_Sleeper.cpp", "No_Sync_Sleeper.cpp", "Barriers_Helper_Sleeper.cpp"]
inpt = "70.txt"
#sleeps = [100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000]
sleeps = [100, 150]
files = os.listdir(".")
#thd_num = "_thd_32_"
thd_num = "_thd_8_"
headers = ["Program", "Input", "Sleep-Time", "Parallel-Time"]
csv_data = [headers]

plot_data = []
for sz in sleeps :
    times = []
    seq_time = 0.0
    for program in programs :
        time_val = 0.0
        for count in range(0, 1) :
            file = program[:-4] + "_" + inpt[:-4] + "_thd_8_" + str(count) + "_sleep_" + str(sz) + ".txt"
            first_line=open(program[:-4] + "_" + inpt[:-4] + "_thd_8_" + str(count) + "_sleep_" + str(sz) + ".txt").readline().rstrip()
            time_val += float(first_line)
        csv_data.append([program, inpt, sz, time_val])
        times.append(float(time_val))
    plot_data.append(times)

#Write to the csv file, create the filename using timestamp
now = datetime.now()
date_str = now.strftime("%B_%d_%H_%M")
csv_file = date_str + '_sleep_thread_result_data.csv'
np.savetxt(csv_file, csv_data, delimiter=',', fmt="%s")

dic_data = {}
_data = list(map(list, zip(*plot_data)))
for d in range(len(_data)) :
    dic_data[programs[d][:-4]] = _data[d]

def bar_plot(ax, data, colors=None, total_width=0.8, single_width=1, legend=True):
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

    ax.set_title('PageRank Time with Random Thread Sleep', fontsize=30)
    ax.set_xlabel('Sleep Time (sec)', fontsize=25)
    ax.set_ylabel('Program Execution Time (sec)', fontsize=25)
    ax.set_xticks(np.arange(len(sleeps)))
    ax.set_xticklabels([str(x/100) for x in sleeps])
    ax.tick_params(axis="x", labelsize=18)
    ax.tick_params(axis="y", labelsize=25)
    #ax.set_yticks(np.arange(10), [str(max_val/(10-x)) for x in range(10)])
    #ax.set_yticklabels([str(max_val/(10-x)) for x in range(10)], fontsize=14)
    # Draw legend if we need
    if legend:
        box = ax.get_position()
        ax.set_position([box.x0, box.y0 + box.height * 0.1, box.width, box.height * 0.9])
        ax.legend(bars, data.keys(), loc="upper center", bbox_to_anchor=(0.5, -0.16), fancybox=True, shadow=True, ncol=8, prop={"size":25})

fig, ax = plt.subplots(figsize=(15,10))
bar_plot(ax, dic_data, total_width=.8, single_width=.9)
plt.savefig("plot_5.png")
