library(countrycode)
library(lubridate)
library(ROCR)
library(stringr)
library(glmnet)
library(plyr)
library(reshape2)

timeit = function(expr, name=NULL) {
  # print how long the expression takes, and return its value too.
  # So you can interpose   timeit({ blabla })   around any chunk of code "blabla".
  start = Sys.time()
  ret = eval(expr)
  finish = Sys.time()
  if (!is.null(name)) cat(name,": ")
  print(finish-start)
  invisible(ret)
}


read.tsv = function(..., header=F, sep='\t', quote='', comment='', na.strings='', stringsAsFactors=FALSE) {
  # read.table() wrapper with default settings for no-nonsense, pure TSV
  # Typical use case is output from another program.
  # (R's defaults are more geared for human-readable datafiles, which is less
  # feasible for large-scale data anyway.)
  # These options are substantially faster than read.table() defaults.
  #   (see e.g. LINK)
  # stringsAsFactors is the devil.

  args = list(...)
  args$header = header
  if (!is.null(args$col.names)) {
    # read.delim() is not smart about this.  Yikes.
    args$header = FALSE
  }
  args$sep = sep
  args$quote = quote
  args$comment = comment
  args$stringsAsFactors = stringsAsFactors
  args$na.strings = na.strings
  do.call(read.delim, args)
}

bgrep = function(pat,x, ...) {
  # "boolean" grep: return a logical vector ready for vector ops
  # like & |  and others
  unwhich(grep(pat,x,...), length(x))
}


fixpath = function(filename) {
  # Take a forward-slashed path, and fix it for windows if necessary.
  if (Sys.info()["sysname"]=="Windows") {
    filename = gsub("/", "\\\\", filename)
  }
  filename
}
  
    

##############################################

convert_cow_countrycode_to_iso = function(mid_code_numbers) {
# countrycode() only does a subset.
# consult dict/countrylist.txt (from TABARI CountryInfo)
# and "COW State list.xls"  (from COW) for rest
  x = countrycode(mid_code_numbers, "cown", "iso3c")
  x[mid_code_numbers==345] <- "SRB"
  x[mid_code_numbers==731] <- "PRK"
  x[mid_code_numbers==816] <- "DRV"  # 816 is Vietnam. DRV
  x
}

load_mids = function() {
  mid <- read.csv("MIDDyadic_v3.10.csv")

  mid$sidea <- convert_cow_countrycode_to_iso(mid$CCodeA)
  mid$sideb <- convert_cow_countrycode_to_iso(mid$CCodeB)

# dyads <- cbind(mid$sidea, mid$sideb)
# dyads <- rbind(dyads[,1:2],dyads[,2:1])
# dir.dyads <- unique(dyads)

  mid$StDay[mid$StDay==-9] <- 1
  mid$EndDay[mid$EndDay==-9] <- 28 # note I use 28th
  mid$start <- ymd(paste(mid$StYear, mid$StMon, mid$StDay))
  mid$end<- ymd(paste(mid$EndYear, mid$EndMon, mid$EndDay))
  mid$interval <- new_interval(mid$start, mid$end)
  mid$dyads <- paste(as.character(mid$sidea), as.character(mid$sideb))
  mid
}

align_loaded_mids_against_contexts = function(mids, model) {
  # return a vector, same length as the contextvocab.

  mid_melt = melt(mids)
  ## (t,s,r) strings
  mid_ctx = sprintf("%s %s", mid_melt$Var2, mid_melt$Var1)
  contextvocab = sprintf("%s %s %s", model$contextvocab$t, model$contextvocab$s, model$contextvocab$r)
  inds = match(contextvocab, mid_ctx)
  mid_melt$value[inds]
}

