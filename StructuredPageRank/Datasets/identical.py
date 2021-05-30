import numpy as np
import sys

argumments = sys.argv
file = argumments[1]

data = open(file, 'r').readline()
data = data.strip().split("\t")

if len(data) == 2:
	data = [x.strip().split("\t") for x in open(file).readlines()]
else :
	data = [x.strip().split(" ") for x in open(file).readlines()]

data = np.asarray(data)

dict1 = {}
dict2 = {}
rep_map = {}

max_val = 0
for i in range(len(data)) :
	max_val = max(max_val, max(int(data[i][0]), int(data[i][1])))
	if data[i][0] not in dict1:
		dict1[data[i][0]] = {data[i][1]}
	else :
		dict1[data[i][0]].add(data[i][1])

print ("max value : ", max_val)
for i in dict1 :
	strng = ",".join(sorted(dict1[i]))
	if strng not in dict2 :
		dict2[strng] = {i}
	else :
		dict2[strng].add(i)

for i in dict2 :
	lst = sorted(dict2[i])
	if len(lst) > 1 :
		for j in lst :
			rep_map[j] = lst[0]

counter = 0
f = open(file[:-4] + "_rep_map.txt", "w")
for i in range(0, max_val+1) :
	if str(i) in rep_map :
		f.write(str(i) + " " + rep_map[str(i)] + "\n")
	else :
		f.write(str(i) + " " + str(i) + "\n")
	counter += 1
f.close()
print ("counter : ", counter)
