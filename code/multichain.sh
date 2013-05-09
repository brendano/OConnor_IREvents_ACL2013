#!/bin/zsh
set -eu

propfile=$1
outname=$2
[ -f $propfile ]
set -eux
for c in 1 2 ; {
  outdir=$outname.$c.out
  logdir=$outname.$c.log
  # outdir=$outname.out
  # logdir=$outname.log
  mkdir -p $outdir
  git log-for-model > $outdir/gitlog.txt
  git diff > $outdir/gitdiff.txt
  cp $propfile $outdir/run.prop
  time ./fast.sh MainModel $propfile $outdir > $logdir &
}
wait

# for c in 1 2 3; {
#   post/trim.py $outname.$c
# }

