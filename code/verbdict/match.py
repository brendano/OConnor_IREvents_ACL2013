import sys,re,os,itertools,hashlib
import ujson as json
from collections import defaultdict
# sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..","post"))

# 'pattern' means TABARI pattern
# 'code' means a CAMEO code (we always keep them as strings, though they are numeric strings)
# 'key' means our parsed version of a TABARI pattern which is a space-separated
#  sequence of quasi-lemma thingies

keyed_patterns = None  # one->many
pattern_codes = None   # one->many
keyed_resolved_codes = None  # one->one

def pattern_filter(pat):
    # only use implicit simple patterns
    if 'TARGET' in pat or 'SOURCE' in pat: return False
    if 'BOTHST' in pat: return False
    if '{' in pat: return False
    if '*' in pat: return False   ## having this is a bug at this point?
    return True

def uniq_c(seq):
  ret = defaultdict(lambda:0)
  for x in seq:
    ret[x] += 1
  return dict(ret)

# Irregular forms that need hard-coding, from going through 100 most common
# first-in-pattern tokens (which are most all verbs.  down to count 20 from
# tabari cameo lexicon.)

verb_lemmatization_map = {
    "said":"say",
    "suspen":"suspend",
}

## Decided to not manually add on trailing e's.  Instead handle this at lookup time.
## Some examples of these would include...
# "announc":"announce",
# "urg":"urge",
# "pledg":"pledge",
# "welcom":"welcome",
# "denie":"deny",
# "refus":"refuse",
# "promis":"promise",
# "giv":"give",
# "prepar":"prepare",
# "issu":"issue",
# "demonstrat":"demonstrate",
# "arriv":"arrive",
# "reiterat":"reiterate",
# "voic":"voice",
# "resum":"resume",
# "continu":"continue",

def keyify_pattern(pat):
    # Yields keyed version of the pattern, in priority order (more and more normalization)
    # Basically this function has to do lemmatization of Tabari patterns.

    # use extra whitespace for regexes here then strip when returning

    pat = pat.replace("ENTITYSKIP"," ")
    pat = pat.replace("_", " ")
    # pat.replace(" to ", " ")
    pat = " " + pat + " "
    for k,v in verb_lemmatization_map.items():
        pat = pat.replace(" "+k+" ", " "+v+" ")
    def versions():
        yield pat
        no_ing = re.sub(r'ing ', " ", pat)
        yield no_ing
        no_ed = re.sub(r'ed ', " ", pat)
        yield no_ed
        yield re.sub(r'ing ', " ", no_ed)

    for p in versions():
        p = re.sub(r'\s+', " ", p).strip()
        yield p

def load_tabari_patterns():
    global pattern_codes, keyed_patterns, keyed_resolved_codes

    ## this data is messy so everything is a one->many map.
    ## Resolve ambiguities at the end (so can do ambiguity inspection for diagnosis)

    pattern_codes = defaultdict(set)

    for line in open(os.path.join(os.path.dirname(__file__), "cameo_verbs.tsv")):
        # ~/ptab/verbdict % head cameo_verbs.tsv | tsv2fmt
        # | ---  | abandon | abandon                         | ;oy 25 jul 2003  |
        # | 100  | abandon | said TARGET must abandon policy | ;ab 10 jul 2003  |
        # | 0874 | abandon | abandon headquarters            | ;ab 10 nov 2005  |

        code,headverb,pattern,comment = line.rstrip('\n').split('\t')

        # See TABARI manual (version 0.8.3b1) section 6.6, "Special Purpose Codes for Verbs"

        # 6.2, Null Code
        if code=='---': continue

        # 6.6.3, Paired Codes -- I think in the most common subj/obj interp,
        # the subj->obj direction gets the first code.  So let's do just that.
        code = code.split(":")[0]
        pattern_codes[pattern].add(code)

    print>>sys.stderr, "num TABARI patterns with a code", len(pattern_codes)
    pattern_codes = {p:cs for p,cs in pattern_codes.items() if pattern_filter(p)}
    print>>sys.stderr, "num TABARI patterns, after filter", len(pattern_codes)

    keyed_patterns = defaultdict(set)

    for p in pattern_codes:
        for k in keyify_pattern(p):
            keyed_patterns[k].add(p)

    for k in keyed_patterns.keys():
        ps = keyed_patterns[k]
        if len(ps)>1:
            # print>>sys.stderr, "conflicts", ps, [(p,pattern_codes[p]) for p in ps]
            pass
        keyed_patterns[k] |= ps

    keyed_patterns = dict(keyed_patterns)

    keyed_resolved_codes = {}

    for keystr,ps in keyed_patterns.items():
        codes = list(itertools.chain(*[pattern_codes[p] for p in ps]))
        codecounts = uniq_c(codes)
        resolved_code = None

        if len(codecounts)==1:
            ## easy resolution
            resolved_code = codes[0]
        else:
            # print>>sys.stderr, "conflict", repr(keystr), repr(codes)
            maxct = max(codecounts.values())
            most_common_codes = [code for code,ct in codecounts.items() if ct==maxct]
            # most of the time only one most-common. break ties (if any?) with md5
            most_common_codes.sort(key=lambda code: hashlib.md5(code).hexdigest())
            resolved_code = most_common_codes[0]

        keyed_resolved_codes[keystr] = resolved_code


