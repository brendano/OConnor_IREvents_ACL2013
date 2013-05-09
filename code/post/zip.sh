#!/bin/zsh
set -eux
trimdir=$1
[ -d $trimdir ]
outfile=~/www/ptab/$(echo $trimdir | perl -pe 's|^/||; s|/$||; s|.*/ptab/||; s|/|.|g').zip
rm -f $outfile
zip -r $outfile $trimdir

echo http://www.ark.cs.cmu.edu/brendano/ptab/${outfile:t}
