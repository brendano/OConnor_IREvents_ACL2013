package a;
import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import java.util.Set;

import org.codehaus.jackson.JsonProcessingException;

import struct.Dep;
import struct.Sentence;
import struct.Token;
import util.BasicFileIO;
import util.U;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Queues;
import com.google.common.collect.Sets;

import edu.stanford.nlp.stats.ClassicCounter;
import edu.stanford.nlp.stats.Counters;


/** Syntax analysis routines **/
public class Syntax {
	/**
	 * find the maximal governor of the span, only for paths contained completely within the span. 
	 * this is a catena-like notion.
	 * So using dep dominance. alternative would be stanford SemanticHeadFinder, 
	 * then tregex can do it super easy
	 * the algo we use is silly, could be made faster
	 **/
	public static int findPhraseHead(Sentence sent, int startTok, int endTok) {
		assert startTok < endTok;
		Map<Integer,Integer> tokensMaximalGov = Maps.newHashMap();
		for (int t=startTok; t < endTok; t++) {
			int maximalGov = t;
			// BF traversal to find upward paths
			// the queue will only have things within the span.
			Queue<Integer> queue = Queues.newArrayDeque();
			queue.add(t);
			Set<Integer> allSeen = Sets.newHashSet();
			while (!queue.isEmpty()) {
				int s = queue.remove();
				for (Dep dep : sent.child2govs.get(s)) {
					int r = dep.gov;
					if (startTok <= r && r < endTok && !allSeen.contains(r)) {
						maximalGov = r;
						queue.add(r);
					}
					allSeen.add(r);
				}
			}
			tokensMaximalGov.put(t, maximalGov);
		}
		// um, ok, take the majority vote I guess.  would be better to break ties via... rightmost?  basically to correct for parsing errors.
		ClassicCounter<Integer> c = new ClassicCounter<Integer>( tokensMaximalGov.values() );
		return Counters.argmax(c);
	}
	
	public static int findNominalHead(Sentence sent, int t) {
		List<SynPath> paths = basePaths(sent, t);
		for (int plen=1; plen<=20; plen++) {
			for (SynPath p : paths) {
				if (isNominal(sent, p.endpointNode())) {
					return p.endpointNode();
				}
			}
			paths = enlargenPaths(sent, paths);
		}
		return -1;
	}
	
	public static int findMaximalNominalHead(Sentence sent, int t) {
		// traverse amod and nn edges.
		
		for (int plen=1; plen <= 5; plen++) {
			for (Dep upEdge : sent.child2govs.get(t)) {
				if ((upEdge.rel.equals("amod") || upEdge.rel.equals("nn")) &&
						isNominal(sent, upEdge.gov)) {
					// traverse
					t = upEdge.gov;
					continue;
				}
			}
			// might as well quit immediately
			break;
		}
		return t;
	}
	
	public static class SynPath {
		public int startToken=-1;
		public List<PathArc> pathArcs;
		private Set<Integer> _tokenIndexes;
		
		public Set<Integer> tokenIndexes() {
			if (_tokenIndexes == null) {
				_tokenIndexes = Sets.newHashSet();
				_tokenIndexes.add(startToken);
				for (PathArc pe : pathArcs) {
					_tokenIndexes.add(pe.endpointNode);
				}
			}
			return _tokenIndexes;
		}
		
