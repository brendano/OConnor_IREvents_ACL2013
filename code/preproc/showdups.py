# postprocess output of docfilter.py
# to look at how the duplicate detector is doing

import re,sys
from collections import defaultdict

parts = sys.stdin.read().split("===")
index = defaultdict(list)
for part in parts:
    if not part.strip(): continue
    text = re.search(r'TEXT\t[^\n]*', part).group()
    sig = re.search(r'SIG\t[^\n]*', part).group()
    index[sig].append(text)

for sig,docs in sorted(index.items()):
    if len(docs)>1:
        print "===", len(docs), " duplicates"
        print sig
        for doc in docs:
            print doc

