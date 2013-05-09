# "internal_diff" reports the pair-based purity scores in the paper.
# "rankcorr_purity" is something else that was not used for the paper.

from __future__ import division
import re,os,sys,random,math,bisect
from collections import defaultdict
import numpy as np
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..","post"))
import util, rankcorr

# is this from http://eli.thegreenplace.net/2010/01/22/weighted-random-generation-in-python/ maybe?
class WeightedRandomGenerator(object):
    def __init__(self, weights):
        self.totals = []
        running_total = 0
        for w in weights:
            running_total += w
            self.totals.append(running_total)
    def next(self):
        rnd = random.random() * self.totals[-1]
        return bisect.bisect_right(self.totals, rnd)
    def __call__(self):
        return self.next()

# x = [1,2,3]
# r = WeightedRandomGenerator(x)
# for i in xrange(int(10e3)):
#     print r()
# sys.exit()

def simple_score(code):
# Neutral: 010
# Cooperative: other 01, 02-08
# Conflictual: 10-20
# Ignore: 09
    if code=='010':
        return 0
    coarse = code[:2]
    if '01' <= coarse <= '08':
        return 1
    if '10' <= coarse <= '20':
        return -1
    return None

goldstein_scores = {}
for line in open(os.path.join(os.path.dirname(__file__), "cameo.05b1.scales")):
    code,score,gloss = line.rstrip('\n').split('\t')
    goldstein_scores[code] = float(score)

def goldstein_score(code):
    global goldstein_scores
    if code in goldstein_scores:
        return goldstein_scores[code]
    if code[:2] in goldstein_scores:
        return goldstein_scores[code[:2]]
    return None

path2score = {}
path2code = {}
path2count = {}  ## should this be a global count?  no, it should be specified relative to the dataset used for modeling

def load_path_goldscores(scorer):
    global path2score, path2code
    # for line in open(os.path.join(os.path.dirname(__file__), "matches.withcounts")):
    for line in open(os.path.join(os.path.dirname(__file__), "v7.path_pattern_matches")):
        code, pattern, path = line.rstrip('\n').split('\t')
        score = scorer(code)
        if score is None: continue
        path2score[path] = score
        path2code[path] = code
    print "%d scored paths loaded" % len(path2score)

def load_path_counts(event_tuple_filename):
    global path2count
    path2count = defaultdict(int)
    for line in open(event_tuple_filename):
        pathstr = line.strip().split('\t')[-1]
        path2count[pathstr] += 1
    path2count = dict(path2count)

cameo_code2score = {}
def load_cameo_goldscores():
    global cameo_code2score
    for line in open(os.path.join(os.path.dirname(__file__), "cameo.05b1.scales")):
        code,score,descr = line.rstrip('\n').split('\t')
        cameo_code2score[code] = float(score)

def dump_scored_paths():
    for path,score in path2score.items():
        print '\t'.join(['%.1f' % score, path])

modeldir, cPathFrame, cFrame, num2path, path2num, scored_paths, frameScales = [None]*7

def load_model(modelprefix):
    global modeldir, cPathFrame, num2path, path2num, scored_paths, frameScales, cFrame
    print "loading ", modelprefix
    modeldir = os.path.dirname(modelprefix)
    cPathFrame = np.loadtxt(modelprefix + ".cPathFrame")
    cFrame = np.loadtxt(modelprefix + ".cFrame")
    # frameScales = [float(x) for x in open(modelprefix + ".frameScales")]
    # frameScales = np.array(frameScales + [0.0])
    num2path = [x.strip() for x in open(os.path.join(modeldir, "path.vocab"))]
    path2num = {p:i for i,p in enumerate(num2path)}
    ## scored_paths is the intersection: paths with gold scores, AND are in the model.
    scored_paths = {p for p in path2score.keys() if p in path2num}
    print>>sys.stderr, len(scored_paths), "paths in model with gold scores"