load_mids_against_datefile = function(mid,  # from load_mids()
                                      datefilename,
                                      dyad_strings=c("USA IRQ","IRQ USA") ) {

## Create the dense matrix of MID values for every (dyad, timestep)
## according to the model's notion of timestep intervals.

# make a lubridate interval for each model timestep
  dates <- read.table(datefilename, sep="\t", col.names=c('t','day','bucketid','leftday'), stringsAsFactors=FALSE)

  dates = dates[,c('t','day','leftday')]
  finaldates <- c()
  c <- 1
  for (i in unique(dates$t)) {
    mini <- dates[dates$t==i,]
    finaldates[c] <- as.character(mini[nrow(mini), "day"])
    c <- c+1
  }
  dates <- dates[,c('t','leftday')]
  dates <- unique(dates)
  dates$t <- dates$t+1

  colnames(dates) <- c("TimeIndex", "Date")
  dates$enddate <- finaldates
  event.interval <- new_interval(ymd(dates$Date), ymd(dates$enddate))
  ## these are intervals for each model timestep

#Select MIDs
  eventdyads <- strsplit(dyad_strings, split=" ")
  eventdyads <- matrix(unlist(eventdyads), ncol=2, byrow=TRUE)
  mid <- mid[mid$dyads %in% dyad_strings,]

 #For each add the MIDs for both directions of the dyad.
  midvalues <- matrix(0, nrow=length(dyad_strings), ncol=nrow(dates))
  for (i in 1:nrow(midvalues)) {
    sidea <- mid$sidea
    sideb <- mid$sideb
    index <- which(sidea==eventdyads[i,1] & sideb==eventdyads[i,2])  
    #loop over the values to check time conditions 
    if (length(index) > 0) {
      for (j in 1:length(index)) {
        # event.interval contains the intervals for each time point
        mid.timeframe <- mid$interval[index[j]]
        # check mid interval against each time point
        mid.periods <- int_overlaps(event.interval, mid.timeframe)
        midvalues[i,which(mid.periods)] <- pmax(mid$HostlevA[index[j]] + 1,midvalues[i,which(mid.periods)])
      }
    }
    #Repeat for Side B to Side A
    index <- which(sidea==eventdyads[i,2] & sideb==eventdyads[i,1])  
    if (length(index) > 0) {
      #loop over the values to check time conditions 
      for (j in 1:length(index)) {
        # event.interval contains the intervals for each time point
        mid.timeframe <- mid$interval[index[j]]
        # check mid interval against each time point
        mid.periods <- int_overlaps(event.interval, mid.timeframe)
        midvalues[i,which(mid.periods)] <- pmax(mid$HostlevB[index[j]] + 1,midvalues[i,which(mid.periods)])
      }
    }
  }
  rownames(midvalues) <- dyad_strings
  midvalues
}

##########################################

# Functions for model loading & more-loading-of-stuff
# Naming convention: 'model' is always a ptab_model object.


load_model_basics = function(modeldir) {

## Load the general metainformation for a modeling run, like
## the datefile, context, dyad, path vocabularies, number of frames.
## But don't load any specific variable samples.

## also, convert to 1-based indexing.
## And do other useful R-ifications here, like date convertion

  datefile = sprintf("%s/datefile", modeldir)
  datefile = read.tsv(fixpath(datefile), col.names=c('t','day','bucketid','leftday'))
  datefile$t = datefile$t + 1
  datefile$leftday = ymd(datefile$leftday)

  dates = unique(datefile[,c('t','leftday')])

  dyadvocab = read.tsv(sprintf("%s/dyad.vocab", modeldir), sep=" ", col.names=c('s','r','snum','rnum'))
  dyadvocab$snum = dyadvocab$snum + 1
  dyadvocab$rnum = dyadvocab$rnum + 1
  dyadvocab$string = sprintf("%s %s", dyadvocab$s, dyadvocab$r)

  if (file.exists(fixpath(sprintf("%s/path.vocab.nice", modeldir)))) {
    pathvocab_nice = readLines(fixpath(sprintf("%s/path.vocab.nice", modeldir)))
  } else {
    pathvocab_nice = NULL
  }
  pathvocab = readLines(fixpath(sprintf("%s/path.vocab", modeldir)))

  contextvocab = read.tsv(fixpath(sprintf("%s/context.vocab", modeldir)), sep=" ", col.names=c('t','s','r'))
  contextvocab$t = contextvocab$t + 1

  m = list(datefile=datefile, dates=dates,
        dyadvocab=dyadvocab,
        pathvocab=pathvocab,
        pathvocab_nice = pathvocab_nice,
        contextvocab=contextvocab,
        modeldir=modeldir,
        Nframe=length(scan(Sys.glob(fixpath(sprintf("%s/model.*.cFrame", modeldir)))[1])),
        Ntime=max(datefile$t), Ndyad=nrow(dyadvocab), Npath=length(pathvocab))
  class(m) = 'ptab_model'
  m
}

