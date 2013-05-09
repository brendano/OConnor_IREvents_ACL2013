# Average Gibbs sample files. 
#  "model.NNN.*" ==> "mean.*"
#  "model.NNN.*" ==> "var.*"

from __future__ import division
import sys,re,glob,os
import ujson as json
from collections import defaultdict
import numpy as np

def available_iters():
    global outdir
    modelfiles = glob.glob("%s/model.*" % outdir)
    numbers = {int(re.search(r'model\.(\d+)', f).group(1)) for f in modelfiles}
    return sorted(list(numbers))

outdir = sys.argv[1]

alliters = available_iters()
if len(sys.argv) < 3:
    print outdir,"Last iter:", alliters[-1] if alliters else None
    if alliters[-1] < 1000: sys.exit()
    startiter, enditer = alliters[-1] - 1000, alliters[-1]
    assert startiter in alliters
else:
    startiter = int(sys.argv[2])
    if len(sys.argv) >= 4:
        enditer = int(sys.argv[3])
    else:
        enditer = alliters[-1]

iters = [i for i in available_iters() if i>=startiter and i<=enditer]
print "Averaging together %d iterations: start %s end %s" % (len(iters), iters[:3], iters[-3:])

def f1(filename): 
    return os.path.join(outdir, filename)
def pf(itr, suffix):
    return os.path.join(outdir, "model.%s.%s" % (itr, suffix))

path_vocab = f1("path.vocab")
path_vocab = np.array([x.strip() for x in open(path_vocab)])
Npath = len(path_vocab)
Ncontext = len(open(f1("context.vocab")).read().strip().split('\n'))
Nframe = len(open(pf(startiter, "cFrame")).read().strip().split('\n'))
Ndyad = len(open(f1("dyad.vocab")).read().strip().split('\n'))
Ntuple = len(open(pf(startiter, "frameIndicators")).read().strip().split('\n'))
Ntime = 1 + max(int(L.split()[0]) for L in open(f1("datefile")))

agg_sum = dict(
cFrame      = np.zeros(Nframe, dtype=float),
cPathFrame  = np.zeros((Npath,Nframe), dtype=float),
frameScales = np.zeros(Nframe-1, dtype=float),
globalCoefs = np.zeros(Nframe-1, dtype=float),
etaVar      = np.zeros(Nframe-1, dtype=float),
# dyadTimeScales=np.zeros(Ndyad*Ntime, dtype=float),
# betas       = np.zeros(Ndyad*Ntime*(Nframe-1), dtype=float),
etas        = np.zeros((Ncontext,Nframe-1), dtype=float),
thetas      = np.zeros((Ncontext, Nframe), dtype=float),
)
agg_ss = {k:v.copy() for k,v in agg_sum.items()}

agg_sparse = dict(
frameIndicators = [defaultdict(int) for i in xrange(Ntuple)],
)

simple_keys = [k for k in agg_sum.keys() if k not in ('thetas',)]

def softmax1(yvec):
    evec = np.exp(yvec)
    if not np.isfinite(evec).all():
        # _min = yvec.min()
        _max = yvec.max()
        shift = 500 - _max
        yvec = yvec+shift
        yvec[yvec < -500] = -500
        evec = np.exp(yvec)
        if not np.isfinite(evec).all():
            ## really lame backoff, but i think this can never happen
            evec[~np.isfinite(evec)] = 1
    z = 1+evec.sum()
    return np.array(list(evec / z) + [1/z])


for itr in iters:
    print itr,
    sys.stdout.flush()
    for k in simple_keys:
        f = pf(itr, k)
        if os.path.exists(f):
            x = np.loadtxt(pf(itr, k))
            x = x.reshape(agg_sum[k].shape)
            agg_sum[k] += x
            agg_ss[k] += x**2

    if 'thetas' in agg_sum:
        etas = np.loadtxt(pf(itr, 'etas')).reshape((Ncontext, Nframe-1))
        for c in xrange(Ncontext):
            theta = softmax1(etas[c])
            agg_sum['thetas'][c] += theta
            agg_ss['thetas'][c] += theta**2

    for k in agg_sparse:
        f = pf(itr, k)
        if os.path.exists(f):
            x = np.loadtxt(pf(itr, k))
            for i,value in enumerate(x):
                assert value==int(value)
                valuekey = "f%d" % int(value)
                agg_sparse[k][i][valuekey] += 1
print

Nsamples = len(iters)
aggmean = {k: agg_sum[k]/Nsamples  for k in agg_sum}
aggvar  = {k: agg_ss[k]/Nsamples - aggmean[k]**2  for k in agg_sum}

for k in agg_sum.keys():
    np.savetxt(f1("mean.%s" % k), aggmean[k], '%.4g')
    np.savetxt(f1("var.%s" % k),  aggvar[k],  '%.4g')

for k in agg_sparse:
    sums = np.array([sum(d.values()) for d in agg_sparse[k]])
    dcts = [ {k:v/sums[i] for k,v in d.items()} for i,d in enumerate(agg_sparse[k]) ]
    with open(f1("mean."+k), 'w') as fp:
        for dct in dcts:
            print>>fp, json.dumps(dct)

np.savetxt(f1("mean.iters"), iters, '%d')
print "Saved: ", f1("mean.*")
# os.system("ls -l %s/mean.*" % outdir)

