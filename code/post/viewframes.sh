#!/bin/zsh
set -eu
prefix=$1

if [ -d $prefix ]; then
  prefix=$prefix/mean
fi

# if [ ! -f $prefix.cContextFrame ]; then
#   ./java.sh DumpModel dump $prefix.ser.gz
# fi
outfile=~/www/ptab/$(echo $prefix | perl -pe 's|^/||; s|/$||; s|.*ptab/||; s|/|.|g').html
python $(dirname $0)/viewframes.py $prefix > $outfile

if [[ $HOSTNAME == cab.ark* ]]; then
echo http://www.ark.cs.cmu.edu/brendano/ptab/$(basename $outfile)
elif [[ $HOSTNAME == btop* ]]; then
  open $outfile
else
  echo $outfile
fi
