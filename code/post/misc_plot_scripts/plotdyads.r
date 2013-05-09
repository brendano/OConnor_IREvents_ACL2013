# base="timescale.2.out"
base="fixy.K=20.trim"
# base="out.200:200"

dyadvocab=readLines(sprintf("%s/dyad.vocab",base))
D=length(dyadvocab)
T=739

ps=lapply(
          seq(100,1200,100),
          function(i) array(scan(sprintf("%s/model.%d.dyadTimeScales",base,i)),c(1,T,D))[,,])
grandmean = mean(sapply(ps, mean))
grandsd = mean(sapply(ps, function(x) sd(as.vector(x))))  ## um that's not quite right, meh

upto = function(x, limit) {
  if (length(x) <= limit)
    x
  else
    sample(x, limit)
}


# dyads = upto(1:D,36)
dyads = 1:36

par(mfrow=c(6,6), mar=c(3,2,1,1))
# for (d in upto(grep("PAL|ISR",dyadvocab),25)) {  
# for (d in upto(grep(" PAL ",dyadvocab),25)) {  
for (d in dyads) {
  plot.new()
  plot.window(xlim=c(0,T), ylim=3*grandsd*c(-1,1)+grandmean)
  title(main=sprintf("%d %s",d,dyadvocab[d]))
  axis(1);axis(2)
  for (pp in ps) lines(pp[,d], col='gray')
  postmean = colMeans(laply(ps, function(pp) pp[,d]))
  lines(postmean, col='blue')

  # acf(ps[[1]][,d], type='cov')
}

