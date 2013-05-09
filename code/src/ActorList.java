import java.io.BufferedReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import struct.Sentence;
import util.BasicFileIO;
import util.U;
import a.Syntax;

import com.google.common.collect.HashMultimap;
import com.google.common.collect.Lists;
import com.google.common.collect.Multimap;
import com.google.common.primitives.Ints;

import edu.stanford.nlp.util.StringUtils;


public class ActorList {
	HashSet<String> actorNames;
	HashMap<String,Pattern> code2pattern;
	Multimap<String, String> actorCodes;
	Multimap<String, String> code2actors;
	Pattern bigDisjunction;
	
	
	ActorList() {
		actorNames = new HashSet<String>();
		actorCodes = HashMultimap.create();
		code2actors = HashMultimap.create();
		code2pattern = new HashMap<String,Pattern>();
		
	}
	
	static Pattern codePat = Pattern.compile("\\[(.*?)\\]");
	
	
	public void readSimpleActorList(String filename) {
		/*
SEY     SEICHELES
SEY     SECHELLES
=== cowalpha=SIE countrycode=SLE
SIE     SIERRA_LEONE
SIE     SIERRA_LEONEAN
SIE     SIERRA_LEOME
SIE     SIERRA_LEON
		*/
		

		for (String line : BasicFileIO.openFileLines(filename)) {
			if (line.startsWith("===") || line.startsWith("\\s*#")) continue;
			String[] parts = line.split("\t");
			assert parts.length==2;
			String countryCode = parts[0].replaceAll("^[^A-Z]*", "").replaceAll("[^A-Z]*$","");
			String namePattern = parts[1].replaceAll("_*$","");
			actorNames.add(namePattern);
			actorCodes.put(namePattern, countryCode);
			code2actors.put(countryCode, namePattern);
		}
		createPatterns();
	}
	public void readTabariActorList(String filename) throws IOException {
		BufferedReader r = BasicFileIO.openFileOrResource(filename);
		String line;
		String mode = "NORMAL";
		while ( (line = r.readLine()) != null) {
			if (line.startsWith("~~FINISH")) break;
			if (line.equals("<NOUN>")) {
				mode = "NOUN";
			} else if (line.equals("</NOUN>")) {
				continue;
			} else if (line.equals("<ADJECTIVE>")) {
				mode = "ADJ";
			} else if (line.equals("</ADJECTIVE>")) {
				mode = "NORMAL";
			}
//			if (line.matches(".*\\</?(NOUN|ADJECTIVE)\\>.*")) continue;
			
			if (mode.equals("NOUN") || mode.equals("ADJ")) {
				continue;
			}
			String actorName;
			List<String> codes = new ArrayList<String>();
			
			line = line.replaceAll(";.*", "");
			actorName = line.replaceFirst("\\[.*", "").trim();

			Matcher m = codePat.matcher(line);
			while (m.find()) {
				String actorCode = m.group(1); // TODO more parsing in here
				actorCode = actorCode.split("\\s")[0];
				codes.add(actorCode);
			}
//			U.p(line + "   ==>   " + actorName);
			if (actorName.trim().equals("")) {
				// how does this happen?
				continue;
			}
			actorNames.add(actorName);
			if (codes.size()==0) codes.add("NO_CODE");
			for (String code : codes) {
				actorCodes.put(actorName, code);
				code2actors.put(code, actorName);
			}
		}
		
//		for (String n : actorNames) {
//			U.pf("%-60s ||| ", n);
//			for (String c : actorCodes.get(n)) {
//				U.pf("%s | ", c);
//			}
//			U.pf("\n");
//		}
		
		createPatterns();
	}
	
	public void createPatterns() {
		for (String code : code2actors.keySet()) {
//			U.p("CODE " + code);
			code2pattern.put(code, makeDisjunction(code2actors.get(code)));
		}
//		for (String a : actorCodes.keySet()) { U.p("ACTOR ||| " + a); }
		bigDisjunction = makeDisjunction(actorCodes.keySet());
	}
	
	static String regexFromName(String name) {
		// use tabari conventions for name pattern
		// assume input is space-separated tokenization
		String s = name;
		assert ! s.contains("=");
//		if (s.contains("{") || s.contains("}")) return null;
		s = s.replaceAll("\\{ *", "(");
		s = s.replaceAll(" *\\}", ")");
		s = s.replaceAll(" *\\| *", "|");
		s = s.replaceAll(" +", "=");
		
		// disable suffix matching ... not sure how this works ...
//		String suffix = "[a-z]*";
//		s = s.replaceAll("([A-Z])=", "$1" +suffix+"=");
//		s = s.replaceAll("([A-Z])$", "$1" + suffix);
		
		s = s.replaceAll("\\.", "\\\\.");
		s = s.replaceAll("'S", " ?'S");  //allow 's as its own token, though pretokenized text always has it so
		s = s.replaceAll("'_", "'?_");   //i'm a bit mystified strict matching matters here, for trailing apostophe
		s = s.replaceAll("_"," ");
		s = s.replaceAll("="," ");
//		U.p(s);
		Pattern.compile(s, Pattern.CASE_INSENSITIVE);
		return s;
	}
	
