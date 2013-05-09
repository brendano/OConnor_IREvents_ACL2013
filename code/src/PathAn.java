import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Set;

import myprolog.Prolog;
import myprolog.Prolog.Compound;
import myprolog.Prolog.PrologException;
import myprolog.Prolog.Term;
import myprolog.Prolog.Variable;

import org.antlr.runtime.RecognitionException;
import org.codehaus.jackson.JsonNode;

import struct.Dep;
import struct.Mention;
import struct.Sentence;
import struct.Token;
import util.BasicFileIO;
import util.JsonUtil;
import util.U;
import a.Syntax;
import a.Syntax.PathArc;

import com.google.common.base.Function;
import com.google.common.base.Predicate;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import com.google.common.collect.Ordering;
import com.google.common.collect.Sets;
import com.google.common.primitives.Ints;

import edu.stanford.nlp.util.Pair;
import edu.stanford.nlp.util.StringUtils;

public class PathAn {
	ActorList actorList;

	public PathAn() throws IOException {
		actorList = new ActorList();
		// actorList.readActorList("/Users/brendano/desktop/tabari/dict/Levant.080629.BTOHACK.actors");
		// actorList.readTabariActorList("dict/Levant.080629.BTOHACK.actors");
		// actorList.readSimpleActorList("dict/countrylist.txt");
		actorList.readSimpleActorList("dict/country_igos.txt");
	}

	/** store results into the Sentence **/
	public void annotateSentence(Sentence sent) {
		List<ActorList.NameMatch> matches = actorList.findMatches(sent
				.stringTokens());
		Set<Integer> seenMentionHeads = Sets.newHashSet();
		for (ActorList.NameMatch match : matches) {
			if (match.code.equals("---"))
				continue;

			Mention ment = new Mention();
			int head = Syntax.findPhraseHead(sent, match.startToken,
					match.endToken);
			if (seenMentionHeads.contains(head)) {
				continue;
			}
			seenMentionHeads.add(head);
			ment.origHeadToken = head;
			ment.startToken = match.startToken;
			ment.endToken = match.endToken;
			ment.actorCode = match.code;

			// This gets amod and nn resolutions, e.g.
			// Darfur/NNP nn> rebels
			// American/JJ amod> diplomats

			if (sent.hasParse) {
				int newHead = Syntax.findMaximalNominalHead(sent, head);
				if (newHead != head) {
					// U.pf("MOVE %s -> %s\n", head, newHead);
				}
				head = newHead;
			}
			ment.headToken = head;
			sent.mentions.add(ment);
			// sent.termDB.add(Prolog.P("actor",
			// Prolog.P(sent.nicename(head))));
		}
	}

	public void justNER() throws IOException {
		for (String line : BasicFileIO.STDIN_LINES) {
			Sentence sent = Sentence.fromJSentLine(line);
			if (sent == null)
				continue;
			annotateSentence(sent);
			U.pf("=== %d matches\t%s\t%s\t%s\n", sent.mentions.size(),
					sent.docidStr, sent.sentidStr, sent.spacesepText());
			for (Mention m : sent.mentions) {
				U.pf("\tMENTION %s %s:%d", m.actorCode,
						m.headToken == -1 ? "NONE" : sent.lemma(m.headToken),
						m.headToken);
				if (m.origHeadToken != m.headToken) {
					U.pf("\tORIG %s:%d", sent.lemma(m.origHeadToken),
							m.origHeadToken);
				}
				U.pf("\n");
			}
		}
	}

