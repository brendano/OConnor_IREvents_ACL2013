import sys
import ujson as json

def get_ngrams(tokens, n):
    return [tuple(tokens[i:(i+n)]) for i in range(len(tokens) - (n-1))]

def doc_signature(tokens):
    tokens = [t.lower() for t in tokens]
    ngrams = get_ngrams(tokens, 2)
    ngrams = list(set(ngrams))
    ngrams.sort(key=lambda ng: hash(ng))
    return tuple(ngrams[:5])

def reject(reason):
    print "REJECT", reason

def main():
    seen_signatures = set()

    for line in sys.stdin:
        row = line.rstrip('\n').split('\t')
        docid_etc, sent_texts, coreinfo = row[:-2], row[-2], row[-1]
        # docid = docid_etc[0]
        sent_texts = json.loads(sent_texts)
        text = u' '.join(sent_texts)

        print
        print "===",docid_etc
        print "TEXT\t" + text.replace("\n"," ")[:80].encode('utf8')

        tokens = text.split()
        sig = doc_signature(tokens)
        print "SIG\t%s\t%s" % (hash(sig), json.dumps(sig))

        if sig in seen_signatures:
            reject("DUPLICATE")
            continue
        seen_signatures.add(sig)

        print "DOC\t" + line.strip()

if __name__=='__main__':
    main()