	public Pattern makeDisjunction(Collection<String> names) {
		List<String> pats = new ArrayList<String>();
		for (String actorName : names) {
			String pat = regexFromName(actorName);
			if (pat == null) {
				continue;
			}
			pats.add(pat);
		}
		
		// Longest-first in the disjunction forces the regex matcher to prioritize longer matches over shorter ones
		Collections.sort(pats, new Comparator() {
			@Override
			public int compare(Object o1, Object o2) {
				String s1 = (String) o1;
				String s2 = (String) o2;
				return -Ints.compare(s1.length(), s2.length());
			}	
		});
		String regex = StringUtils.join(pats, "|");
		regex = "(" + regex + ")";
//		regex = "\\b" + regex + "\\b";
		regex = "(?: |^)" + regex + "(?: |$)";
//		U.p(regex);
		return Pattern.compile(regex, Pattern.CASE_INSENSITIVE);
	}
	
	
	class NameMatch {
		public String matchText, code;
		public int startChar=-1, endChar=-1, startToken=-1, endToken=-1;
		NameMatch(String matchText, String code) { 
			this.matchText = matchText;
			this.code = code;
		}
	}
	
	public List<NameMatch> findNameMatchesFromSpacesep(String spacesepText) {
		// ugh this sucks and is slow.  really wish i had an FST instead
		List<NameMatch> matches = Lists.newArrayList();
		
		Matcher m = bigDisjunction.matcher(spacesepText);
		while (m.find()) {
			String matchtext = m.group(1);
			for (String code : code2pattern.keySet()) {
				Matcher m2 = code2pattern.get(code).matcher(matchtext);
				while (m2.find()) {
					if (m2.group().equals(matchtext)) {
						// need complete match here
						NameMatch nm = new NameMatch(m.group(1), code);
						nm.startChar = m.start(1);
						nm.endChar = m.end(1);
						matches.add(nm);
					}
				}
			}
		}
		return matches;
	}

	/** These should be surface tokens **/
	public List<NameMatch> findMatches(List<String> tokens) {
		String spacesep = StringUtils.join(tokens, " ");
		int[] char2tokid = new int[spacesep.length()+1];
		int c=0;
		for (int t=0; t < tokens.size(); t++) {
			for (int i=0; i < tokens.get(t).length(); i++) {
				char2tokid[c+i] = t;
			}
			char2tokid[c+tokens.get(t).length()] = t+1;
			c += tokens.get(t).length() + 1;
		}
		
		List<NameMatch> matches = findNameMatchesFromSpacesep(spacesep);
		for (NameMatch m : matches) {
			m.startToken = char2tokid[m.startChar];
			m.endToken   = char2tokid[m.endChar];
		}
		return matches;
	}
	
//	public void scanText(String text) {
//		InputStream is = new ByteArrayInputStream(text.getBytes());
//		BufferedReader br = new BufferedReader(new InputStreamReader(is));
//
//		DocumentPreprocessor dp = new DocumentPreprocessor(br);
//		for (List sentence : dp) {
//			U.p(sentence);
//		}
//	}
	
	public static void main(String[] args) throws IOException {
		ActorList a = new ActorList();
		a.readTabariActorList(args[0]);
		
		String line;
		while ( (line = BasicFileIO.stdin().readLine()) != null) {
			String parts[] = line.split("\t");
			U.p("=== " + parts[0] + " " + parts[1]);
			String[] tokens = parts[2].split(" ");
			
			List<NameMatch> matches = a.findMatches(Arrays.asList(tokens));
			
			U.pf("%s\n", parts[2]);
//			for (int i=0; i<parts[2].length(); i++) U.pf("%d", i % 10);
//			U.pf("\n");
			
			Sentence sent = Sentence.fromJSentLine(line);
			U.p(sent);
			for (NameMatch m : matches) {
				U.pf("%s %s %s:%s\n", m.code, m.matchText, m.startToken, m.endToken);
				int h = Syntax.findPhraseHead(sent, m.startToken, m.endToken);
				U.pf("  HEAD %s\n", h);
			}
			
			
		}
		
	}

}
