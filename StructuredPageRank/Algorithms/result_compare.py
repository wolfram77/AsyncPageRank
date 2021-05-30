import sys

arg1 = sys.argv[1]
arg2 = sys.argv[2]

lines1 = [x.strip() for x in open(arg1).readlines()]
lines2 = [x.strip() for x in open(arg2).readlines()]

if len(lines1) != len(lines2):
	print("Please enter correct files for comparison")
else :
        l1_norm = 0.0
        counter = 0
        counter_str = 0
        counter_num = 0
        for i in range(2, len(lines1)):
                l1_norm += abs(float(lines1[i]) - float(lines2[i]))
                if lines1[i] == lines2[i]:
                        counter_str += 1
                        continue
                elif abs(float(lines1[i]) - float(lines2[i])) < 0.0001:
                        counter_num += 1
                        continue
                else:
                        print(float(lines1[i]), float(lines2[i]))
                        counter += 1;

        if counter != 0:
	        print("number of mismatches : ", counter)
        else:
                print("Equal pagerank values")
        if counter_str != 0:
                print ("String matches : ", counter_str)

        if counter_num != 0:
                print ("num matches : ", counter_num)

        print("Manhattan distance : ", l1_norm)