def purity_report():
    global modeldir, cPathFrame, num2path, path2num, scored_paths, frameScales
    print "<table>"
    print "<br>%d scored paths in model vocab" % len(scored_paths)
    print "<br>showing top-99% scored paths mass per topic"
    print "<thead><th>k <th>score <th>num nonzero types <th>n tok <th>paths"
    print "<tbody>"
    for k in range(cPathFrame.shape[1]):
        print "<tr><td>", k
        total = sum([cPathFrame[path2num[p],k] for p in scored_paths])
        nnz = sum([ cPathFrame[path2num[p],k] > 0 for p in scored_paths] )
        sumscore = np.sum([path2score[p]*cPathFrame[path2num[p],k] for p in scored_paths])
        meanscore = sumscore / total
        print "<td>"
        # print "%.3g mean score, over <td>%d non-zero types, <td>%d total tokens" % (meanscore, nnz, total)
        print "%.3g <td>%d <td>%d" % (meanscore, nnz, total)

        print "<td>"
        pathorder = sorted(scored_paths, key=lambda p: -cPathFrame[path2num[p],k])
        pathorder = [p for p in pathorder if cPathFrame[path2num[p],k] > 0]
        cumsum = 0
        for p in pathorder:
            count = cPathFrame[path2num[p], k]
            cumsum += count
            if cumsum >= 0.99*total: break
            print util.nicepath(p)
            s = path2score[p]
            valence = "pos" if s>0 else "neg" if s<0 else "neu" if s==0 else None
            codeurl = "http://brenocon.com/tabari_cameo_verbs.html#" + path2code[p]
            print "<span class=wordinfo>(%.1f <span class='score %s'>%.1f <a href='%s'>%s</a></span>)</span>" % (
                    count, valence, s, codeurl, path2code[p])
            print ', &nbsp; '
    print "</table>"

def purity_stats():
    for k in range(cPathFrame.shape[1]):
        paths = [p for p in scored_paths if cPathFrame[path2num[p],k] > 0]
        for p in paths:
            row = [path2num[p], k, cPathFrame[path2num[p],k], path2score[p], p]
            print '\t'.join(str(x) for x in row)

def rankcorr_purity():
    # only works for scaled models
    global modeldir, cPathFrame, num2path, path2num, scored_paths, frameScales

    # posterior prob that a pair is concordant
    # path-level arrays (token-level counts)
    model_scores = []
    gold_scores = []
    counts = []

    for k in range(cPathFrame.shape[1]):
        paths = [p for p in scored_paths if cPathFrame[path2num[p],k] > 0]
        for p in paths:
            model_scores.append( path2score[p] )
            gold_scores.append( frameScales[k] )
            counts.append( cPathFrame[path2num[p], k] )
    print len(counts)
    print "rankcorr = %.4f" % (rankcorr.rankcorr(model_scores, gold_scores, counts, method='kendall'))

def rankcorr_purity_null():
    global scored_paths, path2score, cPathFrame
    K = cPathFrame.shape[1]

    for itr in xrange(10000):
        gold = []
        fakepred = []
        counts = []

        for p in scored_paths:
            gold.append(path2score[p])
            fakepred.append(random.randrange(K))
            counts.append(path2count[p])
        r = rankcorr.rankcorr(fakepred, gold, counts, method='kendall')
        print r

def internal_diff():
    global cPathFrame, modeldir
    impurity = _internal_diff2(cPathFrame)
    print "IMPURITY\t%s\t%s" % (modeldir, impurity)

def _internal_diff2(_cPathFrame):
    global num2path, path2num, scored_paths

    # Want Q/N where
    # Q = sum_ij 1{zi=zj} 1{wi,wj in M} d(wi,wj)
    # Q = sum_k sum_{w1,w2 both in M} #(w1, k) #(w2, k)  d(w1, w2)
    # N = sum_k sum_{w1,w2 both in M} #(w1, k) #(w2, k)  1

    Q = 0
    N = 0

    for k in range(_cPathFrame.shape[1]):
        for w1 in scored_paths:
            for w2 in scored_paths:
                if w1==w2: continue
                _w1 = path2num[w1]; c1 = _cPathFrame[_w1,k]
                if c1==0: continue
                _w2 = path2num[w2]; c2 = _cPathFrame[_w2,k]
                if c2==0: continue
                diff = abs(path2score[w1] - path2score[w2])
                Q += c1*c2*diff
                N += c1*c2
    # print Q,N
    return Q/N

