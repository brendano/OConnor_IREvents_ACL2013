#!/bin/bash
set -eu

here=$(dirname $0)
CP=${CP:-}
# CP="$CP:$here/bin"   ## eclipse build dir
CP="$CP:$here"       ## hack for easy resource loading
# CP="$CP:supersense-tagger.jar"  ## have to be in its directory to use
# CP="$CP:$HOME/sw/nlp/stanford-corenlp-2012-04-09/stanford-corenlp-2012-04-09.jar"
CP="$CP:$(echo $here/lib/*.jar | tr ' ' ':')"
CP=$CP:$here/ptab.jar
GCTHREADS=${GCTHREADS:-2}
java -Xmx30g -XX:ParallelGCThreads=$GCTHREADS -cp "$CP" "$@"

