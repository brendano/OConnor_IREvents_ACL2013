package myprolog;
// $ANTLR 3.4 Grammar.g 2012-11-29 12:54:33

import org.antlr.runtime.BitSet;
import org.antlr.runtime.NoViableAltException;
import org.antlr.runtime.Parser;
import org.antlr.runtime.ParserRuleReturnScope;
import org.antlr.runtime.RecognitionException;
import org.antlr.runtime.RecognizerSharedState;
import org.antlr.runtime.Token;
import org.antlr.runtime.TokenStream;
import org.antlr.runtime.tree.CommonTree;
import org.antlr.runtime.tree.CommonTreeAdaptor;
import org.antlr.runtime.tree.TreeAdaptor;


@SuppressWarnings({"all", "warnings", "unchecked"})
public class GrammarParser extends Parser {
    public static final String[] tokenNames = new String[] {
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "COMMA", "LCB", "LRB", "LSB", "RCB", "RRB", "RSB", "TOKEN"
    };

    public static final int EOF=-1;
    public static final int COMMA=4;
    public static final int LCB=5;
    public static final int LRB=6;
    public static final int LSB=7;
    public static final int RCB=8;
    public static final int RRB=9;
    public static final int RSB=10;
    public static final int TOKEN=11;

    // delegates
    public Parser[] getDelegates() {
        return new Parser[] {};
    }

    // delegators


    public GrammarParser(TokenStream input) {
        this(input, new RecognizerSharedState());
    }
    public GrammarParser(TokenStream input, RecognizerSharedState state) {
        super(input, state);
    }

protected TreeAdaptor adaptor = new CommonTreeAdaptor();

public void setTreeAdaptor(TreeAdaptor adaptor) {
    this.adaptor = adaptor;
}
public TreeAdaptor getTreeAdaptor() {
    return adaptor;
}
    public String[] getTokenNames() { return GrammarParser.tokenNames; }
    public String getGrammarFileName() { return "Grammar.g"; }


    public static class start_return extends ParserRuleReturnScope {
        CommonTree tree;
        public Object getTree() { return tree; }
    };


    // $ANTLR start "start"
    // Grammar.g:7:1: start : compound ( COMMA compound )* ( COMMA )* ;
    public final GrammarParser.start_return start() throws RecognitionException {
        GrammarParser.start_return retval = new GrammarParser.start_return();
        retval.start = input.LT(1);


        CommonTree root_0 = null;

        Token COMMA2=null;
        Token COMMA4=null;
        GrammarParser.compound_return compound1 =null;

        GrammarParser.compound_return compound3 =null;


        CommonTree COMMA2_tree=null;
        CommonTree COMMA4_tree=null;

        try {
            // Grammar.g:7:7: ( compound ( COMMA compound )* ( COMMA )* )
            // Grammar.g:7:9: compound ( COMMA compound )* ( COMMA )*
            {
            root_0 = (CommonTree)adaptor.nil();


            pushFollow(FOLLOW_compound_in_start28);
            compound1=compound();

            state._fsp--;

            adaptor.addChild(root_0, compound1.getTree());

            // Grammar.g:7:18: ( COMMA compound )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( (LA1_0==COMMA) ) {
                    int LA1_1 = input.LA(2);

                    if ( (LA1_1==TOKEN) ) {
                        alt1=1;
                    }


                }


                switch (alt1) {
            	case 1 :
            	    // Grammar.g:7:19: COMMA compound
            	    {
            	    COMMA2=(Token)match(input,COMMA,FOLLOW_COMMA_in_start31); 
            	    COMMA2_tree = 
            	    (CommonTree)adaptor.create(COMMA2)
            	    ;
            	    adaptor.addChild(root_0, COMMA2_tree);


            	    pushFollow(FOLLOW_compound_in_start33);
            	    compound3=compound();

            	    state._fsp--;

            	    adaptor.addChild(root_0, compound3.getTree());

            	    }
            	    break;

            	default :
            	    break loop1;
                }
            } while (true);


            // Grammar.g:7:36: ( COMMA )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( (LA2_0==COMMA) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // Grammar.g:7:36: COMMA
            	    {
            	    COMMA4=(Token)match(input,COMMA,FOLLOW_COMMA_in_start37); 
            	    COMMA4_tree = 
            	    (CommonTree)adaptor.create(COMMA4)
            	    ;
            	    adaptor.addChild(root_0, COMMA4_tree);


            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);


            }

            retval.stop = input.LT(-1);