		public SynPath() { pathArcs = Lists.newArrayList(); }
		public SynPath copyWithAppend(PathArc newPE) {
			SynPath p = new SynPath();
			p.startToken = this.startToken;
			p.pathArcs = Lists.newArrayList(this.pathArcs);
			p.pathArcs.add(newPE);
			return p;
		}
		public boolean containsNode(int node) {
			if (startToken==node) return true;
			for (PathArc e : pathArcs) {
				if (e.endpointNode==node) return true;
			}
			return false;
		}
		public int endpointNode() {
			return pathArcs.get( pathArcs.size()-1 ).endpointNode;
		}
		public List<PathArc> pathEdgesExceptLast() {
			List<PathArc> ret = Lists.newArrayList();
			for (int i=0; i < pathArcs.size()-1; i++)
				ret.add(pathArcs.get(i));
			return ret;
		}
		public PathArc lastEdge() { return pathArcs.get(pathArcs.size()-1); }
		public String toString(List<String> tokens) {
			StringBuffer sb = new StringBuffer();
			sb.append(U.sf("%s ", tokens.get(startToken)));
			for (PathArc e : pathArcs) {
				String arrow = e.direction==PathArc.Direction.UP ? "->" : "<-";
				sb.append(U.sf("%s%s%s %s ",arrow, e.deprel, arrow, tokens.get(e.endpointNode) ));
			}
			return sb.toString();
		}
		public String toString() {
			List<String> nums = Lists.newArrayList();
			for (int t=0; t < 200; t++) nums.add(Integer.toString(t));
			return toString(nums);
		}
		public String toString(Sentence sent) {
			List<String> tokens = Lists.newArrayList();
//			for (int t=0; t < sent.T(); t++) tokens.add(U.sf("%s:%d", sent.stringTokens().get(t), t));
			for (int t=0; t < sent.T(); t++) tokens.add(U.sf("%s:%d", sent.tokens.get(t).lemma, t));
			return toString(tokens);
		}
	}
	public static class PathArc {
		public Direction direction;
		public int endpointNode; // token ID
		public String deprel;
		
		public PathArc(Direction d, int e, String r) {
			direction=d; endpointNode=e; deprel=r;
		}
		public static enum Direction {
			UP, DOWN;
			public String toString() {
				return this==UP ? "->" : "<-";
			}
		}
	}
	
	public static boolean isOKforPath(Dep d) {
		if (d.rel.equals("parataxis")) return false;
//		if (d.rel.equals("appos")) return false;
		if (d.rel.equals("det")) return false;
		if (d.rel.equals("dep")) return false;
		return true;
	}
	
	public static boolean isOKforVerbalPath(Dep d) {
		if (!isOKforPath(d)) return false;
		if (d.rel.startsWith("conj")) return false;
		return true;
	}
	
	/** BFS style. Return new one-longer paths. **/
	static List<SynPath> enlargenPaths(Sentence sent, List<SynPath> seedPaths) {
		List<SynPath> newPaths = Lists.newArrayList();
		
		for (SynPath seedPath : seedPaths) {
			int endpoint = seedPath.endpointNode();
			for (Dep d : sent.child2govs.get(endpoint)) {
				if (seedPath.containsNode(d.gov)) continue;
				if (!isOKforVerbalPath(d)) continue;
				PathArc e = new PathArc(PathArc.Direction.UP, d.gov, d.rel);
				newPaths.add(seedPath.copyWithAppend(e));
			}
			for (Dep d : sent.gov2childs.get(endpoint)) {
				if (seedPath.containsNode(d.child)) continue;
				if (!isOKforVerbalPath(d)) continue;
				PathArc e = new PathArc(PathArc.Direction.DOWN, d.child, d.rel);
				newPaths.add(seedPath.copyWithAppend(e));
			}
		}
		return newPaths;
	}
	static List<SynPath> basePaths(Sentence sent, int startNode) {
		List<SynPath> newPaths = Lists.newArrayList();
		for (Dep d : sent.child2govs.get(startNode)) {
			if (!isOKforPath(d)) continue;
			SynPath p = new SynPath();
			p.startToken = startNode;
			p.pathArcs.add(new PathArc(PathArc.Direction.UP, d.gov, d.rel));
			newPaths.add(p);
		}
		for (Dep d : sent.gov2childs.get(startNode)) {
			if (!isOKforPath(d)) continue;
			SynPath p = new SynPath();
			p.startToken = startNode;
			p.pathArcs.add(new PathArc(PathArc.Direction.DOWN, d.child, d.rel));
			newPaths.add(p);
		}
		return newPaths;
	}
	
	public static SynPath findShortestPath(Sentence sent, int node1, int node2, int maxPathLength) {
		List<SynPath> paths = basePaths(sent, node1);
		for (int plen=1; plen <= maxPathLength; plen++) {
			for (SynPath p : paths) {
				if (p.endpointNode() == node2)
					return p;
			}
			paths = enlargenPaths(sent, paths);
		}
		return null;
	}
	
	
	public static boolean isNominal(Sentence sent, int t) {
		Token tok = sent.tokens.get(t);
		String rough = roughPOS(tok.posTag);
		return rough.equals("NOUN") 
			|| rough.equals("PRON")
//			|| !tok.nerTag.equals("O")
			;
	}

