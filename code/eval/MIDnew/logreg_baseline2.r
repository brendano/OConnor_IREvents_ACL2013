source("functions.r")
# doesnt matter which model, load only for datefile and vocab
m=load_model_basics("~/ptab/bl/251741.v7.sf.k=3.c=1.out")
d = read.tsv("~/ptab/data/v7.pathfil.dthresh=500.pthresh=10")

mid=load_mids()
mm=load_mids_against_datefile(mid,"../datefile.week.v7",big_dyad_list)

contextvocab = sprintf("%s %s %s", m$contextvocab$t, m$contextvocab$s, m$contextvocab$r)
inds=match(d$V3, m$datefile$day)
timestamps=m$datefile$t[inds]

event_ctx = sprintf("%s %s %s", timestamps, d$V4, d$V5)
aggtup=aggregate(rep(1,nrow(d)), list(event_ctx, path=d$V6), sum)

aggtup[,1] = factor(aggtup[,1], levels=contextvocab)
aggtup[,2] = factor(aggtup[,2], levels=m$pathvocab)

## rows are unique CONTEXTS, though NOT aligned with contextvocab.
## columns are PATH TYPES.
## values are counts. make them probs.

allx_ctx = sparseMatrix(as.integer(aggtup[,1]), as.integer(aggtup[,2]), x=aggtup[,3])
allx_ctx = allx_ctx / rowSums(allx_ctx)   # row normalize

meltmid=melt(mm)
ctx_mid=sprintf("%s %s", meltmid$Var2, meltmid$Var1)
inds = match(ctx_mid, contextvocab)
# allx_midalign = sparseMatrix(i=NULL,j=NULL,dims=c(nrow(meltmid), ncol(allx_aggtup)), giveCsparse=FALSE)
allx_midalign = Matrix(0, nrow=nrow(meltmid), ncol=ncol(allx_ctx), sparse=TRUE)
allx_midalign = as(allx_midalign, 'dgTMatrix')

z=data.frame(new_i=1:length(inds), old_i=inds); z=z[!is.na(inds),]
allx_midalign[z$new_i,] = allx_ctx[z$old_i,]

ally = meltmid$value >= 4

  dt = m$dates$leftday
  splitpoint  = ymd("1993-01-01") + ((ymd("2002-01-01") - ymd("1993-01-01")) / 2)
  splitpoint2 = ymd("1993-01-01") + ((ymd("2002-01-01") - ymd("1993-01-01")) / 4)
  trainspan = dt >= ymd("1993-01-01") & dt < splitpoint
  testspan =  dt >= splitpoint & dt < ymd("2002-01-01")
  tunespan = dt >= splitpoint2 & dt < splitpoint

  ctrainspan = meltmid$Var2 %in% which(trainspan)
  ctestspan =  meltmid$Var2 %in% which(testspan)
  ctunespan =  meltmid$Var2 %in% which(tunespan)

## Tuning run
trX = allx_midalign[ctrainspan & !ctunespan,]
trY =          ally[ctrainspan & !ctunespan]
g = timeit(  glmnet(trX,trY,family='binomial', weights=makeweights(trY))  )

teX = allx_midalign[ctrainspan & ctunespan,]
teY =          ally[ctrainspan & ctunespan]
p = predict(g,teX, type='response')

lls = sapply(1:ncol(p), function(i) {
        pmean = p[,i]
        # weight the held-out LL too.
        trainmean = mean(trY)
        sum(teY*log(pmean))/trainmean + sum((1-teY)*(log(1-pmean)))/(1-trainmean)
})
best_lambda = g$lambda[which.max(lls)]

## Testing run
trX = allx_midalign[ctrainspan,]
trY =          ally[ctrainspan]
g = timeit(  glmnet(trX,trY,family='binomial', weights=makeweights(trY))  )

teX = allx_midalign[ctestspan,]
teY =          ally[ctestspan]
p = predict(g,teX, type='response', s=best_lambda)

cat("AUC\n")
print(calc_auc(p[,1], teY))

closest=which.min( abs(g$lambda-best_lambda) )
x=g$beta[,closest]
cat("df ")
print(sum(x != 0))

coefs = data.frame(coef=x[x != 0], path=m$pathvocab_nice[x != 0])
arrange(coefs, coef)

