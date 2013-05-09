#!/bin/zsh

indir=/cab1/brendano/sem/nlp_preproc/justin_gigaword/corejson_by_source
outdir=/cab1/brendano/sem/nlp_preproc/justin_gigaword/corejson_filtered
here=$(dirname $0)

mkdir -p $outdir
set -eu

for y in {1994..2008}; do
  for m in {01..12}; do
    echo ${y}${m}
  done
done | shuf |
parallel --eta -v -j8  "
  cat $indir/*_{}.* |
  shuf | python $here/docdedup.py | 
    tee $outdir/{}.dedup |
  grep '^DOC' | cut --complement -f1 |
  python $here/docfilter.py |
    cat > $outdir/{}.filter
"

# parallel -u "
#   du -cm $indir/*_{}.*
#   "