print.ptab_model = function(model) {
# This looks a lot nicer than the default way list() is viewed
  str(model)
}

load_sparse_eta = function(model, modeliter, dyad_strings=c("USA IRQ","IRQ USA")) {
  ## Only load the selected dyads

  ret = array(NA, dim=c(model$Nframe-1, model$Ntime, length(dyad_strings)))
  dimnames(ret) = list(NULL,NULL,dyad_strings)

  sparse_etas = sprintf("%s/model.%d.etas", model$modeldir, modeliter)
  sparse_etas = as.matrix(read.tsv(sparse_etas, sep=" "))
  ds = sprintf("%s %s", model$contextvocab$s, model$contextvocab$r)
  context_inds = which(ds %in% dyad_strings)
  for (c in context_inds) {
    ret[,model$contextvocab$t[c],ds[c]] = sparse_etas[c,]
  }
  ret
}

softmax_eta = function(eta) {
  x = array(dim=c(dim(eta)[1]+1, dim(eta)[2], dim(eta)[3]))
  x[1:dim(eta)[1],,] = eta
  x[dim(x)[1],,] = 0
  e = exp(x)
  e[ e==Inf ] = 1e100
  e[ e==-Inf] = -1e100
  Z = apply(e, 2:3, sum)
  ret = sweep(e, 2:3, Z, '/')
  dimnames(ret) = dimnames(eta)
  ret
}

load_dense_eta = function(model, modeliter) {
  f = sprintf("%s/model.%d.denseEtas", model$modeldir, modeliter)
  x = scan(f)
  x = array(x, dim=c(model$Nframe-1, model$Ntime, model$Ndyad))
  x
}

load_dense_theta = function(model, modeliter) {
  f = sprintf("%s/model.%d.denseThetas", model$modeldir, modeliter)
  x = scan(f)
  x = array(x, 
            dim=c(model$Nframe, model$Ntime, model$Ndyad),
            dimnames=list(NULL,NULL,model$dyadvocab$string)
            )
  x
}

uses_dense_etatheta = function(model) {
  length(Sys.glob(sprintf("%s/model.*.denseEtas", model$modeldir))) >= 1
}

load_theta = function(model, modeliter, dyads) {
  if (uses_dense_etatheta(model)) {
    cat("dense theta load\n")
    theta = load_dense_theta(model, modeliter)[,,dyads, drop=FALSE]
    names(dimnames(theta)) = c('frame','time','dyad')
    theta
  } else {
    cat("sparse eta load & theta calc\n")
    eta = load_sparse_eta(model, modeliter, dyads)
    # eta = add_interpolation(eta)
    theta= softmax_eta(eta)
    theta[is.na(theta)] = 0
    names(dimnames(theta)) = c('frame','time','dyad')
    theta
  }
}

load_topics = function(model, modeliter) {
  load_matrix(model, modeliter, "cPathFrame")
}

load_matrix = function(model, modeliter, variablename) {
  f = sprintf("%s/model.%d.%s", model$modeldir, modeliter, variablename)
  read.tsv(f,sep=" ")
}

get_last_iteration = function(model) {
  files = Sys.glob(sprintf("%s/model.*.cFrame", model$modeldir))
  iters = str_extract(files, "model\\.[0-9]+")
  iters = gsub("[^0-9]","",iters)
  max(as.integer(iters))
}


load_many_samples = function(model, load_fn, start_modeliter=9000, end_modeliter=1e12, step=100) {
  end_modeliter = min(end_modeliter, get_last_iteration(model))
  iters = seq(start_modeliter, end_modeliter, step)
  print(iters)
  lapply(iters, function(i) load_fn(model=model, modeliter=i))
}

## Convention: the undirected dyad identifiers are always alphabetical reversesorted:
## the actor code later in the alphabet is first.
## Please be careful to never use factors, always use character strings.  I'm
## afraid factors will silently screw all this up.

