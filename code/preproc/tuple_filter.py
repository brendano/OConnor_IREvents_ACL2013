import sys,re
import ujson as json
import keywordlist

def iter_sents():
    cur = []
    for line in sys.stdin:
        if line.startswith('==='):
            if cur: yield cur
            cur = []
        line = line.rstrip('\n')
        if line:
            cur.append(line)

def quickdirty_stems(w):
    lemmas = set()
    w = w.lower()
    lemmas.add(w)
    lemmas.add(re.sub(r's$','', w))
    lemmas.add(re.sub(r'es$','', w))
    # lemmas.add(re.sub(r'ed$','', w))
    # lemmas.add(re.sub(r'ing$','', w))
    return lemmas

for lines in iter_sents():
    tuplelines = [L for L in lines if L.startswith('TUPLE')]
    if not tuplelines: continue

    firstline = lines[0]
    parts = firstline.split('\t')
    docid = parts[0].split()[-1]
    senttext = parts[-2]
    jsent = json.loads(parts[-1])

    lemmas = [lemma for _surf,lemma,_pos,_ner in jsent['tokens']]

    print
    print firstline
    print

    for line in tuplelines:
        tuplestr = line.split('\t')[-1].strip()
        tpl = json.loads(tuplestr)

        sh = lemmas[tpl['src']['loc']['tokid']]
        th = lemmas[tpl['rec']['loc']['tokid']]
        heads = set([sh, th])
        if heads & keywordlist.HEADWORD_BLACKLIST:
            print "BADHEAD\t",heads, "\t",senttext,"\n"
            continue

        verbs = set()
        for x in tpl['pred']['path']:
            if x[0]=='W' and x[2]=='verb':
                verbs.add(x[1])
        if verbs & keywordlist.VERB_BLACKLIST:
            print "BADVERB\t", verbs, "\t", senttext,"\n"
            continue

        print line

