#!/bin/zsh

## MCMC Traceplots
## and Frame viewer

set -eu
# set -x
outdir=$1
logfile=${outdir:r}.log

# if [ -d $outdir ]; then
# setopt nonomatch
# print -l $outdir/model.{???,????,?????}.* | grep -v '?' > tmp.list
# cat tmp.list | perl -pe 's/(model\.\d+).*/$1/' | uniq > tmp.list2
# prefix=$(tail -1 tmp.list2)
# 
# $(dirname $0)/view.sh $prefix
# echo
# fi

outplot=~/www/ptab/$(echo ${outdir:r} | perl -pe 's|^/||; s|/$||; s|.*ptab/||; s|/|.|g').pdf
Rscript $(dirname $0)/traceplots.r $outplot $logfile
echo http://www.ark.cs.cmu.edu/brendano/ptab/$(basename $outplot)
open $outplot
