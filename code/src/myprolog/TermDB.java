package myprolog;

import java.util.Arrays;
import java.util.Collections;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import myprolog.Prolog.Compound;
import myprolog.Prolog.PrologException;
import myprolog.Prolog.Unification;
import util.U;

import com.google.common.base.Function;
import com.google.common.base.Predicates;
import com.google.common.collect.ArrayListMultimap;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import com.google.common.collect.Multimap;


/** The little prolog-y database of term literals. By "literal" I mean grounded compound or atomic term. **/
public class TermDB {
	public List<Compound> allTerms;
	public Multimap<String,Compound> indexByPredname;
	
	public TermDB() {
		allTerms = Lists.newArrayList();
		indexByPredname = ArrayListMultimap.create();
	}
	
	public void add(Compound term) {
		assert ! Prolog.executingPredicateRegistry.containsKey(term.predname) : U.sf("predicate %s name conflicts with builtin", term.predname);
		allTerms.add(term);
		indexByPredname.put(term.predname, term);
	}
	
	/** match against a single queryTerm. **/
	public Iterable<Unification>  searchLiterals(final Compound queryTerm) {
		Iterable<Compound> termStream;
		termStream = indexByPredname.get(queryTerm.predname);
		
		Iterable<Unification> us = Iterables.transform(termStream, new Function<Compound,Unification>() {

			@Override
			public Unification apply(Compound termInDB) {
				return Prolog.unify(termInDB, queryTerm);
			}
		});
		us = Iterables.filter(us, Predicates.notNull());
		return us;
	}
	
	public static class Solution {
		public Unification varmap;
		public Compound[] fulfilledQueryTerms;
		public String toString() {
			return "SOLUTION[ " + varmap + " " + Arrays.toString(fulfilledQueryTerms) +" ]";
		}
		public Compound get(String varname) {
			return this.varmap.get(new Prolog.Variable(varname));
		}
	}
	public static class SearchState {
		/* a partial completion of the query.
		 * conceptually. (1) varmap, (2) fulfilledterms, (3) iterator for NEXT term
		 * but the #2 isn't actually needed, instead just an integer of the depth (i.e. number of already-fulfilled terms)
		 	< {},      [],      [fullfillments of 0'th qterm...]  >
		 	< {V=q},   [f(V)],  [fullfillments of 1'st qterm...]  >
		 */
		public Unification varmap;
		public int depth;
		public Iterator<Unification> unificationsForNextQueryTerm;  // for queryTerm[depth]
	}

	/** queryTerms are the conjuncts to be fulfilled in depth-first backtracking search
	 * making this a streaming iterator seems painful, so just return all solutions (very un-prolog-y)
	 **/
	public Iterable<Solution> search(Compound rawQueryTerms[]) throws PrologException {
		if (rawQueryTerms.length == 0) return Collections.emptyList();
		List<Solution> allSolutions = Lists.newArrayList();
		LinkedList<SearchState> stack = Lists.newLinkedList();
		
		SearchState start = new SearchState();
		start.varmap = new Prolog.Unification();
		start.depth = 0;
		Compound qt = Prolog.doSubstitution(rawQueryTerms[0], start.varmap);
		if (Prolog.executingPredicateRegistry.containsKey(qt.predname)) assert false : "can't have EP first qterm";
		start.unificationsForNextQueryTerm = searchLiterals(qt).iterator();
		stack.add(start);
		
		do {
			SearchState top = stack.peek();
			if (top.unificationsForNextQueryTerm.hasNext()) {
				// this is only a partial varmap, for the current queryterm
				Unification newVarMap = top.unificationsForNextQueryTerm.next();
				if (top.depth == rawQueryTerms.length) {
					// this is a completed solution
					Solution sol = new Solution();
					sol.varmap = top.varmap.copyAndAdd(newVarMap);
					sol.fulfilledQueryTerms = Prolog.doSubstitution(rawQueryTerms, sol.varmap);
					allSolutions.add(sol);
				}
				else {
					// this is a partial solution. initiate child search
					SearchState newstate = new SearchState();
					newstate.depth = top.depth+1;
					newstate.varmap = top.varmap.copyAndAdd(newVarMap);
					Compound queryTerm = Prolog.doSubstitution(rawQueryTerms[top.depth], newstate.varmap);
					// success
//					U.p("Q on new: " + queryTerm);
					newstate.unificationsForNextQueryTerm = searchLiterals(queryTerm).iterator();
					stack.push(newstate);						
				}
			}
			else {
				// done with solutions for this query term (given its parent searchstate)
				stack.pop();
			}
			
		} while (stack.size() > 0);
		
		return allSolutions;
	}
	public boolean isExecutingPredicate(Compound qt) {
		return Prolog.executingPredicateRegistry.containsKey(qt.predname);
	}
	/** return TRUE if everything is ok.
	 * return FALSE if (1) the compound represents an executingpredicate, and (2) that executingpredicate returns false.
	 */
	public boolean callExecutingPredicate(Compound qt) throws PrologException {
		Prolog.ExecutingPredicateImpl impl = Prolog.executingPredicateRegistry.get(qt.predname);
		return Prolog.callExecutingPredicate(impl, qt);
	}
	public Iterable<Solution> search(List<Compound> rawQueryTerms) throws PrologException {
		return search(rawQueryTerms.toArray(new Compound[0]));
	}
	
	public static void main(String args[]) throws PrologException {
		TermDB db = new TermDB();
		db.add( Prolog.P("f","a") );
		db.add( Prolog.P("f","b") );
		db.add( Prolog.P("g","a") );

		List<Solution> results;
		results = Lists.newArrayList(db.search(
				new Prolog.Compound[]{ P("f","V"), P("g","V")}
		));
		U.p(results);

	}
	
	
	static Compound P(String predname, String... args) throws PrologException {
		return Prolog.P(predname, args);
	}

}
