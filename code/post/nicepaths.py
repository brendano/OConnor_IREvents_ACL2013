import json, sys
import util

for line in sys.stdin:
    parts = line.rstrip('\n').split('\t')
    parts[-1] = util.nicepath(parts[-1], html=False)
    print '\t'.join(parts)
