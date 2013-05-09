grammar Grammar;
options {
  output=AST;
  ASTLabelType=CommonTree;
}

start : compound (COMMA compound)* COMMA*;
compound : predname^ args;
args : LRB term (COMMA term)* RRB ;
predname : TOKEN ;
term : (atom_thingy | compound) ;
atom_thingy : TOKEN ;

COMMA : ',' ;

LSB     :        '[' ;
RSB     :        ']' ;

LCB     :        '{' ;
RCB     :        '}' ;

LRB     :        '(' ;
RRB     :        ')' ;

TOKEN   :       ~(COMMA |LSB|RSB|LCB|RCB|LRB|RRB)+ ;

// WS : (' '|'\t'|'\n'|'\r') ;
 