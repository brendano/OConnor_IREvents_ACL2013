#!/bin/zsh
set -eux
CP=$(print -l ../../lib/*.jar | tr '\n' :)
java -cp $CP:build:../../bin myprolog.PrologParser "$@"