            retval.tree = (CommonTree)adaptor.rulePostProcessing(root_0);
            adaptor.setTokenBoundaries(retval.tree, retval.start, retval.stop);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
    	retval.tree = (CommonTree)adaptor.errorNode(input, retval.start, input.LT(-1), re);

        }

        finally {
        	// do for sure before leaving
        }
        return retval;
    }
    // $ANTLR end "start"


    public static class compound_return extends ParserRuleReturnScope {
        CommonTree tree;
        public Object getTree() { return tree; }
    };


    // $ANTLR start "compound"
    // Grammar.g:8:1: compound : predname ^ args ;
    public final GrammarParser.compound_return compound() throws RecognitionException {
        GrammarParser.compound_return retval = new GrammarParser.compound_return();
        retval.start = input.LT(1);


        CommonTree root_0 = null;

        GrammarParser.predname_return predname5 =null;

        GrammarParser.args_return args6 =null;



        try {
            // Grammar.g:8:10: ( predname ^ args )
            // Grammar.g:8:12: predname ^ args
            {
            root_0 = (CommonTree)adaptor.nil();


            pushFollow(FOLLOW_predname_in_compound45);
            predname5=predname();

            state._fsp--;

            root_0 = (CommonTree)adaptor.becomeRoot(predname5.getTree(), root_0);

            pushFollow(FOLLOW_args_in_compound48);
            args6=args();

            state._fsp--;

            adaptor.addChild(root_0, args6.getTree());

            }

            retval.stop = input.LT(-1);


            retval.tree = (CommonTree)adaptor.rulePostProcessing(root_0);
            adaptor.setTokenBoundaries(retval.tree, retval.start, retval.stop);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
    	retval.tree = (CommonTree)adaptor.errorNode(input, retval.start, input.LT(-1), re);

        }

        finally {
        	// do for sure before leaving
        }
        return retval;
    }
    // $ANTLR end "compound"


    public static class args_return extends ParserRuleReturnScope {
        CommonTree tree;
        public Object getTree() { return tree; }
    };


    // $ANTLR start "args"
    // Grammar.g:9:1: args : LRB term ( COMMA term )* RRB ;
    public final GrammarParser.args_return args() throws RecognitionException {
        GrammarParser.args_return retval = new GrammarParser.args_return();
        retval.start = input.LT(1);


        CommonTree root_0 = null;

        Token LRB7=null;
        Token COMMA9=null;
        Token RRB11=null;
        GrammarParser.term_return term8 =null;

        GrammarParser.term_return term10 =null;


        CommonTree LRB7_tree=null;
        CommonTree COMMA9_tree=null;
        CommonTree RRB11_tree=null;

        try {
            // Grammar.g:9:6: ( LRB term ( COMMA term )* RRB )
            // Grammar.g:9:8: LRB term ( COMMA term )* RRB
            {
            root_0 = (CommonTree)adaptor.nil();


            LRB7=(Token)match(input,LRB,FOLLOW_LRB_in_args55); 
            LRB7_tree = 
            (CommonTree)adaptor.create(LRB7)
            ;
            adaptor.addChild(root_0, LRB7_tree);


            pushFollow(FOLLOW_term_in_args57);
            term8=term();

            state._fsp--;

            adaptor.addChild(root_0, term8.getTree());

            // Grammar.g:9:17: ( COMMA term )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( (LA3_0==COMMA) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // Grammar.g:9:18: COMMA term
            	    {
            	    COMMA9=(Token)match(input,COMMA,FOLLOW_COMMA_in_args60); 
            	    COMMA9_tree = 
            	    (CommonTree)adaptor.create(COMMA9)
            	    ;
            	    adaptor.addChild(root_0, COMMA9_tree);


            	    pushFollow(FOLLOW_term_in_args62);
            	    term10=term();

            	    state._fsp--;

            	    adaptor.addChild(root_0, term10.getTree());

            	    }
            	    break;

            	default :
            	    break loop3;
                }
            } while (true);


            RRB11=(Token)match(input,RRB,FOLLOW_RRB_in_args66); 
            RRB11_tree = 
            (CommonTree)adaptor.create(RRB11)
            ;
            adaptor.addChild(root_0, RRB11_tree);


            }

            retval.stop = input.LT(-1);


            retval.tree = (CommonTree)adaptor.rulePostProcessing(root_0);
            adaptor.setTokenBoundaries(retval.tree, retval.start, retval.stop);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
    	retval.tree = (CommonTree)adaptor.errorNode(input, retval.start, input.LT(-1), re);

        }

        finally {
        	// do for sure before leaving
        }
        return retval;
    }
    // $ANTLR end "args"


    public static class predname_return extends ParserRuleReturnScope {
        CommonTree tree;
        public Object getTree() { return tree; }
    };


    // $ANTLR start "predname"
    // Grammar.g:10:1: predname : TOKEN ;
    public final GrammarParser.predname_return predname() throws RecognitionException {
        GrammarParser.predname_return retval = new GrammarParser.predname_return();
        retval.start = input.LT(1);


        CommonTree root_0 = null;

        Token TOKEN12=null;

        CommonTree TOKEN12_tree=null;

        try {
            // Grammar.g:10:10: ( TOKEN )
            // Grammar.g:10:12: TOKEN
            {
            root_0 = (CommonTree)adaptor.nil();


            TOKEN12=(Token)match(input,TOKEN,FOLLOW_TOKEN_in_predname74); 
            TOKEN12_tree = 
            (CommonTree)adaptor.create(TOKEN12)
            ;
            adaptor.addChild(root_0, TOKEN12_tree);


            }

            retval.stop = input.LT(-1);


            retval.tree = (CommonTree)adaptor.rulePostProcessing(root_0);
            adaptor.setTokenBoundaries(retval.tree, retval.start, retval.stop);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
    	retval.tree = (CommonTree)adaptor.errorNode(input, retval.start, input.LT(-1), re);

        }

        finally {
        	// do for sure before leaving
        }
        return retval;
    }
    // $ANTLR end "predname"


    public static class term_return extends ParserRuleReturnScope {
        CommonTree tree;
        public Object getTree() { return tree; }
    };


    // $ANTLR start "term"
    // Grammar.g:11:1: term : ( atom_thingy | compound ) ;
    public final GrammarParser.term_return term() throws RecognitionException {
        GrammarParser.term_return retval = new GrammarParser.term_return();
        retval.start = input.LT(1);


        CommonTree root_0 = null;

        GrammarParser.atom_thingy_return atom_thingy13 =null;

        GrammarParser.compound_return compound14 =null;



        try {
            // Grammar.g:11:6: ( ( atom_thingy | compound ) )
            // Grammar.g:11:8: ( atom_thingy | compound )
            {
            root_0 = (CommonTree)adaptor.nil();


            // Grammar.g:11:8: ( atom_thingy | compound )
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0==TOKEN) ) {
                int LA4_1 = input.LA(2);

                if ( (LA4_1==COMMA||LA4_1==RRB) ) {
                    alt4=1;
                }
                else if ( (LA4_1==LRB) ) {
                    alt4=2;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("", 4, 1, input);

                    throw nvae;

                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("", 4, 0, input);

                throw nvae;

            }
            switch (alt4) {
                case 1 :
                    // Grammar.g:11:9: atom_thingy
                    {
                    pushFollow(FOLLOW_atom_thingy_in_term83);
                    atom_thingy13=atom_thingy();

                    state._fsp--;

                    adaptor.addChild(root_0, atom_thingy13.getTree());

                    }
                    break;
                case 2 :
                    // Grammar.g:11:23: compound
                    {
                    pushFollow(FOLLOW_compound_in_term87);
                    compound14=compound();

                    state._fsp--;

                    adaptor.addChild(root_0, compound14.getTree());

                    }
                    break;

            }


            }

            retval.stop = input.LT(-1);


            retval.tree = (CommonTree)adaptor.rulePostProcessing(root_0);
            adaptor.setTokenBoundaries(retval.tree, retval.start, retval.stop);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
    	retval.tree = (CommonTree)adaptor.errorNode(input, retval.start, input.LT(-1), re);

        }

        finally {
        	// do for sure before leaving
        }
        return retval;
    }
    // $ANTLR end "term"


    public static class atom_thingy_return extends ParserRuleReturnScope {
        CommonTree tree;
        public Object getTree() { return tree; }
    };


    // $ANTLR start "atom_thingy"
    // Grammar.g:12:1: atom_thingy : TOKEN ;
    public final GrammarParser.atom_thingy_return atom_thingy() throws RecognitionException {
        GrammarParser.atom_thingy_return retval = new GrammarParser.atom_thingy_return();
        retval.start = input.LT(1);


        CommonTree root_0 = null;

        Token TOKEN15=null;

        CommonTree TOKEN15_tree=null;

        try {
            // Grammar.g:12:13: ( TOKEN )
            // Grammar.g:12:15: TOKEN
            {
            root_0 = (CommonTree)adaptor.nil();


            TOKEN15=(Token)match(input,TOKEN,FOLLOW_TOKEN_in_atom_thingy96); 
            TOKEN15_tree = 
            (CommonTree)adaptor.create(TOKEN15)
            ;
            adaptor.addChild(root_0, TOKEN15_tree);


            }

            retval.stop = input.LT(-1);


            retval.tree = (CommonTree)adaptor.rulePostProcessing(root_0);
            adaptor.setTokenBoundaries(retval.tree, retval.start, retval.stop);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
    	retval.tree = (CommonTree)adaptor.errorNode(input, retval.start, input.LT(-1), re);

        }

        finally {
        	// do for sure before leaving
        }
        return retval;
    }
    // $ANTLR end "atom_thingy"

    // Delegated rules


 

    public static final BitSet FOLLOW_compound_in_start28 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_COMMA_in_start31 = new BitSet(new long[]{0x0000000000000800L});
    public static final BitSet FOLLOW_compound_in_start33 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_COMMA_in_start37 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_predname_in_compound45 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_args_in_compound48 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_LRB_in_args55 = new BitSet(new long[]{0x0000000000000800L});
    public static final BitSet FOLLOW_term_in_args57 = new BitSet(new long[]{0x0000000000000210L});
    public static final BitSet FOLLOW_COMMA_in_args60 = new BitSet(new long[]{0x0000000000000800L});
    public static final BitSet FOLLOW_term_in_args62 = new BitSet(new long[]{0x0000000000000210L});
    public static final BitSet FOLLOW_RRB_in_args66 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_TOKEN_in_predname74 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_atom_thingy_in_term83 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_compound_in_term87 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_TOKEN_in_atom_thingy96 = new BitSet(new long[]{0x0000000000000002L});

}
