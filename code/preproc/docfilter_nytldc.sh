#!/bin/zsh

# outdir=~/ptab/pathex.nyt.v1
outdir=/cab1/brendano/sem/nlp_preproc/nyt_ldc/filtered
here=$(dirname $0)

# TODO hasn't been updated to docfilter vs docdedup separation

print -l /cab1/brendano/sem/nlp_preproc/nyt_ldc/parse/*/*.corejson |
ruby -ne '
f=$_.strip
month=f.sub(/.*parse\//,"").sub(".corejson","").sub("/","-")
puts f
puts month
' | parallel -N2 -j20 --eta "
cat {1} | shuf | python $here/docfilter.py > $outdir/{2}.filtered
"

