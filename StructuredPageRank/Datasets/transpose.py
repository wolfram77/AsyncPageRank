import numpy as np
import sys

if len(sys.argv) != 2 :
        print("pass correct arguments")
else :
        file_name = sys.argv[1]
        data = open(file_name, 'r').readline().strip().split("\t")
        if len(data) == 2 :
                data = [list(map(int, x.strip().split("\t"))) for x in open(file_name, 'r').readlines()]
        else :
                data = [list(map(int, x.strip().split(" "))) for x in open(file_name, 'r').readlines()]
        data = np.asarray(data)
        data[:,[0,1]] = data[:,[1,0]]
        data = ["\t".join(list(map(str, x))) for x in data]
        f = open(file_name, 'w')
        f.write("\n".join(data))
        f.close()
