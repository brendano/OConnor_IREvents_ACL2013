# Turn numeric timenum into left-bucket identifier
import sys

datefile = 'data/datefile.month'

mymap = {}
for line in open(datefile):
    tnum, exact_dt, bucketname, left_dt = line.rstrip('\n').split('\t')
    mymap[tnum] = left_dt

for line in sys.stdin:
    parts = line.split()
    parts[0] = mymap[parts[0]]
    print ' '.join(parts)

