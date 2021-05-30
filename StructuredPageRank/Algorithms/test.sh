#!/bin/bash

#Barriers.cpp
g++ -std=c++11 Barriers.cpp -lpthread
./a.out 10.txt 8 0 40000000 75

#No_Sync.cpp
#g++ -std=c++11 No_Sync.cpp -lpthread -o c.out
#./c.out 10.txt 8 0 40000000 75

#No_Sync_Opt.cpp
#g++ -std=c++11 No_Sync_Opt.cpp -lpthread -o d.out
#./d.out 10.txt 8 0 40000000 75

echo "Compare Barriers_Out_8_75_10.txt No_Sync_Opt_Out_8_75_10.txt"
python3 result_compare.py Barriers_Out_8_75_10.txt No_Sync_Opt_Out_8_75_10.txt