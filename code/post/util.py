import re,sys,json

def nicepath(pathstr, html=True):
    d = json.loads(pathstr)
    s = nicepath2(d)
    if not html:
        s = re.sub(r'\<.*?\>', "", s)
        s = re.sub(" +", " ", s)
        s = s.replace("&larr;", "<-").replace("&rarr;", "->")
        s = s.strip()
    return s

def nicepath2(path_arr):
    out = []
    for x in path_arr:
        if x[0]=='A':
            _,rel,arrow = x
            if rel.startswith("prep_"):
                out.append(rel.replace("prep_",""))
            elif rel=='dobj' or rel=='semagent':
                pass
            else:
                deparc = ("&larr;"+rel) if arrow=='<-' else (rel+"&rarr;")
                out.append("<span class=depedge>" +deparc+ "</span>")
        elif x[0]=='W':
            _,lemma,pos = x
            out.append(lemma)
    return ' '.join(out)

def parse_wordpos(wpos):
    # report/VERB
    if wpos is None: return wpos
    parts = wpos.split('/')
    return ['/'.join(parts[:-1]), parts[-1]]

def parse_relhop(relhop):
    direction = 'DOWN' if relhop.endswith("<-") else 'UP' if relhop.endswith("->") else None
    assert direction is not None
    rel = relhop.replace("<-","").replace("->","")
    rel = re.sub(",$","", rel)
    return [rel,direction]

def parsepath(pathstr):
    # deal with the stupid pipe/arrow format i made up a while ago
    # returns a JSON-friendly nest list format
    parts = pathstr.split('|')
    parts[0] = None
    result = [None]*len(parts)
    for i in range(0, len(parts), 2):
        result[i] = parse_wordpos(parts[i])
        result[i+1]=parse_relhop(parts[i+1])
    return result

def pageheader():
    print """
    <meta content="text/html; charset=utf-8" http-equiv="Content-Type"/>
    <style>
    .depedge { font-size: 8pt; color: #333; }
    .wordinfo { font-size: 9pt; }
    .pos { color: blue; }
    .neg { color: red; }
    .score a { color: inherit; }
    </style>
    <link rel="stylesheet" href="http://brenocon.com/js/tablesorter/2.7.2/css/mytheme.css" >
    
    <script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.9.0/jquery.min.js"></script> 
    <script type="text/javascript" src="http://brenocon.com/js/tablesorter/2.7.2/js/jquery.tablesorter.min.js"></script> 
    """
    # <script type="text/javascript" src="http://brenocon.com/js/tablesorter/2.7.2/js/jquery.tablesorter.widgets.js"></script> 
    print """<script>
    $(document).ready(function() 
        { 
            $("table").tablesorter();
        } 
    ); 
    </script>
    """

if __name__=='__main__':
    for line in sys.stdin:
        parts = line.rstrip('\n').split()
        parts.append(nicepath(parts[-1], html=False))
        print '\t'.join(parts)
        # print parsepath(line.strip())