	public Predicate<Sentence> sentenceProcessor() {
		return new Predicate<Sentence>() {
			@Override
			public boolean apply(Sentence sent) {
				if (sent == null)
					return false;
				annotateSentence(sent);

				// if (sent.mentions.size() < 2) return false;

				for (Mention m : sent.mentions) {
					if (m.actorCode.equals("###")) {
						assert false : "bad code";
						return false;
					}
				}

				Collections.sort(sent.mentions, new Ordering<Mention>() {
					public int compare(Mention m1, Mention m2) {
						return Ints.compare(m1.headToken, m2.headToken);
					}
				});
				// for (int i1=0; i1 < sent.mentions.size()-1; i1++) {
				// for (int i2=i1+1; i2 < sent.mentions.size(); i2++) {
				// int m1 = sent.mentions.get(i1).headToken;
				// int m2 = sent.mentions.get(i2).headToken;
				// sent.termDB.add(P("apair", P(sent.nicename(m1)),
				// P(sent.nicename(m2))));
				// sent.termDB.add(P("apair", P(sent.nicename(m2)),
				// P(sent.nicename(m1))));
				// }
				// }
				return true;
			}
		};
	}

	public Iterable<Sentence> sentenceIter() {
		Iterable<Sentence> sents = Iterables.transform(BasicFileIO.STDIN_LINES,
				new Function<String, Sentence>() {
					@Override
					public Sentence apply(String line) {
						return Sentence.fromJSentLine(line);
					}
				});

		sents = Iterables.filter(sents, sentenceProcessor());

		return sents;
	}

	public void dumpPaths() throws IOException {
		for (final Sentence sent : sentenceIter()) {
			U.pf("\n=== %s\t%s\t%s\t%s\n", 
					sent.docidStr,
					sent.sentidStr,
					StringUtils.join(sent.stringTokens()),
					sent.jsentStr);
			U.p("");
			for (int i1 = 0; i1 < sent.mentions.size() - 1; i1++) {
				for (int i2 = i1 + 1; i2 < sent.mentions.size(); i2++) {
					Mention m1 = sent.mentions.get(i1);
					Mention m2 = sent.mentions.get(i2);
					int t1 = m1.headToken;
					int t2 = m2.headToken;
					if (t1 == t2)
						continue;
					if (t1 == -1 || t2 == -1)
						continue;
					assert t1 < t2;
					Syntax.SynPath path = Syntax.findShortestPath(sent, t1, t2,
							8);

					// U.pf("PAIR\t%s(%s)\t%s(%s)\t", m1.actorCode,
					// sent.word(m1.headToken), m2.actorCode,
					// sent.word(m2.headToken));
					U.pf("PAIR\t%s %s:%d\t%s %s:%d\t", m1.actorCode,
							sent.lemma(m1.headToken), m1.headToken,
							m2.actorCode, sent.lemma(m2.headToken),
							m2.headToken);
					if (path == null) {
						U.pf("nopath\n");
						continue;
					}
					boolean hasVerb = Iterables.any(path.pathArcs,
							new Predicate<Syntax.PathArc>() {
								public boolean apply(Syntax.PathArc p) {
									return Syntax
											.isVerbal(sent, p.endpointNode);
								}
							});
					String pathSig = JsonUtil.toJson(pathInfo(sent, path).second).toString();
					U.pf("%s\t%s", hasVerb ? "hasverb" : "noverb", pathSig);
					U.pf("\n");

					// verbFun(sent, path);
				}
			}
			U.p("");
			U.p(sent);
			sent.tree.pennPrint();
		}
	}