get_undir_dyad_strings = function(dir_dyad_strings) {
## created the undirected dyad identifiers for a directed dyad
## e.g. "USA IRQ" and "IRQ USA" both go to "USA IRQ"
  dyad_pairs = strsplit(dir_dyad_strings, " ")
  s = sapply(dyad_pairs, function(p) p[1])
  r = sapply(dyad_pairs, function(p) p[2])
  ifelse(s>r, sprintf("%s %s",s,r), sprintf("%s %s", r,s))
}

expand_dyad_dirs = function(undir_dyads) {
## Take undirected dyads, and give all the corresponding directed dyads:
## twice as many.
  ret = c()
  d = as.data.frame(matrix(unlist(strsplit(undir_dyads," ")), byrow=TRUE,ncol=2))
  for (i in 1:nrow(d)) {
    ret[2*(i-1)+1] = paste(d$V1[i], d$V2[i])
    ret[2*(i-1)+2] = paste(d$V2[i], d$V1[i])
  }
  ret
}



###################################

## dyad-level evaluation

## old code has 'dyadthetas' a list of matrixes, one per dyad
## newer code has 'dyadthetas' a 3d array, the last dimension is for the dyad


add_interpolation = function(dense_eta) {
  # 3d array
  numtime = dim(dense_eta)[2]
  for (k in 1:dim(dense_eta)[1]) {
    for (dyad in 1:dim(dense_eta)[3]) {
      x = dense_eta[k,,dyad]
      finite = which(!is.na(x))
      firstval = x[finite[1]]
      lastval  = x[finite[ length(finite)-1 ]]
      x = approxfun(x, yleft=firstval, yright=lastval)(1:numtime)
      dense_eta[k,,dyad] = x
    }
  }
  dense_eta
}

calc_auc = function(pred, labels) {
  # from the ROCR library
  # 'pred' are confidence scores (i think any real numbers are fine)
  # 'labels' are binary labels, i think it has to be 0's and 1's ?
  performance(prediction(pred, labels), measure='auc')@y.values[[1]]
}

load_results = function(csvfile) {
  ## load a results file created earlier by eval_models(),
  ## handle the naming conventions we're using on the blacklight runs,
  ## and melt() it for ggplot-friendliness

  d = read.csv(csvfile, stringsAsFactors=FALSE)
  d = cbind(k=as.integer(gsub("k=","", str_extract(d$modelname, "k=[0-9]+"))),
            model=str_extract(d$modelname, "\\.(v|sf)\\."),
            d)
  x = melt(d, id.vars=1:4, measure.vars=5:ncol(d))
  x
}
load_results2 = function(logfile) {
  ## load a results file created earlier by eval_models(),
  ## handle the naming conventions we're using on the blacklight runs,
  ## and melt() it for ggplot-friendliness

  d = read.tsv(pipe(sprintf("grep -h AUC %s", logfile)))
  if (all(d[,1]=="AUC")) d[,1] = NULL
  names(d)=c('modelname','auc')
  d = data.frame(result_config(d$modelname), d)
  d = unique(d)
  x = melt(d, id.vars=1:3, measure.vars=4:ncol(d))
  x
}

load_scale_results = function(logfile) {
  d = read.tsv(pipe(sprintf("grep IMPURITY %s",logfile)))
  d[,1] = NULL
  names(d)=c('modelname','impur')
  d = data.frame(result_config(d$modelname), d)
  x = melt(d, id.vars=1:3, measure.vars=4:ncol(d))
  x
}



result_config = function(modelname) {
  d = data.frame(k=as.integer(gsub("k=","", str_extract(modelname, "k=[0-9]+"))),
            model=str_extract(modelname, "\\.(v|sf)\\."))
  d$model = gsub("\\.", "", d$model)
  d
}



################################

## a few plotting routines

