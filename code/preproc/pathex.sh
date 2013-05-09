#!/bin/bash

# input: jsent, possibly with DUP tags
# output: path extractions (verbose, noncollapsed version)

here=$(dirname $0)
grep -v '^DUP' | (cd $here/.. && ./java.sh PathAn)
