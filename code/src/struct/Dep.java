package struct;

/** Dependency edge (triple).
 * 
 *  Nomenclature:
 *  "dep" is the dependency edge.
 *  "relation" is the type of grammatical relation for the edge.
 *  "governor" is the dominating node.  (aka "head")
 *  "child" is the subordinate node.  (aka "dependent" but that's an ambig term)
**/
public class Dep {
	public String rel;
	public int gov;
	public int child;
}
