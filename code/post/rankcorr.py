from __future__ import division
import math

def rankcorr(X, Y, counts, method):
    # Rank correlations based on concordant and discordant pairs, optionally
    # with weights.
    # Arguments: Three parallel vectors for the data,
    #   X,Y: have to admit an ordering, but can be floats, ints, tuples,
    #        whatever.
    #   counts: the count or weight for this contingency table cell. give all
    #       1's to be instance level.
    # method: 
    #   gk (goodman-kruskal)
    #       = (A - D) / (A + D)
    #       = 2*p-1 where p = p(pair agrees | neither side ties)
    #   kendall:
    #       = (A - D) / sqrt((A+D+T1)*(A+D+T2))
    # (A=numagree, B=numdisagree, T1=ties only on X, T2=ties only on Y)
    #
    # If X are gold labels and Y are predicted confidences, then goodman-kruskal
    # is the same as rescaled AUC (AUC=p above); I think this is also known as
    # the "Gini index" but it's hard to find information online about it since
    # it shares that name with the presumably related univar diversity measure,
    # the Gini coefficient.
    # 
    # The kendall's tau here is "tau-b", the same as found in R cor() and
    # scipy.stats.  if the dimensionalities of the variables are different
    # ("non-square table") then it can't reach +1 or -1.
    # tau-a is the version Kendall's tau that doesn't know what to do about
    # ties. unimplemented.
    # tau-c is supposed to be like tau-b but it adjusts for different
    # dimensions between X and Y.  but it varies based on
    # number of dimensions of unseen categories.  this seems odd.
    # goodman-kruskal does not have this issue.
    assert method in ('kendall','gk')
    N = len(X)
    assert N == len(Y) == len(counts)

    Ngoldonly_ties = 0
    Npredonly_ties = 0
    Nagree = 0
    Ndisagree = 0
    for i in range(N):
        for j in range(i+1, N):
            goldtie = X[i]==X[j]
            predtie = Y[i]==Y[j]
            if goldtie and predtie:
                continue
            w = counts[i] * counts[j]
            Ngoldonly_ties += w * (goldtie and not predtie)
            Npredonly_ties += w * (predtie and not goldtie)
            if goldtie or predtie:
                continue
            agree = ((X[i]>X[j] and Y[i]>Y[j])
                        or
                     (X[i]<X[j] and Y[i]<Y[j]))
            # print agree, i,j
            Nagree += w*agree
            Ndisagree += w*(not agree)
    # print Nagree, Ndisagree, Ngoldonly_ties, Npredonly_ties
    if method=='kendall':
        n = (Nagree+Ndisagree+Ngoldonly_ties) * (Nagree+Ndisagree+Npredonly_ties)
        return (Nagree - Ndisagree) / math.sqrt(n)
    if method=='gk':
        return (Nagree - Ndisagree) / (Nagree + Ndisagree)
    assert False

def randpair_test(goldcats, predscores, counts):
    import random
    assert all(x==1 for x in counts)
    N = len(goldcats)
    tot,num_pred_lt = 0,0
    for itr in xrange(10000):
        i = random.randrange(N)
        j = random.randrange(N)
        # if i==j: continue
        if not goldcats[i] < goldcats[j]: continue
        # if predscores[i]==predscores[j]: continue
        assert predscores[i] != predscores[j]
        tot += 1
        num_pred_lt += (predscores[i] < predscores[j])
    return num_pred_lt/tot, tot

if __name__=='__main__':
    import sys
    lines = sys.stdin.read().strip().split('\n')
    rows = [ [float(x) for x in line.split()] for line in lines]
    X, Y, W =[row[0] for row in rows], [row[1] for row in rows], [row[2] for row in rows]
    k = rankcorr(X,Y,W, method='kendall')
    print 'kendall', k, (k+1)/2
    gk = rankcorr(X,Y,W, method='gk')
    print "Goodman-Kruskal", gk, (gk+1)/2

    # print "estimated via simulation"
    # p,n = randpair_test([row[0] for row in rows], [row[1] for row in rows], [row[2] for row in rows])
    # print p,'+/-', 1.96*math.sqrt(p*(1-p)/n)