	public void stricterPaths() throws IOException {
		for (final Sentence sent : sentenceIter()) {
			List<EventTuple> tuples = getStrictPaths(sent);

			if (sent.mentions.size() == 0)
				continue;

			U.pf("\n=== %s\t%s\t%s\t%s\n", sent.docidStr, sent.sentidStr,
					StringUtils.join(sent.stringTokens()), sent.jsentStr);
			U.p("");
			for (Mention m : sent.mentions) {
				U.pf("MENTION\t%s %s:%d\n", m.actorCode,
						sent.lemma(m.headToken), m.headToken);
			}
			U.p("");
			
			if (tuples.size() == 0)
				continue;
			
			for (int tuplenum=0; tuplenum < tuples.size(); tuplenum++) {
				EventTuple tpl = tuples.get(tuplenum);
				boolean c = checkForGoverningModalities(sent, tpl.path);
				if (c) {
					continue;
				}
				
				Pair<List<Integer>, List<JsonNode>> pathinfo = pathInfo(sent, tpl.path);
				
				Map<String,Object> allinfo = map()
						.put("tupleid", new int[]{ sent.sentid, tuplenum })
						.put("pred", 
								map()
								.put("loc", map()
										.put("sentid", sent.sentid)
										.put("tokids", pathinfo.first)
										.build())
								.put("path", pathinfo.second)
								.build())
						.put("src", 
								map()
								.put("loc", map().put("sentid", sent.sentid).put("tokid", tpl.m1.headToken)
											.build())
								.put("entityid", tpl.m1.actorCode)
								.build())
						.put("rec", 
								map()
								.put("loc", map().put("sentid", sent.sentid).put("tokid", tpl.m2.headToken)
											.build())
								.put("entityid", tpl.m2.actorCode)
								.build())
						.build();

				U.p("TUPLE\t" + JsonUtil.toJson(allinfo).toString());

//				U.pf("PAIR\t%s %s:%d\t%s %s:%d\t%s\n", tpl.m1.actorCode,
//						sent.lemma(tpl.m1.headToken), tpl.m1.headToken,
//						tpl.m2.actorCode, sent.lemma(tpl.m2.headToken),
//						tpl.m2.headToken, pathSignature(sent, tpl.path));
			}
		}
	}
	static ImmutableMap.Builder<String,Object> map() {
		return new ImmutableMap.Builder<String,Object>();
	}

	/** return true if it has a governing off-path potential modality. **/
	public boolean checkForGoverningModalities(Sentence sent,
			Syntax.SynPath path) {
		for (Syntax.PathArc pe : path.pathArcs) {
			int t = pe.endpointNode;
			for (Dep d : sent.child2govs.get(t)) {
				boolean offpath = !path.containsNode(d.gov);
				if (!offpath)
					continue;
				boolean govIsVerb = Syntax.roughPOS(sent.pos(d.gov)).equals(
						"VERB");
				if (!govIsVerb)
					continue;
				boolean potentiallyModalRelation = !(d.rel.startsWith("conj"));
				if (!potentiallyModalRelation)
					continue;
				// U.pf("GOV\t%s:%s:%d -%s-> %s:%s:%d\n",
				// sent.word(t), sent.pos(t), t,
				// d.rel,
				// sent.word(d.gov), sent.pos(d.gov), d.gov);
				return true;
			}
		}
		return false;
	}

	public boolean isAgentRelation(String deprelname) {
		return deprelname.equals("nsubj") || deprelname.equals("agent");
	}

	static class EventTuple {
		Mention m1;
		Mention m2;
		Syntax.SynPath path;
	}

	public List<EventTuple> getStrictPaths(Sentence sent) {
		List<EventTuple> tuples = Lists.newArrayList();

		for (int i1 = 0; i1 < sent.mentions.size() - 1; i1++) {
			for (int i2 = i1 + 1; i2 < sent.mentions.size(); i2++) {
				Mention m1 = sent.mentions.get(i1);
				Mention m2 = sent.mentions.get(i2);
				int t1 = m1.headToken;
				int t2 = m2.headToken;
				if (t1 == t2)
					continue;
				if (t1 == -1 || t2 == -1)
					continue;
				assert t1 < t2;
				Syntax.SynPath path = Syntax.findShortestPath(sent, t1, t2, 4);
				if (path == null) {
					continue;
				}
				if (!isAgentRelation(path.pathArcs.get(0).deprel)) {
					// false negative, we want a semrel ask(spain, britain):
					// spain is to ask britain to extradite pinochet
					// spain ... ask <xcomp extradite
					// britain nsubj> extradite <dobj pinochet
					continue;
				}
				if (isAgentRelation(path.pathArcs
						.get(path.pathArcs.size() - 1).deprel)) {
					continue;
				}
				
				// path normalization
				for (PathArc p : path.pathArcs) {
					if (isAgentRelation(p.deprel)) {
						p.deprel = "semagent";
					}
				}

				EventTuple t = new EventTuple();
				t.m1 = m1;
				t.m2 = m2;
				t.path = path;
				tuples.add(t);
			}
		}
		return tuples;
	}

