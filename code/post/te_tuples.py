# make tuple posteriors format for text explorer
import ujson as json
import sys, os

tuple_collapsed_file = sys.argv[1]
frameind_mean_jsonfile = sys.argv[2]

lines1 = os.popen("wc -l < %s" % tuple_collapsed_file).read().strip()
lines2 = os.popen("wc -l < %s" % frameind_mean_jsonfile).read().strip()
assert lines1 == lines2

for tupleline,postline in zip(open(tuple_collapsed_file), open(frameind_mean_jsonfile)):
    tpl_row = tupleline.split('\t')
    docid,tupleid = tpl_row[:2]
    tupleid = json.loads(tupleid)
    gtid = [docid,tupleid]

    post_dict = json.loads(postline)
    post_dict['gtid'] = gtid
    print json.dumps(post_dict)

