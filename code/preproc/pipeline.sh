#!/bin/zsh
set -eu

tag=v7
collfile=data/all.coll.$tag

# print -l pathex.$tag/*/*.tuple | 
# parallel -j8 --eta "
#   cat {} |
#   python preproc/tuple_filter.py | 
#   python preproc/tuple_collapse.py |
#   cat > {.}.coll
# "

set -x

# pv pathex.$tag/*/*.tuple |
#   python preproc/tuple_filter.py | 
#   python preproc/tuple_collapse.py |
#   cat > $collfile



# pv pathex.$tag/*.coll | tabawk '$2 ~ /\[[1-3],/' > $collfile

pv $collfile | cut -f4-5 | count | sort -rn > data/$tag.dyadcounts
pv $collfile | cut -f4-5 | awk '$1>$2{x=$1;y=$2} $1<$2{x=$2;y=$1} {print x,y}' |
  count | sort -rn > data/$tag.undirdyadcounts
pv $collfile | cut -f6   | count | sort -rn > data/$tag.pathcounts

python preproc/make_datefile.py month $collfile > data/datefile.month
python preproc/make_datefile.py week $collfile  > data/datefile.week
python preproc/make_datefile.py year $collfile  > data/datefile.year

# pv $collfile | python preproc/pathfilter.py data/$tag.dyadcounts data/$tag.pathcounts 5 5 > data/$tag.pathfil.dthresh=5.pthresh=5
pv $collfile | python preproc/pathfilter.py data/$tag.dyadcounts data/$tag.pathcounts 100 10 > data/$tag.pathfil.dthresh=100.pthresh=10
