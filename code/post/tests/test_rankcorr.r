onerep = function() {
# x=rnorm(10);y=rnorm(10);w=rep(1,10)
x=sample.int(5,10, replace=T)
y=sample.int(5,10, replace=T)
w=rep(1,10)
write.tsv(data.frame(x,y,w), file="tmp")
internal = cor(x,y, method='kendall')
ext = scan(p<-pipe("python rankcorr.py < tmp"))
print(list(internal, ext))
stopifnot( abs(internal - ext) < 1e-10 )
close(p)
}

wtest = function(x,y) {
# Use replication to test weighting
  x2 = c(x[1:3], x)
  y2 = c(y[1:3], y)
  internal = cor(x2,y2, method='kendall')
  n = length(x)
  w = c(rep(2,3), rep(1,n-3))
  write.tsv(data.frame(x,y,w), file="tmp")
  ext = scan(p<- pipe("python rankcorr.py < tmp"))

print(list(internal, ext))
stopifnot( abs(internal - ext) < 1e-10 )
close(p)
}

calcauc = function(gold,pred) {
  library(ROCR)
  rpred = prediction(pred, gold)
  performance(rpred, 'auc')@y.values[[1]]
}


n=100
x=rnorm(n)
# y=rnorm(n)
# y=rep(1:2, n/2)
# x=sample.int(2,n, replace=T)
y=sample.int(4,n, replace=T)
# y=rnorm(n)
w=rep(1,n)
write.tsv(data.frame(x,y,w), file="tmp")

internal = cor(x,y, method='kendall')
cat("R cor(method=kendall): ")
print(internal)
system("python rankcorr.py < tmp")

if (all(y %in% 1:2)) {
  cat("AUC ")
  print(calcauc(y,x))
}