def output_keyed_patterns():
    for k,ps in sorted(keyed_patterns.items()):
        out = [
                k, 
                keyed_resolved_codes[k],
                json.dumps([(p,list(pattern_codes[p])) for p in sorted(keyed_patterns[k])]),
            ]
        print '\t'.join(out)


########################


def lookuppath(path):
    ## examples
    # [["A","semagent","->"],["W","urge","verb"],["A","dobj","<-"]]
    # [["A","semagent","->"],["W","tell","verb"],["A","dobj","<-"]]
    # [["A","semagent","->"],["W","meet","verb"],["A","prep_with","<-"]]
    # [["A","semagent","->"],["W","criticize","verb"],["A","dobj","<-"]]
    
    modes = [ {'convert_xcomps':True}, {'convert_xcomps':False} ]
    for mode in modes:
        wpath = get_wordbased_path(path, **mode)

        def candidates():
            yield [w for w,pos in wpath]

            # backoffs to get stanford lemmas into tabari's stemming conventions.
            # tabari implements prefix matching
            # also remember this is after our preprocessing of tabari patterns further up, so could be weird.
            yield [re.sub(r'[aeiou]+$', "", w) if pos=="verb" else w  for w,pos in wpath]
            # tabari "tried" => "tri" then match lemma "try" .. 10 out of 2082 yield
            yield [re.sub(r'y$', "i", w) if pos=="verb" else w  for w,pos in wpath]

        for cand in candidates():
            x = ' '.join(cand)
            if x in keyed_resolved_codes:
                # print "MATCH", repr(x), repr(cand), json.dumps(path), json.dumps(wpath), repr(keyed_patterns[x])
                return keyed_resolved_codes[x], keyed_patterns[x]
    return None

def get_wordbased_path(path, convert_xcomps=True):
    # [["A","semagent","->"],["W","meet","verb"],["A","prep_with","<-"]]
    # to list of (word,pos) pairs
    # [(meet,verb),(with,prep)]

    wpath = []
    # skip first item in the path, since it's always the boring "semagent" arc.
    for i in range(1,len(path)):
        node = path[i]

        if node[0]=='W':  ## word node
            wpath.append( (path[i][1], path[i][2]) )
        elif node[0]=='A':  ## arc descriptor
            deprel = node[1]
            # only use prepositions (uncollapsing), all else ignore.
            if deprel.startswith("prep"):
                word = deprel.replace("prep_", "").replace("prepc_","")
                wpath.append((word,"prep"))
            elif deprel=="xcomp" and convert_xcomps:
                wpath.append(("to","*inf*"))
    return wpath


#####

load_tabari_patterns()
# output_keyed_patterns()
# sys.exit()

seen_patterns = set()
for line in sys.stdin:
    pathstr = line.strip()
    path = json.loads(pathstr)
    x = lookuppath(path)
    if x:
        code,patterns = x
        for p in patterns:
            seen_patterns.add(p)
        print '\t'.join([code, json.dumps(list(patterns)), pathstr])

# for pat in pattern_codes:
#     if pat not in seen_patterns:
#         print '\t'.join(["UNSEEN", json.dumps(list(pattern_codes[pat])), pat])