	public static boolean isVerbal(Sentence sent, int t) {
		String pos = sent.pos(t);
		return roughPOS(pos).equals("VERB");
	}


	public static void main(String args[]) throws JsonProcessingException, IOException {
		String line;
	
		while ( (line = BasicFileIO.stdin().readLine()) != null) {
			Sentence sent = Sentence.fromJSentLine(line);
			U.p(sent);
			int h = Syntax.findPhraseHead(sent, Integer.parseInt(args[0]), Integer.parseInt(args[1]));
			U.p("HEAD " + h);
		}

	}

	public static String roughPOS(String posTag) {
		String c = PTB_TO_COARSE.get(posTag);
		if (c==null) return posTag;
		return c;
//		String s= COARSE_TO_SHORT.get(c);
//		assert s != null;
//		return s;
	}
	public static String roughPOSclean(String posTag) {
		String c = PTB_TO_COARSE.get(posTag);
		if (c==null) c = posTag;
		if (c.equals(".")) c = "PUNCT";
		return c.toLowerCase();
	}
	
	/** http://universal-pos-tags.googlecode.com/svn/trunk/en-ptb.map **/
	public static ImmutableMap<String,String> PTB_TO_COARSE;
	/** This one I just made up, vaguely like the Twitter tags **/
	public static ImmutableMap<String,String> COARSE_TO_SHORT;
	
	static {
		
		COARSE_TO_SHORT = new ImmutableMap.Builder<String,String>()
			.put("ADJ","J")
			.put("ADP","P")
			.put("ADV","R")
			.put("CONJ","C")
			.put("DET","D")
			.put("NOUN","N")
			.put("NUM","NUM")
			.put("PRON","O")
			.put("PRT","T")
			.put("VERB","V")
			.put(".",".").put("X","X")
			.build();
		PTB_TO_COARSE = new ImmutableMap.Builder<String,String>()
			.put("!",".")
			.put("#",".")
			.put("$",".")
			.put("''",".")
			.put("(",".")
			.put(")",".")
			.put(",",".")
			.put("-LRB-",".")
			.put("-RRB-",".")
			.put(".",".")
			.put(":",".")
			.put("?",".")
			.put("``",".")
			.put("JJ","ADJ")
			.put("JJR","ADJ")
			.put("JJRJR","ADJ")
			.put("JJS","ADJ")
			.put("JJ|RB","ADJ")
			.put("JJ|VBG","ADJ")
			.put("IN","ADP")
			.put("IN|RP","ADP")
			.put("RB","ADV")
			.put("RBR","ADV")
			.put("RBS","ADV")
			.put("RB|RP","ADV")
			.put("RB|VBG","ADV")
			.put("WRB","ADV")
			.put("CC","CONJ")
			.put("DT","DET")
			.put("EX","DET")
			.put("PDT","DET")
			.put("WDT","DET")
			.put("NN","NOUN")
			.put("NNP","NOUN")
			.put("NNPS","NOUN")
			.put("NNS","NOUN")
			.put("NN|NNS","NOUN")
			.put("NN|SYM","NOUN")
			.put("NN|VBG","NOUN")
			.put("NP","NOUN")
			.put("CD","NUM")
			.put("PRP","PRON")
			.put("PRP$","PRON")
			.put("PRP|VBP","PRON")
			.put("WP","PRON")
			.put("WP$","PRON")
			.put("POS","PRT")
			.put("PRT","PRT")
			.put("RP","PRT")
			.put("TO","PRT")
			.put("MD","VERB")
			.put("VB","VERB")
			.put("VBD","VERB")
			.put("VBD|VBN","VERB")
			.put("VBG","VERB")
			.put("VBG|NN","VERB")
			.put("VBN","VERB")
			.put("VBP","VERB")
			.put("VBP|TO","VERB")
			.put("VBZ","VERB")
			.put("VP","VERB")
			.put("CD|RB","X")
			.put("FW","X")
			.put("LS","X")
			.put("RN","X")
			.put("SYM","X")
			.put("UH","X")
			.put("WH","X")
			.build();
	}


}