plot_manytheta = function(denselist, frame, dyad, model) {
## 'denselist' is a list of 3d arrays. each is a 3d dense version of 'theta'.
## each is one sample.  plot all of them, for the given frame/dyad pair.

  plot.new()
  daterange = range(model$dates$leftday)

  plot.window(xlim=daterange, ylim=c(0,1))
  axis(2)
  box()
  dseq = seq(daterange[1], daterange[2] + years(1), by='year')
  axis(1, at=dseq, labels=year(dseq))

  for (i in 1:length(denselist)) {
    x = denselist[[i]]
    lines(model$dates$leftday, x[frame, ,dyad], col=rgb(.8,.8,.8,.5))
  }
  avg_timeseries = rowMeans(sapply(denselist, function(x) x[frame,,dyad]))
  lines(model$dates$leftday, avg_timeseries, col=rgb(.2,.2,.9,.9))
}

uncertainty_plot = function(thetas,m, dyadstr) {

## Usage:
## m=load_model_basics("../../bl/251192.v7.sf.k=3.c=1.out")
## x=load_many_samples(m,load_dense_theta,9000)
## uncertainty_plot(x,m,'ISR','PSE')

# dyad_time_counts=read.tsv("../../data/v7.pathfil.dthresh=500.pthresh=10.dyad_time_counts",sep=" ",col.names=c('count','leftday','s','r'))
# x=lapply(seq(9000,10000,100),function(i) load_dense_theta(m,i))

  topics = load_topics(m, 9000)

  # par(mfrow=c(m$Nframe, 1),mar=c(2,3,3,1))

  # cc=subset(dyad_time_counts, s==src & r==rec)
  # plot(ymd(cc$leftday), cc$count,log='y', xlim=range(m$dates$leftday), type='h', main=sprintf("%s -> %s counts", src, rec))

  for (k in 1:m$Nframe) {
    plot_manytheta(thetas, k, dyadstr, m)
    topwords = order(-topics[,k])[1:10]
    topwords = paste(m$pathvocab[topwords], collapse=', ')
    title(main=sprintf("k=%d  ||  %s", k, topwords) )
  }
}

######################################################


make_XY = function(thetas, midvalues, timespan, dyadmask=NULL) {
# 'thetas' is a list of 3d arrays (one sample each)
# 'midvalues' is a matrix
# 'timespan' is a boolean mask over the *full* timespan
# 'dyadmask' is a boolean mask over the rows of midvalues

  if (is.null(dyadmask))  dyadmask = rep(TRUE, nrow(midvalues))

  stopifnot(length(timespan)==ncol(midvalues))
  stopifnot(length(dyadmask)==nrow(midvalues))

  nsamp = length(thetas)

  xmats = lapply(1:nsamp, function(s) {
    melted = melt(thetas[[s]][, timespan, dyadmask])
    xmat = acast(melted, dyad+time ~ frame)
    xmat
  })
  bigX = do.call(rbind, xmats)

  ## bigX has time varying the fastest, then dyad.  (then sample.)
  ## midvalues is (dyad, time)
  ## so transpose it before vectorifying
  ## because (time,dyad) in column-major order has time varying fastest

  yvec = as.vector(t(midvalues[dyadmask,timespan] >= 4))
  bigY = rep(yvec, nsamp)  ## sample varies *slowest*

  list(X=bigX, Y=bigY, oneY=yvec, ntime=sum(timespan))
}

random_split = function(n, nfold) {
  x = rep(1:nfold, n)[1:n]
  x[order(runif(n))]
}

makeweights = function(Y) {
  # 'Y' is binary variable.  create weights to balanced the data,
  # and their sum equals N.
  p = mean(Y)
  0.5 * 1 / ifelse(Y, p, 1-p)
}

