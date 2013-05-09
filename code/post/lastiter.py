import re,sys,glob

def available_iters(outdir):
    modelfiles = glob.glob("%s/model.*" % outdir)
    numbers = {int(re.search(r'model\.(\d+)', f).group(1)) for f in modelfiles}
    return sorted(list(numbers))

for outdir in sys.argv[1:]:
    alliters = available_iters(outdir)
    lastiter = alliters[-1] if alliters else 0
    print "%s\t%s" % (lastiter, outdir)
