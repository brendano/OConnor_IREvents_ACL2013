source("plotcommon.r")
# d=load_results2("timeeval.9k*.log")
d=load_results2("timeeval.nointerpolate*.log")
# d=d[!bgrep("252380",d$modelname),]
d = rbind(d,
  data.frame(k=c(2,100), model='lr', modelname='bla', variable='auc', value=c(.62,.62))
)
kf = factor(d$k)
d$ki = as.integer(kf)
d$ki = d$ki + offset*(d$model=='sf') - offset*(d$model=='v')

d$model = c(nullhypo='Null', lr='Log. Reg', sf='Smoothed', v='Vanilla')[d$model]
d$model = factor(d$model, levels=c('Log. Reg','Vanilla','Smoothed'))

p = qplot(ki, value, colour=model, data=d, shape=model)
# p = p + geom_hline(data=data.frame(y=.51,model='Log. Reg'), 
#                    aes(yintercept=y, colour=model, colour=model), show_guide=TRUE)
p = p + scale_color_manual(values=brewer.pal(3,"Set2"))
# p = p + geom_point(aes(alpha=0.9, shape=model))
p = p + geom_point(alpha=0.9)
p = p + stat_summary(fun.y=mean, fun.ymin=mean, fun.ymax=mean, geom='line', lwd=1.3, alpha=0.8)
p = p + stat_summary(fun.data='mean_cl_boot', lwd=0.5, pch='', alpha=0.8)

p = p + scale_x_continuous(breaks=1:nlevels(kf), labels=levels(kf))
p = p + labs(x="Number of frames (K)", y="Conflict prediction AUC\n(higher is better)\n")
p = p + theme(plot.margin=unit(c(0.1, 0, 0, 0),"in"))

print(p)
ggsave("mideval.pdf", width=5, height=3)
system("cp mideval.pdf ~/ptab/writing/newplots")
