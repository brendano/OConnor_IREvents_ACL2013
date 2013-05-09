import json,sys
for line in sys.stdin:
    docid,date,sents = line.split('\t')
    sents = json.loads(sents)
    sents = sents[:5]
    print '\t'.join([docid,date,json.dumps(sents)])
