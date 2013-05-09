package myprolog;
import java.util.List;

import myprolog.Prolog.Compound;
import myprolog.Prolog.PrologException;
import myprolog.Prolog.Term;

import org.antlr.runtime.ANTLRStringStream;
import org.antlr.runtime.CharStream;
import org.antlr.runtime.CommonTokenStream;
import org.antlr.runtime.RecognitionException;
import org.antlr.runtime.tree.CommonTree;

import util.U;

import com.google.common.collect.Lists;



public class PrologParser {
	
	/** e.g.
	 * 
	 * f(a,b)
	 * or
	 * f(a,b), g(q)
	 * @throws RecognitionException 
	 * @throws PrologException 
	 * 
	 */
	public static List<Compound> parseTermString(String str) throws RecognitionException, PrologException {
		CharStream input = new ANTLRStringStream(str);
		GrammarLexer lex = new GrammarLexer(input);
		CommonTokenStream tokens = new CommonTokenStream(lex);
		GrammarParser parser = new GrammarParser(tokens);
		GrammarParser.start_return r = parser.start();
		CommonTree root = (CommonTree) r.tree;
//		U.p(root.toStringTree());
		List<Prolog.Compound> topTerms = Lists.newArrayList();
		if ( ((CommonTree)root.getChildren().get(0)).getType() == GrammarParser.LRB) {
			// if it's a singleton, it's directly the root
			topTerms.add((Compound) parseTermFromSubtree(root));
		} else {
			for (Object _child : root.getChildren()) {
				CommonTree child = (CommonTree) _child;
				if (child.getType() == GrammarParser.TOKEN){
					// this is the predname head of a top compound
					Compound c = (Compound) parseTermFromSubtree(child);
					topTerms.add(c);
				}
			}			
		}
		U.p("---");
		return topTerms;
		
	}
	static Term parseTermFromSubtree(CommonTree tree) throws PrologException {
//		U.p("HANDLING " +tree);
//		U.p("TYPE " + tree.getType());
//		U.p("TEXT " + tree.getText());
//		U.p("CHILDREN " + tree.getChildren());
		if (tree.getChildren()!=null && tree.getChildren().size() > 0 ) {
			// it's a compound
			String predname = tree.getText().trim();
			List<Term> children = Lists.newArrayList();
			for (Object _c : tree.getChildren()) {
				CommonTree c = (CommonTree) _c;
				if (c.getType() == GrammarParser.COMMA) {
					continue;
				}
				else if (c.getType() == GrammarParser.TOKEN) {
					children.add(parseTermFromSubtree(c));
				}
			}
			return new Compound(predname, children);
		}
		else {
			return Prolog.interpretStringArg(tree.getText().trim());
		}
	}
	
	public static void main(String[] args) throws Exception {
		U.p(parseTermString(args[0]));
	}
}