eval_timesplit_multisample = function(mid, model, mydyads, si, ei) {
  d = model$dates$leftday

  splitpoint  = ymd("1993-01-01") + ((ymd("2002-01-01") - ymd("1993-01-01")) / 2)
  splitpoint2 = ymd("1993-01-01") + ((ymd("2002-01-01") - ymd("1993-01-01")) / 4)

  trainspan = d >= ymd("1993-01-01") & d < splitpoint
  testspan =  d >= splitpoint & d < ymd("2002-01-01")
  tunespan = d >= splitpoint2 & d < splitpoint

  cat("Load data from disk\n")
  mm = load_mids_against_datefile(mid,"../datefile.week.v7",mydyads)
  tt = timeit(  lapply(seq(si,ei,100),function(i) load_theta(model, i, mydyads))  )

  ## the directed dyads line up.
  stopifnot(all(dimnames(tt[[1]])[[3]] == row.names(mm)))

  nsamp = length(tt)

## lambda selection
  cat("Load tuning data\n")
  tr = make_XY(tt, mm, trainspan & !tunespan)
  te = make_XY(tt, mm, tunespan)
  cat("Train tuning model\n")
  g = timeit(  glmnet(tr$X, tr$Y, family='binomial', weights=makeweights(tr$Y))  )
  p = predict(g, te$X, type='response')
  # aucs = sapply(1:ncol(p), function(i) {
  #        pmean = rowMeans(matrix(p[,i], ncol=nsamp))
  #        calc_auc(pmean, te$oneY)
  # })
  lls = sapply(1:ncol(p), function(i) {
         pmean = rowMeans(matrix(p[,i], ncol=nsamp))
         # weight the held-out LL too.
         trainmean = mean(tr$Y)
         sum(te$oneY*log(pmean))/trainmean + sum((1-te$oneY)*(log(1-pmean)))/(1-trainmean)
  })


  best_lambda = g$lambda[which.max(lls)]
  cat("Tuning path analysis\n")
  print(data.frame(g$lambda, g$df, g$dev.ratio, lls, chosen=ifelse(lls==max(lls), " *** ","")))


## Real run
  cat("Load full data\n")
  tr=make_XY(tt,mm,trainspan)
  te=make_XY(tt,mm,testspan)

  cat("Train full model\n")
  g= timeit(  glmnet(tr$X,tr$Y,family='binomial', weights=makeweights(tr$Y))  )

  cat("Full model info\n")
  closest = which.min( abs(g$lambda - best_lambda) )
  cat(sprintf("df %s\n", g$df[closest]))
  cat(sprintf("coefs %s\n", paste( sprintf("%.4g",g$beta[,closest]), collapse=' ')))
  p=predict(g,te$X,type='response',lambda=best_lambda)
  pmean=rowMeans(matrix(p[,ncol(p)], ncol=nsamp))

  x = row.names(p)[1:(nrow(p)/nsamp)]
  list(auc=calc_auc(pmean, te$oneY),
       pred= data.frame(
         dyad = gsub("_.*","",x), 
         t = model$dates$leftday[which(testspan)[as.integer(gsub(".*_","",x))]], 
         pmean = pmean))
}


do_evals = function(modeldirs, evaller=eval_dyadsplit_multisample, startiter=9000, enditer=10000) {
  mid = load_mids()
  for (modeldir in modeldirs) {
    m = load_model_basics(modeldir)
    x = evaller(mid, m, big_dyad_list, startiter, enditer)
    cat(sprintf("AUC\t%s\t%f\n", modeldir, x$auc))
  }
}


