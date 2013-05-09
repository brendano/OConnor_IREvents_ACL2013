import re,sys,json
from pprint import pprint
from collections import defaultdict

def read_frame_descriptions(filename):
    global prefix_names, code_names
    prefix_names = {}
    code_names = {}
    for line in open(filename):
        line = line.strip()
        if line.startswith('//'):
            m = re.search(r'^//(\d+)[:=] *(.*)', line)
            if not m: continue
            prefix_code, label = m.groups()
            prefix_names[prefix_code] = label
            continue
        m = re.search(r'^LABEL: *(\d+)= *(.*)', line)
        if not m: continue
        code, label = m.groups()
        code_names[code] = label



def read_patterns(filename):
    global patterns_by_code
    patterns_by_code = defaultdict(list)
    for line in open(filename):
        codespec, pattern, comment = line.split('\t')
        code = re.search('\[([^:\]]*)',codespec).group(1)
        patterns_by_code[code].append((pattern,comment))

read_frame_descriptions("dict/CAMEO.09b5.options")
read_patterns("dict/simple_verbs.tsv")
# read_patterns("dict/verbs.tsv")

mode = sys.argv[1]
if mode=='json':
    for code,patterns in patterns_by_code.items():
        for pat,comment in patterns:
            d = {}
            d['code'] = code
            d['pattern'] = pat
            print json.dumps(d)

elif mode=='html':



    # print prefix_names

    print """
    <style>
    .comment { color: gray; font-size: 70% }
    </style>

    <h1>Tabari patterns for the CAMEO frame system</h1>

    These identify binary relations for roles "source" and "target", and by default, the source is on the left of the verb pattern, and the target is on the right.  This can be overridden with various symbols (section 5.5 of the Tabari manual):

    <ul>
    <li> + (plus sign) indicates the target
    <li> $ (dollar sign) indicates the source
    <li> % (percent sign) indicates that actor is both source and target
    </ul>

    By default, gaps are allowed anywhere in the phrase.  (I think there is a max size for a gap?)  Joining two words with an underscore (e.g. "carried_out") forces them to be contiguous.  By default, tokens are matched via prefix matching; a trailing underscore prevents this (e.g. "of_" only matches the preposition, not "officer").
    <hr>
    """

    print "<ul>"
    for prefix in sorted(prefix_names.keys()):
        print "<li>"
        codes = sorted([c for c in code_names.keys() if c.startswith(prefix)])
        print "<a href='#{p}'>{p} - {n}</a> ({tot} top-level patterns)".format(p=prefix, n=prefix_names[prefix],
            tot=sum(len(patterns_by_code[code]) for code in codes))
    print "</ul>"

    print "<ul>"
    for prefix in sorted(prefix_names.keys()):
        print "<li>"
        codes = sorted([c for c in code_names.keys() if c.startswith(prefix)])
        print "<a href='#{p}'>{p} - {n}</a> ({tot} patterns)".format(p=prefix, n=prefix_names[prefix],
            tot=sum(len(patterns_by_code[code]) for code in codes))
        print "<ul>"
        for code in codes:
            print "<li>"
            print "<a href='#{c}'>{c} - {n}</a> ({tot} patterns)".format(c=code, n=code_names[code], tot=len(patterns_by_code[code]))
        print "</ul>"
    print "</ul>"
    print "<hr>"

    for prefix in sorted(prefix_names.keys()):
        print "<a name={p}><h1>{p} - {n}</h1></a>".format(p=prefix, n=prefix_names[prefix])
        for code in sorted([c for c in code_names.keys() if c.startswith(prefix)]):
            tot = len(patterns_by_code[code])
            print "<a name={c}><h2>{c} - {n} ({tot} patterns)</h2></a>".format(
                c=code, n=code_names[code], tot=tot)
            print ''.join("<br>{} {}".format(
                    pat,
                    '&nbsp;&nbsp;<span class=comment>{}</span>'.format(comment) if comment else '')
                for pat,comment in patterns_by_code[code])


else: assert False
