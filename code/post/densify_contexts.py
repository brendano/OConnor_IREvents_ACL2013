"""
Takes a file matrix shaped (Ncontext, Nframe)
Transforms into multidim array shaped (Ndyad, Ntime, Nframe)
Save file in C order. So in R (fortran order), it's shaped (Nframe, Ntime, Ndyad).
This works for either .thetas or .etas (and does it as Nframe-1 for etas).

Data sparsity implies many missing values (since Ndyad*Ntime >> Ncontext)
Missing values are positive infinity (because numpy doesnt support missing values)

SANITY TEST:

~/ptab/bl/230916.v5.sf.k=5.c=1.trim % python ~/ptab/post/densify_contexts.py .
loading ./mean.etas
shape (34554, 4)
output shape (2955, 739, 4)
contextid 100  has dyadid 87  and timeid 2
[ 5.339 -3.569  4.928  2.535]
[ 5.339 -3.569  4.928  2.535]
saving ./mean.etas.dense

~/ptab/bl/230916.v5.sf.k=5.c=1.trim % R
> x=array(scan("mean.etas.dense"), c(4,739,2955))
> dim(x)
[1]    4  739 2955
> x[,3,88]
[1]  5.339 -3.569  4.928  2.535

> x[!is.finite(x)]=NA
> mean(x,na.rm=T)
[1] -0.01105422
> quantile(x,na.rm=T)
       0%       25%       50%       75%      100% 
-49.73000  -6.49800   0.76380   6.88925  46.42000 
"""
import sys,os
import numpy as np

INF = float("inf")

basedir = sys.argv[1]

def f1(filename): 
    return os.path.join(basedir, filename)

dyad_vocab = {}
context_reversevocab = {}

Ncontext = len(open(f1("context.vocab")).read().strip().split('\n'))
# Nframe = len(open(f1("%s.cFrame" % modelname)).read().strip().split('\n'))
Ndyad = len(open(f1("dyad.vocab")).read().strip().split('\n'))
Ntime = 739

for dyadid, line in enumerate(open("%s/dyad.vocab" % basedir)):
    src, tgt, srcid, tgtid = line.split()
    dyad_vocab[src,tgt] = dyadid

for contextid, line in enumerate(open("%s/context.vocab" % basedir)):
    timeid, src, tgt = line.split()
    timeid = int(timeid)
    # context_vocab[timeid, src, tgt] = contextid
    context_reversevocab[contextid] = (timeid, src, tgt)

assert Ncontext == len(context_reversevocab)

def process_contextfile(basename, input_suffix):
    inf = "%s/%s.%s" % (basedir, basename, input_suffix)
    outf= "%s/%s.%s.dense" % (basedir, basename, input_suffix)

    print "loading",inf
    data = np.loadtxt(inf)
    print "shape", data.shape
    if len(data.shape)==1:
        # very low K, or a cContext file
        data = data.reshape((len(data),1))
    assert data.shape[0] == Ncontext
    ncol = data.shape[1]  ## could be Nframe or Nframe-1 for thetas or etas
    output = np.ones((Ndyad, Ntime, ncol)) * INF
    print "output shape", output.shape
    for contextid, row in enumerate(data):
        timeid,src,tgt = context_reversevocab[contextid]
        dyadid = dyad_vocab[src,tgt]
        assert dyadid < Ndyad
        output[dyadid,timeid] = row
        if contextid==100:
            print "contextid", contextid," has dyadid",dyadid," and timeid",timeid
            print "dyad is", src, tgt
            print data[contextid], " (input version)"
            print output[dyadid,timeid], " (output version)"
    print "saving", outf
    np.savetxt(outf, output.flatten(), "%.4g")

# for pre in ['mean','var']:
#     for varname in ['etas','thetas']:
#         process_contextfile(pre, varname)

# process_contextfile("mean",cContext")
# process_contextfile("mean","etas")
# process_contextfile("var","etas")
process_contextfile("mean","thetas")

