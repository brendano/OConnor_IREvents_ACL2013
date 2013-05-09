# Take verb pattern file and turn it into a flat TSV format
# and substitute the triggerwords into the * positions
# e.g.
# cat dict/CAMEO.080612.verbs | python simplify_tabari_patterns.py > verbs.tsv

# $ indicates SOURCE
# + indicates TARGET
# % (percent sign) indicates that actor is both source and target

mappings = {
    '$': 'SOURCE',
    '+': 'TARGET',
    '%': 'BOTHST',
    '^': 'ENTITYSKIP',
}

import sys,re

topword = None
leftpart,code,comment = '','',''

def fixpat(pat):
    parts = pat.split()
    assert all(x not in mappings.values() for x in parts)
    parts = [mappings.get(x, x) for x in parts]
    if parts[-1]=='TARGET':
        parts = parts[:-1]
    if parts[0]=='SOURCE':
        parts = parts[1:]
    s = ' '.join(parts)
    # for k,v in mappings.items():
    #     s = s.replace(k, v)
    return s

def emit(pat):
    global code,comment
    pat = fixpat(pat)
    print "{}\t{}\t{}\t{}".format(code, topword, pat, comment)


for line in sys.stdin:
    s = line.strip()
    if '~~FINISH' in s: continue
    # print "INPUT\t" + line.strip()

    s = s.lower()
    m = re.search(r'(^.*)(\[[^\]]+\]) *(;?.*)', s)
    leftpart,code,comment = m.groups()

    leftpart = leftpart.strip().replace('\t',' ')
    code = code.strip().replace('\t',' ')
    code = re.sub(r'^\[', '' ,code)
    code = re.sub(r'\]$', '' ,code)
    comment = comment.strip().replace('\t',' ')

    # print "\nINPUT\t\t" + s

    if leftpart[0]=='-':
        # continuation
        pat = re.search(r'^- +(.*)', leftpart).group(1)
        pat = pat.replace('*', topword)
        emit(pat)
    else:
        topword = leftpart
        emit(topword)

