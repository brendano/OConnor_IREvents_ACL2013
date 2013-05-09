# make the datefile that maps individual dates to rollup periods.

import sys
from collections import OrderedDict
from datetime import datetime, timedelta
import tsutil

class Numberizer(OrderedDict):
  def num(self, ob):
    if ob not in self:
      self[ob] = len(self)   # +1  ==>  1-indexed
    return self[ob]

granularity = sys.argv[1]
collfile = sys.argv[2]

assert granularity in tsutil.rollup_bucket

def rowiter():
    for line in open(collfile):
        row = line.rstrip('\n').split('\t')
        yield row


dates = {datetime.strptime(r[2], '%Y-%m-%d') for r in rowiter()}
mind,maxd = min(dates), max(dates)

alldates = []
d = mind
while d <= maxd:
    alldates.append(d)
    d = d + timedelta(days=1)

timebucket_to_integer = Numberizer()
for d in alldates:
    bucket = tsutil.rollup_bucket[granularity](d)
    timenum = timebucket_to_integer.num(bucket)
    print '\t'.join([
        str(timenum),
        d.strftime('%Y-%m-%d'), 
        repr(bucket), 
        tsutil.bucket_leftdt[granularity](bucket).strftime('%Y-%m-%d'),
        ])


