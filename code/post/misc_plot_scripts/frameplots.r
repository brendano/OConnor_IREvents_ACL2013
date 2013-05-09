# assume 'x' is the multidim array for thetas, dimension
# assume 'v' is the variance version
dyadvocab=readLines("dyad.vocab")
Nframe = dim(x)[1]

## Data loading
## note cContext is 0 when missing, but the others should be NA.
# x=array(scan("mean.thetas.dense"), c(5,739,2955));x[!is.finite(x)]=NA
## Read 10918725 items
# v=array(scan("var.thetas.dense"), c(5,739,2955));v[!is.finite(v)]=NA
## Read 10918725 items
# cContext=array(scan("mean.cContext.dense"),c(739,2955)); cContext[!is.finite(cContext)]=0

## Convenient
# dyads=data.frame(name=gsub(" \\d+ \\d+","",dyadvocab),id=1:length(dyadvocab),count=colSums(cContext,na.rm=T))



framets1 = function(dyads) {
# dyads = c(33, 138) ## israel palestine
# dyads = c(32, 42) ## usa iraq
# dyads = arrange(dyads[bgrep("IRQ",dyads$name),], -count)$id[1:6]

  par(mfrow=c(Nframe+1, length(dyads)), mar=c(4,2,1,1))
# for (k in 1:5) {plot(x[k,,33],ylim=c(0,1),type='b'); plot(x[k,,138],ylim=c(0,1),type='b')}

  for (d in dyads) {
    plot(cContext[,d], type='b', main=sprintf("totalcounts for %s", dyadvocab[d]))
  }

  for (k in 1:Nframe) {
    for (d in dyads) {
      plot(x[k,,d], ylim=c(0,1), type='b', main=sprintf("k=%d %s", k, dyadvocab[d]))
    }
  }
}

framets2 = function(dyads) {

  par(mfrow=c(length(dyads), Nframe+1), mar=c(2.7,2,1,1))
  col = rgb(.1,.1,.1, .6)

  for (d in dyads) {
    cc = cContext[,d]
    plot(cc, type='p', ylim=c(0, max(cc,na.rm=T)+1), main=sprintf("totalcounts for %s", dyadvocab[d]), col=ifelse(cc>=1, rgb(.6,.2,.2,.8), rgb(.4,.4,.4,.8)))
    for (k in 1:Nframe) {
      # ylim = c(-1,1) * 20
      ylim=c(0,1)
      type='p'
      plot(x[k,,d], ylim=ylim, type=type, main=sprintf("k=%d %s", k, dyadvocab[d]),
           cex=.7, col=col
           )
      sds = sqrt(v[k,,d])
      segments(x0=1:739, y0=x[k,,d]-sds, y1=x[k,,d]+sds, col=col)
    }
  }
}

