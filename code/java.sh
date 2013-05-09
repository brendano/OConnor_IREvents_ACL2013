#!/bin/bash
here=$(dirname $0)

CP="$CP:$here"       ## hack for easy resource loading
# CP="$CP:supersense-tagger.jar"  ## have to be in its directory to use

CP="$CP:$here/bin"          # Eclipse build dir
CP="$CP:$here/ptab.jar"     # jar file

# Libraries
# CP="$CP:$(echo $HOME/projects/myutil/lib/*.jar | tr ' ' ':')"
# CP="$CP:$HOME/projects/myutil/bin"
CP="$CP:$(echo $here/lib/*.jar | tr ' ' ':')"

# set -eux
java -Xmx2g -XX:ParallelGCThreads=2 -ea -cp "$CP" "$@"
