#!/bin/zsh
set -eux
java -cp ../../lib/antlr-3.4-complete.jar org.antlr.Tool Grammar.g

awk 'BEGIN{print "package myprolog;"} 1' =(cat GrammarLexer.java) > GrammarLexer.java
awk 'BEGIN{print "package myprolog;"} 1' =(cat GrammarParser.java) > GrammarParser.java

set +eu
rm -rf ../../bin/myprolog/Grammar*.class
set -eu
rm -rf build/ && mkdir build
CP=$(print -l ../../lib/*.jar | tr '\n' :)
javac -cp $CP:../../bin Grammar*.java PrologParser.java -d build

