library(plyr)
library(ggplot2)
theme_set(theme_bw())
library(stringr) 

readts = function(filename) {
}

readlog = function(filename, gets=c("totalLL"), scalar=TRUE, getall=FALSE) {

  # scalars, or vectors both work
  scalarfilter = "awk -vpat='^%s' '
    /^ITER/ {iter=$2}
    /^ITER/ && (iter <= 500 || iter %% 100==0) {want=1}
    want && $0~pat{ print iter, $0; want=0}
    '"
  # scalarfilter = "awk -vpat='^%s' '
  #   /^ITER/ && ($2 <= 500 || $2 %% 50 == 0) {iter=$2; want=1}
  #   want && $0~pat{ print iter, $0; want=0}
  #   '"
  # matrixes are multiline. have to remember state
  matrixfilter = "awk -vpat='^%s' '
    /^ITER/ {iter=$2}
    /^ITER/ && (iter <= 500 || iter %% 100 == 0) { want=1 }
    !want{next}
    $0~pat{m=iter \" \" $1; next}  ## start the matrix
    m && $0==\"]\" {print m; m=0; want=0}          ## end the matrix
    m{m = m \" \" $0}                      ## middle of the matrix
    '"
  
  awkfilter = if (scalar)  sprintf(scalarfilter, gets)  else  sprintf(matrixfilter, gets)

  filter = sprintf("grep -Pv 'nsubj|APW|Properties|resampling|turning off|suff stats' | %s", awkfilter)

  # greps = paste(sprintf("grep '%s'", gets), collapse=' | ')
  # cmd = sprintf("cat %s | %s | %s", filename, filter, greps)

  cmd = sprintf("cat %s | %s", filename, filter)
  cmd = sprintf("%s | perl -pe 's/[\\[\\],]/ /g'", cmd)
  cat(cmd); cat("\n")

  system(sprintf("%s > tmp", cmd))
  if (file.info("tmp")$size < 10) {
    return(NULL)
  } else {
    # d=read.table(pipe(cmd), fill=F)
    d = read.table("tmp", fill=F)
    d = data.frame(run=filename, d)
    d
  }
}

alllog = function(filename,quick=F) {
  lst = list()

  for (var in c('frameconc','pathconc')) {
    d=readlog(filename,var)
    if (!is.null(d))
      lst[[ length(lst)+1 ]] = data.frame(iter=d$V1, variable=var, value=d$V5, run=d$run)
  }
  for (var in c('wordLL','totalLL','cmLL','transVar')) {
    d=readlog(filename,var)
    if (!is.null(d))
      lst[[ length(lst)+1 ]] = data.frame(iter=d$V1, variable=var, value=d$V3, run=d$run)
  }

  if (!quick) {

    for (var in c('alpha','etaVar')) {
      d=readlog(filename, var)
      if (!is.null(d)) {
        K = ncol(d)-3
        names(d)[4:ncol(d)] = sprintf("f%d", 0:(K-1))
        for (k in 1:K) {
          ind = 3 + k
          lst[[ length(lst)+1 ]] = data.frame(iter=d$V1,
                                              variable=sprintf('%s.%s', var, names(d)[ind]),
                                              value=d[,ind],
                                              run=d$run)
        }
      }
    }
    for (var in c()) {  #c('framescales')) {
      d=readlog(filename, var, scalar=FALSE)
      if (!is.null(d)) {
        K = ncol(d)-3
        names(d)[4:ncol(d)] = sprintf("f%d", 0:(K-1))
        for (k in 1:K) {
          ind = 3 + k
          lst[[ length(lst)+1 ]] = data.frame(iter=d$V1,
                                              variable=sprintf('%s.%s', var, names(d)[ind]),
                                              value=d[,ind],
                                              run=d$run)
        }
      }
    }
  }

  d = do.call(rbind.fill, lst)
  d$h=str_match(d$variable, "(.*?)\\.f[0-9]+")[,2]
  d$f=str_extract(d$variable, "\\.f[0-9]+")
  d
}

alllogs = function(..., quick=F) {
  fileglobs = list(...)
  filenames = do.call(c, lapply(fileglobs, Sys.glob))
  do.call(rbind, lapply(filenames, function(f) alllog(f, quick)))
}
  

## BinIO readers
# readbin = function(filename, type, datum_size=NULL) {
#   # for outputs from java (specifically, fastutil's BinIO routines)
#   # ... always big-endian it seems.
#   stopifnot(type %in% c('int','double'))
#   if (is.null(datum_size)) {
#     datum_size = if (type=='int') 4 else if (type=='double') 8 else -1
#   }
#   filesize = file.info(filename)$size
#   n = filesize / datum_size
#   stopifnot(n*datum_size == filesize)
#   readBin(filename, type, n, signed=TRUE, endian='big')
# }
# 
# readmat = function(filename, type, ncol) {
# # java saves in row-major order.
# # so let's read into same shaped matrix for sanity's sake
#   vec = readbin(filename, type)
#   mat = matrix(vec, ncol=ncol, byrow=TRUE)
#   mat
# }

## TextIO output reader (yes it's lame fastutil saves a matrix as vector.)
readmat = function(filename, ncol) {
  matrix(scan(filename), ncol=ncol, byrow=TRUE)
}

readmodel = function(dirname, nFrame, iter=NULL) {
  # if (is.null(iter)) {
  #   cmd = sprintf("ls %s/model* | gsort -V | tail -1",dirname)
  #   iter = str_match(readLines(pipe(cmd)), "model\\.([0-9]+)")[,2]
  #   print(iter)
  # }
  # prefix = sprintf("%s/model.%s", dirname, iter)

  m = list()
  m$dyad_vocab = readLines(sprintf("%s/dyad.vocab", dirname))
  m$path_vocab= readLines(sprintf("%s/path.vocab", dirname))
  m$nDyad = length(m$dyad_vocab)
  m$nPath = length(m$path_vocab)

  prefix = sprintf("%s/modelavg.last", dirname)

  # m$cFrame = readbin(sprintf("%s.cFrame",prefix), 'int')
  # m$cPathFrame = readmat(sprintf("%s.cPathFrame",prefix), 'int', ncol=nFrame)
  # m$cDyadFrame = readmat(sprintf("%s.cDyadFrame",prefix), 'int', ncol=nFrame)
  m$cFrame = scan(sprintf("%s.cFrame",prefix))
  m$cPathFrame = readmat(sprintf("%s.cPathFrame",prefix), ncol=nFrame)
  m$cDyadFrame = readmat(sprintf("%s.cDyadFrame",prefix), ncol=nFrame)
  m$frameFields= readmat(sprintf("%s.frameFields",prefix), ncol=nFrame)
  m
}


traceplots = function(d, filename="out.pdf") {
  numruns = length(unique(d$run))
  pdf(filename, onefile=T, width=3+numruns*3)
  p = qplot(iter,value,geom='line',data=subset(d,!is.na(h)), colour=f, alpha=.5)+facet_grid(h~run,scales='free')
  print(p)
  p = qplot(iter,value,geom='line',data=subset(d, is.na(h)), colour=run)+facet_grid(variable~.,scales='free')
  print(p)
  # maxiter = max(d$iter)
  # p = qplot(iter,value,geom='line',data=subset(d, iter > maxiter-1000 & is.na(h)), colour=run)+facet_grid(variable~.,scales='free')
  # print(p)
  dev.off()
  if (exists('OPEN') && OPEN)
    system(sprintf("open %s", filename))
}

