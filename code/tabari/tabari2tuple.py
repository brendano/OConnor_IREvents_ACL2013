import sys,re
import ujson as json
from collections import defaultdict
import argparse

p = argparse.ArgumentParser()
p.add_argument('tabari_output')
p.add_argument('--pospred-only', action='store_true')
args = p.parse_args()

extractions = defaultdict(list)

for line in open(args.tabari_output):
    line = line.rstrip('\n')
    row = line.split('\t')
    docid,sentid = row[-1].split(":S")
    sentid = "S"+sentid
    d = {'docid':docid,'sentid':sentid}
    d['src'],d['rec'],d['code'] = row[1:4]
    extractions[docid].append(d)

currently_in_doc = False
for line in sys.stdin:
    # docid,sentid,text = line.rstrip('\n').split('\t')
    # docid,text = 
    row = line.rstrip('\n').split('\t')
    docid = row[0]
    sents = row[-1]

    if docid not in extractions:
        continue

    print
    print '\t'.join(['DOC', docid, sents])

    sents = json.loads(sents)
    for si,senttext in enumerate(sents):
        sentid = "S%d" % si
        tuples_for_sent = [t for t in extractions.get(docid,[]) if t['sentid']==sentid]
        if not tuples_for_sent: continue

        print
        print '\t'.join(['SENT', docid, sentid, senttext])
        # print "=== %s\t%s\t%s" % (docid, sentid, senttext)
        for i,t in enumerate(tuples_for_sent):
            d = {'tupleid':[int(sentid.replace("S","")), i],
                'src':{'entityid':t['src']},
                'rec':{'entityid':t['rec']},
                'code':t['code']
                }
            print "TUPLE\t%s" % json.dumps(d)
            # print "TSV\t%s\t%s\t%s" % (t['src'],t['rec'],text)
