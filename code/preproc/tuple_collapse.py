# Takes the multiline, human-readable-ish plus JSON tuple extraction format
# and turns into one-line-per-event TSV, the "tuplecollapsed" format.
import sys, re, os
import ujson as json
from datetime import datetime

def iter_sents():
    cur = []
    for line in sys.stdin:
        if line.startswith('==='):
            if cur: yield cur
            cur = []
        line = line.rstrip('\n')
        if line:
            cur.append(line)

nytldc_dates = {}
print>>sys.stderr, "loading NYTLDC metadata"
for line in os.popen("pv %s" % "/cab1/brendano/sem/nlp_preproc/nyt_ldc/allmeta.tsv"):
    docid,datestr=  line.strip().split("\t")
    nytldc_dates[docid] = datestr

for lines in iter_sents():
    x = lines[0].split('\t')
    docid = x[0].split()[-1]
    sentid = x[1]
    # for gigaword
    m = re.search(r'_(\d\d\d\d\d\d\d\d)', docid)
    if m:
        datestr = m.group(1)
        dt = datetime.strptime(datestr, '%Y%m%d')
        dt = dt.strftime('%Y-%m-%d')
    else:
        dt = nytldc_dates[docid]

    tuplelines = [L for L in lines if L.startswith('TUPLE')]
    for line in tuplelines:
        tpl = json.loads(line.split('\t')[-1])
        row = [docid, json.dumps(tpl['tupleid']),
                dt,
                tpl['src']['entityid'],
                tpl['rec']['entityid'],
                json.dumps(tpl['pred']['path']),
            ]
        print '\t'.join(row)

            
