import sys,re
from collections import defaultdict

import argparse
p = argparse.ArgumentParser()
p.add_argument('tabari_input')
p.add_argument('tabari_output')
p.add_argument('--pospred-only', action='store_true')
args = p.parse_args()

extractions = defaultdict(list)

for line in open(args.tabari_output):
    line = line.rstrip('\n')
    docid = line.split('\t')[-1]
    extractions[docid].append( line )

docs = {}
for rawtext in open(args.tabari_input).read().strip().split("\n\n"):
    lines = rawtext.split("\n")
    datestamp,docid = lines[0].split()
    # datestamp not interesting to us, would rather use docid for everything
    text = '\n'.join(lines[1:])
    docs[docid] = {'docid':docid, 'text':text}

for docid,doc in sorted(docs.items()):
    if args.pospred_only and docid not in extractions:
        continue
    orig_docid,sentid = re.split(':S', docid)
    sentid = 'S'+sentid
    orig_text = ' '.join(doc['text'].split())
    print "=== {}\t{}\t{}".format(orig_docid, sentid, orig_text)
    print
    for line in extractions[docid]:
        print line
    print
