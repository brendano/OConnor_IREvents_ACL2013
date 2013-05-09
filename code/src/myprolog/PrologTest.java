package myprolog;

import java.util.List;

import junit.framework.TestCase;
import myprolog.Prolog.Compound;
import myprolog.Prolog.PrologException;
import myprolog.Prolog.Term;
import myprolog.Prolog.Unification;
import myprolog.Prolog.Variable;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

public class PrologTest extends TestCase {

	public void testSimpleUnification() throws PrologException {
		Compound p1 = P("p", "a", "b");
		Compound p2 = P("p", "a", "b");
		assertUnify(p1, p2);
		assertNotSame(p1, p2);
		Compound q = P("q", "a", "b");
		assertNull(Prolog.unify(p1, q));
		assertNotUnify(P("q","a"), q);
		
		assertEquals(new Variable("asdf"), new Variable("asdf"));
		assertNotSame(new Variable("asdfq"), new Variable("asdf"));
		
	}
	public void testMoreUnification() throws PrologException {
		Unification u;
		u = Prolog.unify(P("p","a","b"), P("p","a","V"));
		assertNotNull(u);
		assertEquals(u.map(), new ImmutableMap.Builder<Variable,Compound>()
				.put(new Variable("V"), P("b")).build());
		Compound f_g_a = new Compound("f", new Term[]{P("g", "a")});
		Compound f_G = P("f","G");
		u = Prolog.unify(f_g_a, f_G);
		assertNotNull(u);
		assertEquals(u.map(),
				new ImmutableMap.Builder<Variable,Compound>()
				.put(new Variable("G"), P("g","a")).build());
		assertEquals(f_g_a, Prolog.doSubstitution(f_G, u));
		assertNotSame(f_G, Prolog.doSubstitution(new Compound[]{f_G}, u)[0]);
	}
	public void testSimpleSearch() throws PrologException {
		TermDB db = new TermDB();
		db.add( Prolog.P("f","a") );
		db.add( Prolog.P("f","b") );
		db.add( Prolog.P("g","a") );
		List<Unification> results;
		results = Lists.newArrayList(db.searchLiterals( 
				new Compound("e", new Term[]{})));
		assertEquals(0, results.size());
		results = Lists.newArrayList(db.searchLiterals( 
				new Compound("f", new Term[]{})));
		assertEquals(0, results.size());
		results = Lists.newArrayList(db.searchLiterals( 
				new Compound("f", new Term[]{ new Prolog.Variable("V") } )));
		assertEquals(2, results.size());
		assertEquals("VARMAP{V=a()}", results.get(0).toString());
		assertEquals("VARMAP{V=b()}", results.get(1).toString());
		results = Lists.newArrayList(db.searchLiterals( 
				new Compound("g", new Term[]{ new Prolog.Variable("V") } )));
		assertEquals(1, results.size());
		results = Lists.newArrayList(db.searchLiterals( 
				new Compound("q", new Term[]{ new Prolog.Variable("V") } )));
		assertEquals(0, results.size());
	}
	public void testBacktrackingSearch() throws PrologException {
		TermDB db = new TermDB();
		db.add( P("f","a") );
		db.add( P("f","b") );
		db.add( P("g","a") );
		List<TermDB.Solution> results;
		results = Lists.newArrayList(db.search(
				new Prolog.Compound[]{}));
		assertEquals(0, results.size());
		results = Lists.newArrayList(db.search(
				new Prolog.Compound[]{ P("q")}
		));
		assertEquals(0, results.size());
		results = Lists.newArrayList(db.search(
				new Prolog.Compound[]{ P("f","z")}
		)); assertEquals(0, results.size());
		
		results = Lists.newArrayList(db.search(
				new Prolog.Compound[]{ P("f","V")}
		));
		assertEquals(2, results.size());
		assertEquals(P("f","a"), results.get(0).fulfilledQueryTerms[0]);
		assertEquals(P("f","b"), results.get(1).fulfilledQueryTerms[0]);
		
		results = Lists.newArrayList(db.search(
				new Prolog.Compound[]{ P("f","V"), P("g","V")}
		));
		assertEquals(1, results.size());
		assertEquals(P("f","a"), results.get(0).fulfilledQueryTerms[0]);
		assertEquals(P("g","a"), results.get(0).fulfilledQueryTerms[1]);
		
		results = Lists.newArrayList(db.search(
				new Prolog.Compound[]{ P("f","V"), P("f","b"), P("g","V")}
		));
		assertEquals(1, results.size());
	}
	
	public void testMoreBacktrackingSearch() throws PrologException {
		TermDB db = new TermDB();
		db.add( P("f","a","b") );
		db.add( P("f","a","c") );
		db.add( P("g","a","b") );
		List<TermDB.Solution> results;
		
		results = Lists.newArrayList(db.search(
				new Prolog.Compound[]{ P("f","V","b") }
		));
		assertEquals(1, results.size());

		results = Lists.newArrayList(db.search(
				new Prolog.Compound[]{ P("f","V","S"), P("g","V","b")}
		));
		assertEquals(2, results.size());
		assertEquals(P("f","a","b"), results.get(0).fulfilledQueryTerms[0]);
		assertEquals(P("g","a","b"), results.get(0).fulfilledQueryTerms[1]);
		assertEquals(P("f","a","c"), results.get(1).fulfilledQueryTerms[0]);
		assertEquals(P("g","a","b"), results.get(1).fulfilledQueryTerms[1]);
		
		results = Lists.newArrayList(db.search(
				new Prolog.Compound[]{ P("f","V","b"), P("g","a","V")}
		));
		assertEquals(0, results.size());

}

	
	static Compound P(String predname, String... args) throws PrologException {
		return Prolog.P(predname, args);
	}
	static void assertNotUnify(Compound a, Compound b) {
		assertNull(Prolog.unify(a,b));
	}
	static void assertUnify(Compound a, Compound b) {
		assertNotNull(Prolog.unify(a,b));
	}
}
