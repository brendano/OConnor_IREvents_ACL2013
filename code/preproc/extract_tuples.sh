#!/bin/zsh

here=$(dirname $0)
set -eux


function go() {

indir=$1
outdir=$2
mkdir -p $outdir

print -l $indir/*.filtered | shuf |
parallel --eta -v -j10 "
cat {} | grep '^DOC' | cut --complement -f1 | 
python ~/sem/lib/doc2jsent.py |
./java.sh PathAn > $outdir/{/.}.tuple
"

}

# go /cab1/brendano/sem/nlp_preproc/justin_gigaword/corejson_filtered ~/ptab/pathex.v7/gw
go /cab1/brendano/sem/nlp_preproc/nyt_ldc/filtered ~/ptab/pathex.v7/nytldc
