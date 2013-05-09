from __future__ import division
import sys,re,itertools

dyad_countfile = sys.argv[1]
path_countfile = sys.argv[2]
dyad_thresh = int(sys.argv[3])
path_thresh = int(sys.argv[4])

dyad_whitelist = []
for line in open(dyad_countfile):
    count,src,tgt = line.split()
    count=int(count)
    if src==tgt: continue
    if count < dyad_thresh: continue
    dyad_whitelist.append((src,tgt))
    dyad_whitelist.append((tgt,src))
dyad_whitelist = set(dyad_whitelist)
print>>sys.stderr, "%d directed dyads at %d (undir?) threshold" % (len(dyad_whitelist), dyad_thresh)

path_whitelist = {x.split()[-1] for x in open(path_countfile) if int(x.split()[0]) >= path_thresh}
print>>sys.stderr, "%d paths at %d threshold" % (len(path_whitelist), path_thresh)

# print dyad_whitelist
# print path_whitelist

for line in sys.stdin:
    line = line.rstrip('\n')
    row = line.split('\t')
    docid,sentid,datestr,src,tgt,path = row
    if (src,tgt) not in dyad_whitelist: continue
    if path not in path_whitelist: continue
    print line
