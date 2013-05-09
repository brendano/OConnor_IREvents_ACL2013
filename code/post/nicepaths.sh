#!/bin/bash
set -eux
modeldir=$1
python $(dirname $0)/nicepaths.py < $modeldir/path.vocab > $modeldir/path.vocab.nice
