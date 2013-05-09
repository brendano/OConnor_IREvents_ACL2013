
import java.util.List;
import java.util.Set;

import myprolog.Prolog.Compound;
import myprolog.Prolog.PrologException;
import myprolog.Prolog.Variable;
import myprolog.PrologParser;
import myprolog.TermDB.Solution;

import org.antlr.runtime.RecognitionException;

import struct.Sentence;
import util.U;

import com.google.common.collect.HashMultimap;
import com.google.common.collect.Lists;
import com.google.common.collect.Multimap;
import com.google.common.collect.Sets;

/** Structural grep with a Prolog query. **/
public class PGrep {

	public static String compressedView(Sentence sent, Solution sol) {
		StringBuilder sb = new StringBuilder();
		Multimap<Integer,String> m = HashMultimap.create();
		for (Variable v : sol.varmap.vars()) {
			String s = sol.varmap.get(v).predname;
			String x[] = s.replace("()","").split(":");
			if (x.length==1) continue;
			int i = Integer.parseInt(x[x.length-1]);
			m.put(i, v.toString());
		}
		for (int i=0; i < sent.T(); i++) {
			if (m.containsKey(i)) {
				sb.append(U.sf("**%s**%s",sent.word(i), Lists.newArrayList(m.get(i)))); 
			} else {
				sb.append(sent.word(i));
			}
			sb.append(" ");
		}
		return sb.toString();
	}

	

	public static void runPrologQuery(PathAn runner, String queryStr) throws RecognitionException, PrologException {
		List<Compound> q = PrologParser.parseTermString(queryStr);
		Set<String> whitelist = Sets.newHashSet();
		for (Compound qt : q) whitelist.add(qt.predname);
		U.p("QUERY " + q);
		for (final Sentence sent : runner.sentenceIter()) {
//			U.pf("\n=== %s\t%s\t%s\n", sent.docid, sent.sentid, StringUtils.join(sent.stringTokens()));
			sent.populateTermDB(null);
			List<Solution> results = Lists.newArrayList(sent.termDB.search(q));
			if (results.size()==0) { continue; }
//			U.p(Arrays.toString(sol.fulfilledQueryTerms));
//			for (Compound c : sent.termDB.allTerms) U.p(c);
			U.pf("\n=== %s\t%s\n", sent.docidStr, sent.sentidStr);
			for (Solution sol : results) {
				U.p(compressedView(sent, sol));
				for (Variable v : sol.varmap.vars()) {
					String s = sol.varmap.get(v).toString();
					if (!s.contains(":")) {
						U.pf("%s=%s ", v.toString(), s);
					}
				}
				U.pf("\n");
			}
			U.p(queryStr);
		}
		System.out.flush();
	}
	
	public static void main(String args[]) throws Exception {
		PathAn runner = new PathAn();
		runPrologQuery(runner, args[0]);
	}
}