def internal_diff_simple_null():
    paths = set(path2score) & set(path2count)
    path_count_score = [(p, path2count[p], path2score[p]) for p in paths]
    gen = WeightedRandomGenerator([c for p,c,s in path_count_score])
    diffs = []
    for itr in xrange(100000):
        i1 = gen()
        i2 = gen()
        if i1==i2: continue
        s1 = path_count_score[i1][2]
        s2 = path_count_score[i2][2]
        diffs.append(abs(s1 - s2))
        # print s1,s2
    print "mean",np.mean(diffs), " sd",np.std(diffs)

def internal_diff_null():
    global cPathFrame, num2path, path2num
    _cPath = cPathFrame.sum(1)
    K = cPathFrame.shape[1]

    for itr in xrange(10):
        fake_cPathFrame = np.zeros((cPathFrame.shape))
        for p in scored_paths:
            topic_assignment = random.randrange(K)
            w = path2num[p]
            fake_cPathFrame[w,topic_assignment] = _cPath[w]
        impurity = _internal_diff2(fake_cPathFrame)
        print "NULL", K, impurity
        sys.stdout.flush()

def cameo_internal_diff():
    assert False, " THIS IS WRONG"
    gethigh = lambda code: code[:2]
    # gethigh = simple_score
    highcodes = {gethigh(code):[] for code in path2code.values()}
    cPath = cPathFrame.sum(1)
    Ntok = 0
    for path in scored_paths:
        code = path2code[path]
        _w = path2num[path]
        highcode = gethigh(code)
        highcodes[highcode].append( (code, cPath[_w]) )
        Ntok += cPath[_w]
    highcode_counts = {highcode: sum(c for _,c in pairs) for highcode,pairs in highcodes.items()}
    grand_total = 0
    for highcode, codecounts in highcodes.items():
        Nk = highcode_counts[highcode]
        # print highcode, Nk
        frame_total = 0
        pair_weights = 0
        for code1,count1 in codecounts:
            for code2,count2 in codecounts:
                weight = count1*count2 / Nk**2
                diff = abs(goldstein_score(code1) - goldstein_score(code2))
                frame_total += weight * diff
                pair_weights += weight
        if pair_weights==0:
            assert frame_total==0
            continue
        grand_total += (Nk/Ntok)**2 * frame_total/pair_weights
    print grand_total / sum((x/Ntok)**2 for x in highcode_counts.values())



def cameo_internal_rankcorr():
    assert False, "not sure we should be using this anymore"
    finescores = []
    coarsescores=[]
    counts=[]
    for p in path2code:
        code = path2code[p]
        if len(code)==2: continue
        if path2count[p] < 5: continue
        finescore = cameo_code2score.get(code)
        if finescore is None: continue
        coarsescore=cameo_code2score[code[:2]]
        # print code, finescore, coarsescore
        finescores.append(finescore)
        coarsescores.append(coarsescore)
        counts.append(path2count[p])
    print "%d path types over %d corpus event instances" % ( len(counts), sum(counts) )
    r1 = rankcorr.rankcorr(finescores, coarsescores, [1]*len(counts), method='gk')
    r2 = rankcorr.rankcorr(finescores, coarsescores, counts, method='gk')
    print "raw rank corr = %.4f, weighted by corpus freq = %.4f" % (r1, r2)

action = sys.argv[1]
modeldir = sys.argv[2] if len(sys.argv)>2 else None

if action=='purity_report':
    util.pageheader()

load_path_goldscores(goldstein_score)
load_cameo_goldscores()
load_path_counts(os.path.join(os.path.dirname(__file__), "../data/v7.pathfil.dthresh=500.pthresh=10"))

if modeldir:
    load_model(modeldir)

eval(action)()

# purity_report()
# purity_stats()
# internal_diff()
# purity_rankcorr()
# purity_null()

