package myprolog;

import myprolog.Prolog.Compound;
import myprolog.Prolog.PrologException;
import util.U;

/** these don't work yet... **/
public class ExecutingPredicates {
	public static class Neq implements Prolog.ExecutingPredicateImpl {
		@Override
		public boolean apply(Compound[] args) throws PrologException {
			if (args.length!=2) throw new Prolog.PrologException("bad args");
			return Prolog.unify(args[0], args[1]) == null;
		}
	}
	public static class Eq implements Prolog.ExecutingPredicateImpl {
		@Override
		public boolean apply(Compound[] args) throws PrologException {
			if (args.length!=2) throw new Prolog.PrologException("bad args");
			U.pf("RUN %s %s\n", args[0], args[1]);
			return Prolog.unify(args[0], args[1]) != null;
		}
	}
	
	static {
		Prolog.registerExecutingPredicate("Neq", new Neq());
		Prolog.registerExecutingPredicate("Eq", new Eq());
	}
}
