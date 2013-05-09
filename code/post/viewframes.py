import os,sys,re,json
import numpy as np
import util

prefix = sys.argv[1]
def fname(suff): return prefix + "." + suff

wc_thresh = 2

path_frames = np.loadtxt(fname("cPathFrame"))
framecounts = np.loadtxt(fname("cFrame"))
etaVar= np.loadtxt(fname("etaVar"))
Nframe = path_frames.shape[1]
if os.path.exists(fname("frameScales")):
    frame_scales = np.loadtxt(fname("frameScales"))
else:
    frame_scales = np.array([0.0] * (Nframe-1))
if os.path.exists(fname("frameScales")):
    alpha = np.loadtxt(fname("globalCoefs"))
else:
    alpha = np.array([0.0] * (Nframe-1))

def fix(x):
    if len(x.shape)==0:
        x = x.reshape((1,))
    return np.concatenate((x, [0]))
alpha = fix(alpha)
etaVar= fix(etaVar)
frame_scales = fix(frame_scales)

path_vocab = os.path.join(os.path.dirname(prefix), "path.vocab")
path_vocab = np.array([x.strip() for x in open(path_vocab)])

assert len(frame_scales.shape)==1
if len(frame_scales.shape)==1:
    frame_scales = frame_scales.reshape((len(frame_scales),1))

assert Nframe == len(framecounts)
Ndim = frame_scales.shape[1]

frameorder = frame_scales[:,0].argsort()

util.pageheader()

print "<b>" + prefix + "</b>"
print "only showing paths with count >=", wc_thresh

print "<table class=tablesorter cellpadding=3 border=1 cellspacing=0 width='100%'>"

thead = ['eventtype'] + ['count','alpha','etaVar'] + ['d=%d' % dim for dim in range(Ndim)] + ['top paths']
print "<thead>", ' '.join(["<th>"+x for x in thead]), "</thead>"
print "<tbody>"
print

for k in frameorder:
    top_paths = (-path_frames[:,k]).argsort()[:20]
    top_paths = top_paths[ path_frames[top_paths,k] >= wc_thresh]
    pathelts = [util.nicepath(x) for x in path_vocab[top_paths]]
    # pathelts = ["%s <span class=wordinfo>(%.0f)</span>" % (util.nicepath(path_vocab[i]), path_frames[i,k]) for i in top_paths]
    pathelts = ["%s" % (util.nicepath(path_vocab[i]),) for i in top_paths]

    pathinfo = ',&nbsp; '.join(pathelts)
    row = ['f=%s' % k, str(framecounts[k]),
            '%.3g' % alpha[k], '%.3g' % etaVar[k],
        ]
    row += ["%.3g" % x for x in frame_scales[k,:]]
    row += [pathinfo]
    # row += [str(path_vocab[top_paths])]
    print '<tr>' + ' '.join('<td>'+str(x) for x in row)

print "</tbody>"
print "</table>"