big_dyad_list = c("DEU FRA", "JPN USA", "FRA GBR", "USA EGY", "CHN USA", "RUS DEU", 
"FRA BIH", "PAK IND", "KOR PRK", "IND USA", "USA IND", "KOR USA", 
"ISR SYR", "RUS CHN", "USA FRA", "USA GBR", "USA VNM", "USA SYR", 
"RUS UKR", "USA RUS", "IDN TMP", "IDN USA", "GRC MKD", "RUS USA", 
"USA CHN", "CHN RUS", "HKG CHN", "USA AUS", "ISR LBN", "SYR ISR", 
"DEU RUS", "USA SOM", "RUS SRB", "RUS BIH", "USA ISR", "RUS GBR", 
"PRK JPN", "VNM USA", "IRN USA", "THA USA", "EGY USA", "JPN PRK", 
"ISR JOR", "TUR GRC", "USA PAK", "RUS FRA", "GBR BIH", "GBR FRA", 
"USA PRK", "SDN UGA", "USA DEU", "ISR USA", "IRQ USA", "IRN GBR", 
"USA UKR", "KOR CHN", "FRA RUS", "FRA RWA", "GBR USA", "USA KOR", 
"HRV BIH", "GBR RUS", "GRC TUR", "SYR USA", "TUR IRQ", "TWN CHN", 
"CHN TWN", "RUS TJK", "GBR ISR", "PAK USA", "USA CAN", "RUS ISR", 
"CUB USA", "USA CUB", "UKR RUS", "AUS USA", "TMP IDN", "CHN FRA", 
"LBN ISR", "RUS AFG", "USA THA", "PRK USA", "USA JPN", "GBR IRN", 
"PRK KOR", "IRQ ISR", "EGY SDN", "IND RUS", "GEO RUS", "USA TWN", 
"USA HTI", "AUS CHN", "IND GBR", "EGY ISR", "RUS IND", "IRN AFG", 
"USA LBN", "JOR ISR", "RUS PRK", "JPN KOR", "CHN MMR", "FRA USA", 
"LBN USA", "SRB BIH", "GBR CHN", "USA TUR", "GBR IND", "GBR PAK", 
"JPN CHN", "GBR DEU", "IRN IRQ", "CHN PRK", "PRK CHN", "USA MMR", 
"MMR THA", "USA ITA", "DEU ISR", "USA BIH", "IND PAK", "IRN RUS", 
"RUS GEO", "IRN ISR", "SRB HRV", "PAK AFG", "RUS JPN", "JPN RUS", 
"MEX USA", "IRQ TUR", "KOR JPN", "USA IRQ", "DEU IRQ", "ITA IRQ", 
"CHN JPN", "USA IRN", "THA KHM", "LBN SYR", "CHN KOR", "IRQ SAU", 
"BIH SRB", "FRA CIV", "FRA IRQ", "IRN LBN", "CHN DEU", "ISR IRN", 
"CHN VNM", "SAU IRQ", "JPN IRQ", "CHN IND", "KWT IRQ", "USA HRV", 
"ISR EGY", "USA SRB", "USA JOR", "CAN USA", "TUR USA", "IRQ KWT", 
"USA COL", "FRA DZA", "SAU ISR", "COL USA", "USA NGA", "CHN PAK", 
"JOR USA", "USA POL", "TWN USA", "USA IDN", "FRA TUR", "IND CHN", 
"KWT USA", "THA MMR", "RUS IRN", "TUR IRN", "USA PHL", "POL RUS", 
"HRV SRB", "AUS IDN", "AFG PAK", "SYR LBN", "IRQ IRN", "JPN TWN", 
"JOR IRQ", "SAU USA", "USA SDN", "IRQ JOR", "USA VEN", "GRC CYP", 
"USA LBY", "FRA ISR", "CHN HKG", "LBY USA", "PAK GBR", "RUS POL", 
"USA SAU", "IRN TUR", "DEU IRN", "IRN FRA", "FRA IRN", "RWA COD", 
"UGA SDN", "ISR IRQ", "DEU USA", "PHL USA", "RUS IRQ", "CHN IRQ", 
"BLR RUS", "PHL CHN", "PAK CHN", "BIH HRV", "GBR IRQ", "USA MEX", 
"USA GEO", "TUR CYP", "FRA CHN", "CHN ISR", "POL USA", "CHN GBR", 
"USA KWT", "ESP USA", "SYR IRQ", "IRQ GBR", "USA ESP", "USA CZE", 
"CHN THA", "TUR ISR", "PAK IRQ", "EGY IRQ", "FRA DEU", "MYS USA", 
"ISR TUR", "DEU TUR", "ERI ETH", "USA AFG", "CHN IRN", "ISR RUS", 
"TUR X-TURCYP", "CYP TUR", "TUR SYR", "JPN IND", "RUS BLR", "USA KEN", 
"VEN COL", "IRN DEU", "AFG USA", "POL IRQ", "CYP GRC", "ITA USA", 
"ETH SOM", "ETH ERI", "FRA LBN", "DEU CHN", "VEN USA", "BGR IRQ", 
"ALB SRB", "DEU AFG", "IRQ SYR", "FRA AFG", "PHL IRQ", "TCD SDN", 
"GBR AFG", "KOR IRQ", "ESP IRQ", "AUS IRQ", "SDN TCD", "RWA COG", 
"UGA COG", "CAN AFG", "DNK IRQ")

