# This version is intended to work with the new "doc corejson" format.
# One line per document.
# One of the columns has raw sentence texts, to facilitate fast shallow analysis
# like text classification.

import sys,csv,os,re
import numpy as np
import ujson as json
import keywordlist

class Model:
    def __init__(self):
        self.klasses = None
        self.coefs = {}
    def load_model(self, coef_file):
        all_klasses = set()
        for row in csv.DictReader(open(coef_file)):
            word = row['']
            keys = row.keys()
            klasses = [k for k in keys if k.strip()]
            all_klasses |= set(klasses)
            self.coefs[word] = {k:float(row[k]) for k in klasses}
        self.klasses = list(sorted(all_klasses))

    def calc_probs(self, text):
        toks = tokenize_for_classifier(text)
        scores = [self.coefs["(Intercept)"][k] for k in self.klasses]
        for tok in toks:
            if tok in self.coefs:
                for knum,klassname in enumerate(self.klasses):
                    scores[knum] += self.coefs[tok][klassname]
        probs = softmax(np.array(scores))
        return {klassname:probs[knum] for knum,klassname in enumerate(self.klasses)}

def softmax(yvec):
    evec = np.exp(yvec)
    if not np.isfinite(evec).all():
        # _min = yvec.min()
        _max = yvec.max()
        shift = 500 - _max
        yvec = yvec+shift
        yvec[yvec < -500] = -500
        evec = np.exp(yvec)
        if not np.isfinite(evec).all():
            ## really lame backoff, but i think this can never happen
            evec[~np.isfinite(evec)] = 1
    z = evec.sum()
    return np.array(evec / z)
    # return np.array(list(evec / z) + [1/z])

import nltk.stem.porter
STEMMER = nltk.stem.porter.PorterStemmer()

def tokenize_for_classifier(text):
    # assume already tokenized
    toks = re.split(r'\s+', text.lower())
    # brandon's toknizer seems to say "u.s" instead of "u.s."
    toks = [ re.sub(r'\.$',"",t) for t in toks]
    toks = [ STEMMER.stem(t) for t in toks ]
    return toks

filter_model = Model()
filter_model.load_model(os.path.join(os.path.dirname(__file__), "FilterModel2.csv"))
# print filter_model.klasses
# print filter_model.coefs

def stat_docfilter(text):
    probs = filter_model.calc_probs(text)
    argmax = max(probs.keys(), key=lambda k: probs[k])
    print (u"STAT\t%s\t%s\t%s" % (argmax, probs[argmax], text.strip()[:80])).encode('utf8')

    # good = (probs['Diplomatic'] + probs['Military'] > 0.5)
    good = argmax=='Diplomatic' or argmax=='Military'
    return good
    # return (
    #         (argmax=='Diplomatic' and probs['Diplomatic'] > 0.4)
    #         or
    #         (argmax=='Military' and probs['Military'] > 0.4)
    #         )

tabari_phrase_list = [re.search(r'^(.*?)\s+\[###', line.strip()).group(1).lower().replace('_',' ').strip() for line in keywordlist.tabari_phrase_list]
tabari_regex = '|'.join([ re.escape(x) for x in tabari_phrase_list ])
tabari_regex = '\b(' + tabari_regex + ')\b'
tabari_regex = re.compile(tabari_regex, re.I)

more_bad_regex = '|'.join(x.strip() for x in keywordlist.more_bad_regexes)
more_bad_regex = re.compile(more_bad_regex)

def keyword_filters(text):
    tokens = text.split()
    token_set = set(tokens)

    if 'LEAD' in token_set:
        # lots of NYTLDC articles that start with "LEAD:" and they're weird
        reject("LEAD")
        return False
    num_nums = sum(1 if re.search(r'^[\d,\.\$]+$', x) else 0 for x in tokens)
    if num_nums >= 5 and num_nums > 0.3 * len(tokens):
        reject("numbers")
        return False

    ## underscore filter has to be at the sentence level
    # if '_' in token_set:
    #     reject("underscore (mistokenization, probably parse error)")
    #     return False

    if tabari_regex.search(text):
        reject("tabari phrase list")
        return False
    if more_bad_regex.search(text):
        reject("phrase list")
        return False
    if keywordlist.PHRASE_BLACKLIST_REGEX.search(text):
        reject("sports team list")
        return False

    return True

def keyword_lemma_filters(lemma_set, lemmatext):
    if tabari_regex.search(lemmatext):
        reject("tabari phrase list, on lemma text")
        return False
    if lemma_set & keywordlist.LEMMA_BLACKLIST:
        reject("lemma")
        return False

    return True


def reject(reason):
    print "REJECT", reason

def main():

    for line in sys.stdin:
        row = line.rstrip('\n').split('\t')
        docid_etc, sent_texts, coreinfo = row[:-2], row[-2], row[-1]
        # docid = docid_etc[0]
        sent_texts = json.loads(sent_texts)
        text = u' '.join(sent_texts)

        print
        print "===",docid_etc
        print "TEXT\t" + text.replace("\n"," ")[:80].encode('utf8')

        good = keyword_filters(text)
        if not good:
            continue

        lemma_set = set()
        lemma_texts = []
        for sent in json.loads(coreinfo)['sentences']:
            lemms = [t[1] for t in sent['tokens']]
            lemma_texts.append(u' '.join(lemms))
            lemma_set |= set(lemms)
        good = keyword_lemma_filters(lemma_set, u' '.join(lemma_texts))
        if not good:
            continue

        good = stat_docfilter(text)
        if not good:
            reject("STAT_DOCFILTER")
            continue

        print "ACCEPT"
        print "DOC\t" + line.strip()
        

if __name__=='__main__':
    main()
