#!/bin/zsh

## Make model output directory smaller

dir=$1
trimdir=${dir:r}.trim
zipfile=${trimdir}.zip
echo "$dir => $trimdir"
set -eu
# set -x
setopt nonomatch

cp_cmd() {
  cp -f "$@"
}

mkdir -p $trimdir
cp_cmd $dir/*.{vocab,prop} $trimdir
cp_cmd $dir/datefile $trimdir
set +eu
cp_cmd $dir/{mean,var}.* $trimdir
set -eu

set +x
# n=11
# echo "using last $n samples"
# print -l $dir/model.{???,????,?????}.* | grep -v '?' > tmp.list.$$
# cat tmp.list.$$ | perl -pe 's/(model\.\d+).*/$1/' | uniq > tmp.list2.$$
# tail -n$n tmp.list2.$$ | parallel -j1 "cp -v {}.* $trimdir"

du -cm $dir $trimdir

zip -r $zipfile $trimdir
ls -lh $zipfile
mv $zipfile ~/ptab/www
ls -l ~/ptab/www/$(basename $zipfile)
