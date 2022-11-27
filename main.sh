#!/usr/bin/env bash
src="AsyncPageRank"
out="/home/resources/Documents/subhajit/$src.log"
ulimit -s unlimited
printf "" > "$out"

# Download program
rm -rf $src
git clone https://github.com/wolfram77/$src
cd $src

# Run
g++ -std=c++17 -O3 -pthread StructuredPageRank/Algorithms/No_Sync/No_Sync.cpp
stdbuf --output=L ./a.out ~/out/web-Stanford.edges      2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/web-BerkStan.edges      2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/web-Google.edges        2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/web-NotreDame.edges     2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/soc-Slashdot0811.edges  2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/soc-Slashdot0902.edges  2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/soc-Epinions1.edges     2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/coAuthorsDBLP.edges     2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/coAuthorsCiteseer.edges 2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/soc-LiveJournal1.edges  2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/coPapersCiteseer.edges  2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/coPapersDBLP.edges      2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/indochina-2004.edges    2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/italy_osm.edges         2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/great-britain_osm.edges 2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/germany_osm.edges       2>&1 | tee -a "$out"
stdbuf --output=L ./a.out ~/out/asia_osm.edges          2>&1 | tee -a "$out"
