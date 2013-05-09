#!/usr/bin/env Rscript
source("~/ptab/post/readlog.r")
args = commandArgs(trailingOnly=TRUE)
outfile = args[1]
logfiles = args[2:length(args)]
if (length(logfiles)==0) asdfasdfafds
cat(sprintf("outfile to %s", outfile))
d = alllogs(logfiles)
traceplots(d, outfile)

