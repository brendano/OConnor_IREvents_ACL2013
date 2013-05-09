source("plotcommon.r")
source("functions.r")
# d=load_scale_results("scaleeval.sample=5000")
d=load_scale_results("scaleevals/scaleeval.new2.txt")
# d=d[!bgrep("252380",d$modelname),]

dn=read.table(pipe("grep -h NULL ~/ptab/verbdict/internal_diff_null*"))
y=ddply(dn,.(V2), function(x) quantile(x$V3,.05))
d = rbind(d, data.frame(k=y[,1], model='nullhypo', modelname='bla',variable='impur', value=y[,2]))

kf = factor(d$k)
d$ki = as.integer(kf)
d$ki = d$ki + offset*(d$model=='sf') - offset*(d$model=='v')
d$model = c(nullhypo='Null', sf='Smoothed', v='Vanilla')[d$model]
d$model = factor(d$model, levels=c('Null','Vanilla','Smoothed'))

# x1=ddply(d,.(k,model),function(x) mean(x$value))
# x=ddply(dn,.(V2), function(x) smean.sd(x$V3))
# # x1=rbind(x1, data.frame(k=x[,1], model='nullhypo', V1=x$Mean-1.96*x$SD))
# x1 = rbind(x1, data.frame(k=y[,1], model='nullhypo', V1=y[,2]))


p = qplot(ki, value, colour=model, shape=model, data=d, geom='point')

p = p + geom_point(alpha=0.9)
p = p + stat_summary(fun.y=mean, fun.ymin=mean, fun.ymax=mean, geom='line', lwd=1.3, alpha=0.8)
p = p + stat_summary(fun.data='mean_cl_boot', lwd=0.5, pch='', alpha=0.8)
p = p + scale_x_continuous(breaks=1:nlevels(kf), labels=levels(kf))

# p = qplot(ki, V1, colour=model, shape=model, geom=c('point'), data=x1)
# p = p + geom_line(aes(colour=model, group=model), cex=1, alpha=.9)
# p = p + geom_point(aes(colour=model, group=model, shape=model), cex=3, alpha=.9)

p = p + scale_color_manual(values=brewer.pal(3,"Set2"))
p = p + geom_hline(y=5.3351853211, colour=brewer.pal(3,"Set2")[1])
p = p + ylab("Scale impurity\n(lower is better)\n")
p = p + xlab("Number of frames (K)")
p = p + theme(plot.margin=unit(c(0.1, 0, 0, 0.0),"in"))
print(p)
ggsave("scale_results.pdf", width=5, height=2.5)
system("cp scale_results.pdf ~/ptab/writing/newplots")
