package struct;
import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Set;

import myprolog.Prolog;
import myprolog.TermDB;

import org.codehaus.jackson.JsonNode;
import org.codehaus.jackson.JsonProcessingException;

import util.BasicFileIO;
import util.JsonUtil;
import util.U;
import a.Syntax;

import com.google.common.collect.HashMultimap;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableMultimap;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Multimap;
import com.google.common.collect.Ordering;
import com.google.common.primitives.Ints;

import edu.stanford.nlp.trees.LabeledScoredTreeFactory;
import edu.stanford.nlp.trees.PennTreeReader;
import edu.stanford.nlp.trees.Tree;
import edu.stanford.nlp.trees.TreeFactory;
import edu.stanford.nlp.trees.TreeReader;
import edu.stanford.nlp.util.Pair;
import edu.stanford.nlp.util.StringUtils;

/**
 * Our sentence representation, with jsent readers, and dependency navigation accessors
 */
public class Sentence {
	public String docidStr;
	public String sentidStr;
	public int sentid;
	public String jsentStr;
	public List<Token> tokens;
	public List<Dep> dependencies;
	public List<Mention> mentions;
	public Tree tree;
	public boolean hasParse = false;
	
	// dependency indexes
	public Multimap<Integer,Dep> child2govs;
	public Multimap<Integer,Dep> gov2childs;
	private Map<Pair<Integer,Integer>, String> childgov2rel;
	public TermDB termDB;
	
	public int T() { return tokens.size(); }
	
	public Sentence() {
		tokens = Lists.newArrayList();
		dependencies = Lists.newArrayList();
		mentions = Lists.newArrayList();
		termDB = new TermDB();
	}
	
	public List<String> stringTokens() {
		List<String> toks = Lists.newArrayList();
		for (int t=0; t < T(); t++)
			toks.add(tokens.get(t).word);
		return toks;
	}
	public String spacesepText() {
		return StringUtils.join(stringTokens());
	}

	// these are more convenient. maybe we should just go back to parallel arrays
	public String pos(int t) { return tokens.get(t).posTag; }
	public String lemma(int t) {return tokens.get(t).lemma; }
	public String word(int t) {return tokens.get(t).word; }
	public String ner(int t)  {return tokens.get(t).nerTag; }
	
	public void indexDependencies() {
		child2govs = HashMultimap.create();
		gov2childs = HashMultimap.create();
		childgov2rel = Maps.newHashMap();
		
		for (Dep dep : dependencies) {
			child2govs.put(dep.child, dep);
			gov2childs.put(dep.gov, dep);
			childgov2rel.put(U.pair(dep.child, dep.gov), dep.rel);
		}
		child2govs = ImmutableMultimap.copyOf(child2govs);
		gov2childs = ImmutableMultimap.copyOf(gov2childs);
		childgov2rel = ImmutableMap.copyOf(childgov2rel);
	}
	public void populateTermDB(Set<String> relWhitelist) {
		for (Dep dep : dependencies) {
			if (relWhitelist==null || relWhitelist.contains(dep.rel)) {
				termDB.add(Prolog.P(
					dep.rel, 
					Prolog.P(nicename(dep.gov)),
					Prolog.P(nicename(dep.child))));
				termDB.add(Prolog.P(
						"rel",
						Prolog.P(dep.rel),
						Prolog.P(nicename(dep.gov)),
						Prolog.P(nicename(dep.child))));
			}
		}
//		try {
			if (relWhitelist==null || relWhitelist.contains("lemma"))
				for (int t=0; t < T(); t++)
					termDB.add(Prolog.P("lemma",
							Prolog.P(nicename(t)), Prolog.P(lemma(t)) ));
			if (relWhitelist==null || relWhitelist.contains("pos"))
				for (int t=0; t < T(); t++)
					termDB.add(Prolog.P("pos",
							Prolog.P(nicename(t)), Prolog.P(pos(t).toLowerCase()) ));
			if (relWhitelist==null || relWhitelist.contains("rpos"))
				for (int t=0; t < T(); t++) {
					termDB.add(Prolog.P("rpos",
						Prolog.P(nicename(t)), Prolog.P(Syntax.roughPOSclean(pos(t))) ));
				}
//		}
//		catch (PrologException e) { e.printStackTrace(); }
	}
	public String nicename(int t) {
		return word(t)+":"+t;
//		return word(t)+"/"+pos(t)+":"+t;
//		return word(t)+"/"+pos(t)+"/"+ner(t)+":"+t;
	}
	public String deprel(int child, int gov) {
		String r = childgov2rel.get(U.pair(child,gov));
		return r;
	}
	public boolean hasdep(int child, int gov) {
		return childgov2rel.containsKey(U.pair(child,gov));
	}
	
