#!/bin/zsh
dataversion=v7
set -eux
cat ../data/$dataversion.pathcounts | awk 'NF!=2{print "ERROR"; print; exit -1} {print $2}' | python match.py > $dataversion.path_pattern_matches
hashjoin -1 3 -2 2 $dataversion.path_pattern_matches =(perl -pe 's/ /\t/' < ../data/$dataversion.pathcounts) > $dataversion.path_pattern_matches.withcounts