	public static void verbFun(Sentence sent, Syntax.SynPath pat) {
		for (Syntax.PathArc p : pat.pathEdgesExceptLast()) {
			if (Syntax.isVerbal(sent, p.endpointNode)) {
				U.pf("V %s:%d\n", sent.word(p.endpointNode), p.endpointNode);
				for (Dep d : sent.sorted_gov2childs(p.endpointNode)) {
					U.pf("  <-%s %s:%d\n", d.rel, sent.word(d.child), d.child);
				}
			}
		}
	}

	static Compound P(String p) {
		return Prolog.P(p);
	}

	static Compound P(String p, String... a) {
		return Prolog.P(p, a);
	}

	static Compound P(String p, Term... a) {
		return Prolog.P(p, a);
	}

	static Variable V(String v) {
		return Prolog.V(v);
	}

	public static Pair<List<Integer>, List<JsonNode>> pathInfo(Sentence sent,
			Syntax.SynPath path) {
		List<JsonNode> elts = new ArrayList<>();
		List<Integer> tokids = new ArrayList<>();

		int plen = path.pathArcs.size();
		for (int i = 0; i < plen; i++) {
			Syntax.PathArc p = path.pathArcs.get(i);
			Token tok = sent.tokens.get(p.endpointNode);
			elts.add(JsonUtil.toJson(new String[]{"A", p.deprel, p.direction.toString()} ));
			if (i < plen - 1) {
				elts.add(JsonUtil.toJson(new String[]{ "W", tok.lemma, Syntax.roughPOSclean(tok.posTag) }));
				tokids.add(p.endpointNode);
			}
		}
		return U.pair(tokids, elts);
	}

	public void verbAnalysis() throws RecognitionException, PrologException {
		for (final Sentence sent : sentenceIter()) {
			U.pf("\n=== %s\t%s\t%s\n", sent.docidStr, sent.sentidStr,
					StringUtils.join(sent.stringTokens()));
			// sent.populateTermDB(null);
			// List<Compound> q = PrologParser.parseTermString(
			// "apair(A,B), nsubj(V,A), rel(R,V,B)");
			// List<Solution> results =
			// Lists.newArrayList(sent.termDB.search(q));
			// if (results.size()==0) { continue; }
			// for (Solution sol : results) {
			// if (sol.get("A").equals(sol.get("B"))) continue;
			// U.pf("%s %s || %s || %s \n", sol.get("A"), sol.get("B"),
			// sol.get("V"), sol.get("R"));
			// }

			for (int i1 = 0; i1 < sent.mentions.size() - 1; i1++) {
				for (int i2 = i1 + 1; i2 < sent.mentions.size(); i2++) {
					Mention m1 = sent.mentions.get(i1), m2 = sent.mentions
							.get(i2);
					int t1 = m1.headToken, t2 = m2.headToken;
					if (t1 == t2)
						continue;
					Syntax.SynPath path = Syntax.findShortestPath(sent, t1, t2,
							8);
					U.pf("%s ||| %s\n", m1, m2);
					U.p(path);
				}
			}

		}
	}

	public static void main(String[] args) throws Exception {
		PathAn runner = new PathAn();
		// runner.verbAnalysis();
		// runner.filterMultiMatches();
		// runner.justNER();
		// runner.dumpPaths();

		runner.stricterPaths();
	}

}
