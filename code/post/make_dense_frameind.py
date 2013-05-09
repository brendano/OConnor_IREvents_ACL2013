import sys,json,os
modeldir = sys.argv[1]

fn = lambda f: os.path.join(modeldir, f)
Nframe = len(open(fn("mean.cFrame")).read().split())
with open(fn("mean.frameIndicators.txt"), 'w') as fp:
    for line in open(fn("mean.frameIndicators")):
        out = [0]*Nframe
        d = json.loads(line)
        for k,v in d.items():
            k = k.replace("f","")
            k = int(k)
            out[k] = v
        print>>fp, ' '.join(str(x) for x in out)
