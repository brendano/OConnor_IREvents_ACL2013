# takes tuple-formatted extractions and turns into TSV
# intended for Crowdflower input

import sys,json

country_names = {k:v.strip() for k,v in (line.strip().split("\t") for line in open("/users/brendano/ptab/dict/country_canonical_names.txt"))}

columns = ['docid','sentid','code','pretext','senttext','src_id','rec_id', 'src_name','rec_name']
print '\t'.join(columns)

docinfo = None
sentinfo = None

for line in sys.stdin:
    if not line.strip(): continue
    row = line.rstrip('\n').split('\t')
    row = [x.decode('utf8') for x in row]
    if row[0]=='DOC':
        d = {}
        d['docid'] = row[1]
        d['senttexts'] = json.loads(row[2])
        docinfo = d
    if row[0]=='SENT':
        d = {}
        d['docid'],d['sentid'],d['senttext'] = row[1:4]
        d['sentnum'] = int(d['sentid'].replace("S",""))
        sentinfo = d
    if row[0]=='TUPLE':
        tpl = json.loads(row[-1])
        si = sentinfo['sentnum']
        d = {}
        d.update(docinfo)
        d.update(sentinfo)
        d['pretext'] = u' '.join(docinfo['senttexts'][:si]).strip()
        if not d['pretext']:
            d['pretext'] = "[Start of article]"

        d['src_id'],d['rec_id'] = tpl['src']['entityid'], tpl['rec']['entityid']
        if d['src_id'][:3] not in country_names or d['rec_id'][:3] not in country_names:
            print>>sys.stderr,"skipping since don't have country name", d['src_id'], d['rec_id']
            continue
        d['src_name'] = country_names[d['src_id'][:3]]
        d['rec_name'] = country_names[d['rec_id'][:3]]
        d['code'] = tpl['code']
        print u'\t'.join(d[k] for k in columns).encode('utf8')