	public void readDepsStr(String depsStr) throws JsonProcessingException, IOException {
		hasParse = false;
		for (JsonNode depTriple : JsonUtil.readJson(depsStr)) {
			Dep dep = new Dep();
			dep.rel   = depTriple.get(0).asText();
			dep.child = depTriple.get(1).asInt();
			dep.gov   = depTriple.get(2).asInt();
			dependencies.add(dep);
			if (!dep.rel.equals("dep")) hasParse = true;
		}
		indexDependencies();
	}
	public void initializeTokens(String[] surfaceWords) {		
		for (int t=0; t < surfaceWords.length; t++) {
			Token tok = new Token();
			tok.word = surfaceWords[t];
			tokens.add(tok);
		}
	}
	
	public static Sentence fromJSentLine(String line) {
		String parts[] = line.split("\t");
		if (!parts[1].startsWith("S")) return null;
		Sentence sent = new Sentence();
		sent.docidStr = parts[0];
		sent.sentidStr = parts[1];
		sent.sentid =Integer.valueOf(parts[1].replace("S",""));
		sent.jsentStr = parts[parts.length-1];
		JsonNode jsent = JsonUtil.parse(sent.jsentStr);
		JsonNode jTokens = jsent.get("tokens");
		for (int t=0; t < jTokens.size(); t++) {
			Token tok = new Token();
			tok.word = jTokens.get(t).get(0).asText();
			tok.lemma = jTokens.get(t).get(1).asText();
			tok.posTag = jTokens.get(t).get(2).asText();
			tok.nerTag = jTokens.get(t).get(3).asText();
			sent.tokens.add(tok);
		}
		JsonNode jDeps = jsent.get("deps");
		boolean allX = true;
		for (JsonNode depTriple : jDeps) {
			Dep dep = new Dep();
			dep.rel   = depTriple.get(0).asText();
			dep.child = depTriple.get(1).asInt();
			dep.gov   = depTriple.get(2).asInt();
			if ( ! dep.rel.equals("X")) allX = false;
			sent.dependencies.add(dep);
		}
		sent.hasParse = !allX;
		sent.indexDependencies();
		return sent;
	}

	public String cleanNER(String tag) {
		return tag.replace("PERSON", "PER").replace("ORGANIZATION","ORG").replace("LOCATION","LOC");
	}
	
	public Collection<Dep> sorted_gov2childs(int gov) {
		Ordering<Dep> o = new Ordering<Dep>() {
			@Override
			public int compare(Dep arg0, Dep arg1) {
				return Ints.compare(arg0.child, arg1.child);
			}
		};
		return o.sortedCopy(gov2childs.get(gov));
	}
	
	public String govCentricView() {
		StringBuilder sb = new StringBuilder();
		for (int gov=0; gov < T(); gov++) {
//			String s = String.format("%3d %-15s  ", gov, tokens.get(gov).word);
			Token tok = tokens.get(gov);
			String s = String.format("%3d %-15s %-4s %-4s ", gov, tok.word, tok.posTag, cleanNER(tok.nerTag));
			if (tok.ssTag != null) {
				s += U.sf("%-18s ", tok.ssTag);
			}
			sb.append(s);
			
			List<String> strs = new ArrayList<String>();
			for (Dep dep : sorted_gov2childs(gov)) {
				strs.add(String.format("<-%s %s", dep.rel, wt(dep.child)));
			}
			if (strs.size() > 0) {
				sb.append(strs.get(0));
			}
			sb.append("\n");
			for (int c=1; c < strs.size(); c++) {
				sb.append(StringUtils.pad("", s.length()));
				sb.append(strs.get(c));
				sb.append("\n");
			}
		}
		return sb.toString();
	}
	private String wt(int t) {
		return String.format("%s:%d", tokens.get(t).word, t);
	}
	
	public String toString() {
		String s = "";
//		s += naturalOrderView();
//		s += "\n";
		s += govCentricView();
		for (Mention m : mentions) { 
			s += U.sf("MENT\t%s\n", m);
		}
		return s;
	}
//	

	public static TreeFactory tree_factory;
	static { tree_factory = new LabeledScoredTreeFactory(); }

	public static Tree readTreeFromString(String parseStr) {
		//read in the input into a Tree data structure
		TreeReader treeReader = new PennTreeReader(new StringReader(parseStr), tree_factory);
		Tree inputTree = null;
		try{
			inputTree = treeReader.readTree();
			
		}catch(IOException e){
			e.printStackTrace();
		}
		return inputTree;
	}

	public static void main(String args[]) throws RuntimeException, Exception {
		String line;
		while ( (line = BasicFileIO.stdin().readLine()) != null) {
			String parts[] = line.split("\t");
			if (!parts[1].startsWith("S")) continue;
			U.p("=== " + parts[0] + " " + parts[1]);
			Sentence sent = Sentence.fromJSentLine(line);
			U.p(sent);
		}
	}

}
