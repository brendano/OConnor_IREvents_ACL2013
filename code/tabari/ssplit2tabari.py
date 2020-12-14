from __future__ import division
import sys,json,itertools,textwrap,string,re
from pprint import pprint

for line in sys.stdin:
    row = line.split('\t')
    if len(row) != 3:
        print>>sys.stderr, line[:200]
        continue
    docid, datestr, ssplit = row
    try:
        ssplit = json.loads(ssplit)
    except ValueError, e:
        print>>sys.stderr, e
        print>>sys.stderr, line[:200]
        continue

    y,m,d = datestr.split('-')
    tabari_date = "{}{}{}".format(y[2:4],m,d)

    for snum,senttext in enumerate(ssplit):
        sentid = "S{}".format(snum)
        print "{tabari_date} {docid}:{sentid}".format(**locals())
        text = '\n'.join(textwrap.wrap(senttext, 70))
        print text
        print
# 
# 
# 
# def output_tabari():
#     for docid, sent_rows in filter_article_sentences():
#         for sent_row in sent_rows:
#             sentid,toktext = sent_row[1:3]
#             dt = None
# 
#             ## Gigaword's format for dates in docids
#             m = re.search(r'_(\d{8})\.', docid)
#             if m:
#                 datestamp = re.search(r'_(\d{8})\.', docid).group(1)
#                 dt = '{}{}{}'.format(datestamp[2:4], datestamp[4:6], datestamp[6:8])
#             ## My quick hack one for NYT-LDC
#             m = re.search(r'::(\d\d\d\d)-(\d\d)-(\d\d)', docid)
#             if m:
#                 dt = '{}{}{}'.format(m.group(1)[2:],m.group(2),m.group(3))
# 
#             assert dt
# 
#             print "{dt} {docid}:{sentid}".format(**locals())
#             text = toktext
#             text = '\n'.join(textwrap.wrap(text, 70))
#             print text
#             print

