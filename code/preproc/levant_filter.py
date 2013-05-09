import sys
from collections import Counter

dyad_count_cutoff = 15

isgood = lambda a: a in set(['ISR','EGY','PAL','SYR','LEB','USA','CHN','GBR'])
# isgood = lambda a: a.endswith('GOV')

dyad_counts = Counter()

filtered_rows = []

for line in sys.stdin:
    row = line.rstrip('\n').split('\t')
    a1,a2 = row[1],row[2]
    # a1=a1[:3]; a2=a2[:3]
    # row[1],row[2]=a1,a2
    if isgood(a1) and isgood(a2) and a1!=a2:
        dyad_counts[a1,a2] += 1
        filtered_rows.append(row)

goods = {dyad for dyad,c in dyad_counts.items() if c>=dyad_count_cutoff}
filtered_rows = [row for row in filtered_rows if (row[1],row[2]) in goods]
print>>sys.stderr, "{} dyads, {} events at cutoff {}".format(len(goods), len(filtered_rows), dyad_count_cutoff)
for row in filtered_rows:
    print '\t'.join(row)
