package myprolog;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import util.U;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;

import edu.stanford.nlp.util.StringUtils;

/** 
 * this isn't really prolog but like prolog 
 * in its
 * strictly depth-first backtracking proceduralness
 * no predicate definitions -- just db queries
 * disallow structured predicate arguments: everything must be flat.
 * so unification is brain-dead to implement
 * goal: allow easy extensions to java functions.
 **/
public class Prolog {
	
	public static abstract class Term {
	}
	
	public static class Variable extends Term {
		String varname;
		public Variable(String varname) { this.varname = varname; }
		@Override
		public String toString() { return varname; }
		@Override
		public boolean equals(Object o) {
			if ( ! (o instanceof Variable)) return false;
			return ((Variable) o).varname.equals(varname);
		}
		@Override
		public int hashCode() {
			return varname.hashCode();
		}
	}
	
	/** an atomic term is a zero-arity compound.
	 * so all grounded terms are Compounds (for whom all nested terms are compounds too, I guess)
	 */
	public static class Compound extends Term {
		public String predname;
		public Term args[];
		public int arity() { return args.length; }
		public Compound() { }
		public Compound(String predname, Term args[]) {
			this.predname=predname; this.args=args;
		}
		public Compound(String predname, List<Term> args) {
			this.predname=predname;
			this.args = new Term[args.size()];
			for (int i=0; i < args.size(); i++) this.args[i] = args.get(i);
		}
		public String toString(){
			List<String> s = Lists.newArrayList();
			for (Term t : args) {
				s.add(t==null ? "NULL" : t.toString());
			}
			return U.sf("%s(%s)", predname, StringUtils.join(s,","));
		}
		@Override
		public boolean equals(Object o2) {
			if (! (o2 instanceof Compound)) return false;
			// tricky. force a very literal match. no unification.
			String s1 = this.toString();
			String s2 = ((Compound) o2).toString();
			return s1.equals(s2);
		}
		@Override
		public int hashCode() { throw new RuntimeException("unimplemented"); }
	}
	
	static Map<String,ExecutingPredicateImpl> executingPredicateRegistry = Maps.newHashMap();
	
	public static void registerExecutingPredicate(String predname, ExecutingPredicateImpl impl) {
		executingPredicateRegistry.put(predname, impl);
	}
	public static boolean callExecutingPredicate(ExecutingPredicateImpl impl, Compound c) throws PrologException {
		for (Term t : c.args)
			if ( ! (t instanceof Compound))
				assert false : "can only call ExecutingPredicate when all args are grounded.";
		return impl.apply((Compound[]) c.args);
	}


	/** Implement this to be the Java-backed implementation of the predicate.
	 */
	public static interface ExecutingPredicateImpl {
		public boolean apply(Compound args[]) throws PrologException;
	}
	
	/** create a new atomic compound (with no arguments) **/
	public static Compound P(String predname) {
		Compound p = new Compound();
		p.predname = predname;
		p.args = new Term[0];
		return p;
	}
	
	/** create a new compound, applying arg parsing to arguments. **/
	public static Compound P(String predname, String... args) {
		Compound p = new Compound();
		p.predname = predname;
		p.args = new Term[args.length];
		for (int i=0; i < p.arity(); i++) {
			p.args[i] = interpretStringArg(args[i]);
		}
		return p;
	}
	
	/** create a new compound, taking direct terms as argument. **/
	public static Compound P(String predname, Term... args) {
		Compound p = new Compound();
		p.predname = predname;
		p.args = args;
		return p;
	}
	/** create a new Variable with the given name. 
	 * warning: this allows you to make a lowercased variable name.
	 **/
	public static Variable V(String varname) {
		return new Variable(varname);
	}
	
	public static class Unification {
		private Map<Variable, Compound> variable2compound;
		public Unification() { variable2compound = Maps.newHashMap(); }
		
		public final Unification copy() {
			Unification u = new Unification();
			u.variable2compound = Maps.newHashMap(this.variable2compound);
			return u;
		}
		public Unification copyAndAdd(Unification u2) {
			Unification uNew = copy();
			uNew.update(u2);
			return uNew;
		}
		
