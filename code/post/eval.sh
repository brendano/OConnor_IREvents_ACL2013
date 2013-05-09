#!/bin/zsh
# Run all postprocessing evaluations and viewers

set -eu
outdir=$1
here=$(dirname $0)

set -x

python $here/average.py $outdir
$here/viewframes.sh $outdir/mean
python $here/../verbdict/score.py internal_diff $outdir/mean > $outdir/rankcorr.log


