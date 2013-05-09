# filename = "goldscore.learnboth.1.tsv"
# framescale_filename = "../eval/models/learnboth.1/mean.frameScales"
# filename = "goldscore.v3.k=40.tsv"
# framescale_filename = "../eval/models/v3.k=40.d2k.pthresh5/mean.frameScales"
filename = "goldscore.v3.k=20.tsv"
framescale_filename = "../eval/models/v3.k=20/mean.frameScales"

d = read.tsv(filename, col.names=c('pathnum','k','count','score','pathstr'))
framescales = c(scan(framescale_filename), 0)
framescales = data.frame(k=0:(length(framescales)-1), fs=framescales)

# x = ddply(d, .(k), function(x) data.frame(
#           n=nrow(x),
#           mean=weighted.mean(x$score, x$count),
#           sd=sqrt(cov.wt(matrix(x$score), x$count)$cov),
#           total=sum(x$count)))
# x = subset(x,x$total>=10)
# o = order(-x$mean)
# plot.new()
# plot.window(xlim=c(1,nrow(x)), ylim=c(-2,2))
# axis(1, at=1:nrow(x), labels=o)
# axis(2)
# segments(x0=1:nrow(x), y0=x$mean[o]-x$sd[o], y1=x$mean[o]+x$sd[o])
# segments(x0=1:nrow(x), y0=x$mean[o]-x$sd[o]/sqrt(x$total), y1=x$mean[o]+x$sd[o]/sqrt(x$total), col='blue',lwd=5)

x = ddply(d,.(k),function(x) data.frame(n=nrow(x),total=sum(x$count),
              pos=weighted.mean(x$score==1, x$count),
              neu=weighted.mean(x$score==0, x$count),
              neg=weighted.mean(x$score==-1, x$count)))
x = subset(x,x$total>=10)
o = order(x$pos)
plot(1:nrow(x), x$pos[o], col='blue', ylim=c(0,1))
p=x$pos[o]; n=x$total[o]
segments(1:nrow(x), y0=p-sqrt(p*(1-p)/n), y1=p+sqrt(p*(1-p)/n), col='blue')
points(1:nrow(x), x$neu[o], col='gray')
title(main=filename)

x = join(x, framescales)
plot(x$fs, x$pos,type='n', ylim=c(0,1)); text(x$fs,x$pos,x$k)
title(main=filename)

e = join(d, framescales)
stopifnot(nrow(e) == nrow(d))
r = sqrt(abs(cov.wt(cbind(e$fs,e$score), e$count)$cov[1,2]))
cat(sprintf("tok-level r = %.4f\n", r))