		/** Copy RHS' assignments into this one. 
		 * @return true if succeeded, false if non-unifiable conflicts. **/
		public boolean update(Unification otherUtoCopy) {
			for (Variable rhsKey : otherUtoCopy.variable2compound.keySet()) {
				if (variable2compound.containsKey(rhsKey)) {
					assert false; // TODO can this happen here?
					// if we had real prolog this step would be more complicated, would need full-out unification
					
				} else {
					variable2compound.put(rhsKey, otherUtoCopy.variable2compound.get(rhsKey));
				}
			}
			return true;
		}
		public final boolean has(Variable v) {
			return variable2compound.containsKey(v);
		}
		public final Compound get(Variable v) {
			if ( ! has(v)) assert false;
			return variable2compound.get(v);
		}
		public void put(Variable v, Compound c) {
			if (has(v)) assert false;
			variable2compound.put(v, c);
		}
		public final int size() { return this.variable2compound.size(); }
		public Collection<Variable> vars() { return this.variable2compound.keySet(); }
		/** for testing only! **/
		public ImmutableMap<Variable,Compound> map() {
			return ImmutableMap.copyOf(variable2compound);
		}
		public String toString() {
			return "VARMAP" + variable2compound.toString() + "";
		}
		
	}
	/** 
	 * Return null if unification fails.
	 * This is a really lame form of unification: leftside must be GROUNDED, i.e. no free variables.
	 * it's more like lightly structured pattern matching. 
	 * **/
	public static Unification unify(Compound grounded, Compound query) {
		if ( ! grounded.predname.equals(query.predname)) return null;
		if (grounded.arity() != query.arity()) return null;
		Unification unification = new Unification();
		for (int i=0; i < query.arity(); i++) {
			assert grounded.args[i] instanceof Compound;
			
			if (query.args[i] instanceof Variable) {
				// unification has to succeed since grounded.
				Variable qv = (Variable) query.args[i];
				Compound gc = (Compound) grounded.args[i];
				if (unification.has(qv)) {
					if ( ! unification.get(qv).equals(gc)) {
						return null;
					}
				} else {
					unification.put(qv, gc);	
				}
			}
			else if (query.args[i] instanceof Compound) {
				Unification subUnification = unify((Compound) grounded.args[i], (Compound) query.args[i]);
				if (subUnification==null) return null;
				assert subUnification.map().size()==0 : "unifying grounded atoms should yield empty var2cmpd!";
				// since our restricted form of unification forces this to be empty, we're done here
			} else {
				assert false : "bad type for substitution";
			}
		}
		return unification;
	}
	
	public static Compound doSubstitution(Compound input, Unification u) throws PrologException {
		Compound output = new Compound(input.predname, new Term[input.arity()]);
		for (int i=0; i < input.arity(); i++) {
			Term term = input.args[i];
			if ((term instanceof Variable) && u.has((Variable) term)) {
				output.args[i] = u.get((Variable) term);
			}
			else if (term instanceof Compound && ((Compound) term).arity() > 0) {
				throw new PrologException("We don't handle nested compounds for substitution yet");
			}
			else {
				output.args[i] = term;
			}
		}
		return output;
	}
	public static Compound[] doSubstitution(Compound inputs[], Unification u) throws PrologException {
		Compound outputs[] = new Compound[inputs.length];
		for (int i=0; i < inputs.length; i++) {
			outputs[i] = doSubstitution(inputs[i], u);
		}
		return outputs;
	}
	
	/** this routine is where to handle things like regex extensions **/
	public static Term interpretStringArg(String arg) {
		if (Character.isUpperCase(arg.charAt(0))) {
			Variable v = new Variable(arg);
			return v;
		} else {
			return P(arg);
		}
	}
	static boolean legalPredname(String p) { return p.matches("^[a-zA-Z][a-zA-Z0-9_]*$"); } 
	
	public static class PrologException extends Exception {
		public PrologException(String msg) { super(msg); }
	}
}
