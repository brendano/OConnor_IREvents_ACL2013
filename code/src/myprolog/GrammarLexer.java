package myprolog;
// $ANTLR 3.4 Grammar.g 2012-11-29 12:54:33

import org.antlr.runtime.CharStream;
import org.antlr.runtime.EarlyExitException;
import org.antlr.runtime.Lexer;
import org.antlr.runtime.MismatchedSetException;
import org.antlr.runtime.NoViableAltException;
import org.antlr.runtime.RecognitionException;
import org.antlr.runtime.RecognizerSharedState;

@SuppressWarnings({"all", "warnings", "unchecked"})
public class GrammarLexer extends Lexer {
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
    // delegators
    public Lexer[] getDelegates() {
        return new Lexer[] {};
    }

    public GrammarLexer() {} 
    public GrammarLexer(CharStream input) {
        this(input, new RecognizerSharedState());
    }
    public GrammarLexer(CharStream input, RecognizerSharedState state) {
        super(input,state);
    }
    public String getGrammarFileName() { return "Grammar.g"; }

    // $ANTLR start "COMMA"
    public final void mCOMMA() throws RecognitionException {
        try {
            int _type = COMMA;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // Grammar.g:14:7: ( ',' )
            // Grammar.g:14:9: ','
            {
            match(','); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "COMMA"

    // $ANTLR start "LSB"
    public final void mLSB() throws RecognitionException {
        try {
            int _type = LSB;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // Grammar.g:16:9: ( '[' )
            // Grammar.g:16:18: '['
            {
            match('['); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "LSB"

    // $ANTLR start "RSB"
    public final void mRSB() throws RecognitionException {
        try {
            int _type = RSB;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // Grammar.g:17:9: ( ']' )
            // Grammar.g:17:18: ']'
            {
            match(']'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "RSB"

    // $ANTLR start "LCB"
    public final void mLCB() throws RecognitionException {
        try {
            int _type = LCB;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // Grammar.g:19:9: ( '{' )
            // Grammar.g:19:18: '{'
            {
            match('{'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "LCB"

    // $ANTLR start "RCB"
    public final void mRCB() throws RecognitionException {
        try {
            int _type = RCB;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // Grammar.g:20:9: ( '}' )
            // Grammar.g:20:18: '}'
            {
            match('}'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "RCB"

    // $ANTLR start "LRB"
    public final void mLRB() throws RecognitionException {
        try {
            int _type = LRB;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // Grammar.g:22:9: ( '(' )
            // Grammar.g:22:18: '('
            {
            match('('); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "LRB"

    // $ANTLR start "RRB"
    public final void mRRB() throws RecognitionException {
        try {
            int _type = RRB;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // Grammar.g:23:9: ( ')' )
            // Grammar.g:23:18: ')'
            {
            match(')'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "RRB"

    // $ANTLR start "TOKEN"
    public final void mTOKEN() throws RecognitionException {
        try {
            int _type = TOKEN;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // Grammar.g:25:9: ( (~ ( COMMA | LSB | RSB | LCB | RCB | LRB | RRB ) )+ )
            // Grammar.g:25:17: (~ ( COMMA | LSB | RSB | LCB | RCB | LRB | RRB ) )+
            {
            // Grammar.g:25:17: (~ ( COMMA | LSB | RSB | LCB | RCB | LRB | RRB ) )+
            int cnt1=0;
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( ((LA1_0 >= '\u0000' && LA1_0 <= '\'')||(LA1_0 >= '*' && LA1_0 <= '+')||(LA1_0 >= '-' && LA1_0 <= 'Z')||LA1_0=='\\'||(LA1_0 >= '^' && LA1_0 <= 'z')||LA1_0=='|'||(LA1_0 >= '~' && LA1_0 <= '\uFFFF')) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // Grammar.g:
            	    {
            	    if ( (input.LA(1) >= '\u0000' && input.LA(1) <= '\'')||(input.LA(1) >= '*' && input.LA(1) <= '+')||(input.LA(1) >= '-' && input.LA(1) <= 'Z')||input.LA(1)=='\\'||(input.LA(1) >= '^' && input.LA(1) <= 'z')||input.LA(1)=='|'||(input.LA(1) >= '~' && input.LA(1) <= '\uFFFF') ) {
            	        input.consume();
            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt1 >= 1 ) break loop1;
                        EarlyExitException eee =
                            new EarlyExitException(1, input);
                        throw eee;
                }
                cnt1++;
            } while (true);


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "TOKEN"

    public void mTokens() throws RecognitionException {
        // Grammar.g:1:8: ( COMMA | LSB | RSB | LCB | RCB | LRB | RRB | TOKEN )
        int alt2=8;
        int LA2_0 = input.LA(1);

        if ( (LA2_0==',') ) {
            alt2=1;
        }
        else if ( (LA2_0=='[') ) {
            alt2=2;
        }
        else if ( (LA2_0==']') ) {
            alt2=3;
        }
        else if ( (LA2_0=='{') ) {
            alt2=4;
        }
        else if ( (LA2_0=='}') ) {
            alt2=5;
        }
        else if ( (LA2_0=='(') ) {
            alt2=6;
        }
        else if ( (LA2_0==')') ) {
            alt2=7;
        }
        else if ( ((LA2_0 >= '\u0000' && LA2_0 <= '\'')||(LA2_0 >= '*' && LA2_0 <= '+')||(LA2_0 >= '-' && LA2_0 <= 'Z')||LA2_0=='\\'||(LA2_0 >= '^' && LA2_0 <= 'z')||LA2_0=='|'||(LA2_0 >= '~' && LA2_0 <= '\uFFFF')) ) {
            alt2=8;
        }
        else {
            NoViableAltException nvae =
                new NoViableAltException("", 2, 0, input);

            throw nvae;

        }
        switch (alt2) {
            case 1 :
                // Grammar.g:1:10: COMMA
                {
                mCOMMA(); 


                }
                break;
            case 2 :
                // Grammar.g:1:16: LSB
                {
                mLSB(); 


                }
                break;
            case 3 :
                // Grammar.g:1:20: RSB
                {
                mRSB(); 


                }
                break;
            case 4 :
                // Grammar.g:1:24: LCB
                {
                mLCB(); 


                }
                break;
            case 5 :
                // Grammar.g:1:28: RCB
                {
                mRCB(); 


                }
                break;
            case 6 :
                // Grammar.g:1:32: LRB
                {
                mLRB(); 


                }
                break;
            case 7 :
                // Grammar.g:1:36: RRB
                {
                mRRB(); 


                }
                break;
            case 8 :
                // Grammar.g:1:40: TOKEN
                {
                mTOKEN(); 


                }
                break;

        }

    }


 

}
