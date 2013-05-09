
eval_model = function(mid, modeldir, mydyads) {
## Evaluate a model (from modeldir).
## 'mid' is the original MID csv data
## TODO: convert to the load_model() world?

  # mydyads=c("USA IRQ", "IRQ USA", "PAK IND", "IND PAK", "PRK USA", "USA PRK", "ISR LBN", "LBN ISR", "CHN TWN", "TWN CHN", "USA CHN", "CHN USA")

#   mydyads = c(
# "USA IRQ", "IRQ USA", 
# "PAK IND", "IND PAK", 
# "PRK USA", "USA PRK", 
# # "ISR LBN", "LBN ISR", # it's all 1's? maybe i have the daterange wrong though.
# "CHN TWN", "TWN CHN", 
# "USA CHN", "CHN USA", 
# "KOR PRK", "PRK KOR", 
# "RUS USA", "USA RUS", 
# "ISR SYR", "SYR ISR", 
# "TUR IRQ", "IRQ TUR", 
# "TUR GRC", "GRC TUR"
# # "ALB SRB", "SRB ALB"   ## not in v7
# )

  stop("thinks this is bad code now")

  # mydyads = expand_dyad_dirs(mydyads)

  mids = load_mids_against_datefile(mid, file.path(modeldir, "datefile"), mydyads)
                                  # "../../runs/v7.sf.k=10.1.out/datefile", mydyads)
  
  cat("\n")
  print(rowSums(mids >= 4))   # 4="Use of Force", 5="War"
  model = load_model_basics(modeldir)
  alltheta = load_dense_theta(model, 5000)

  ## old sparse loading code broken ATM
  # dyadthetas = load_dyadthetas_from_sparse(modeldir, mids, dyad_strings=mydyads)
  # dyadthetas = add_interpolation(dyadthetas)

  # TODO what is the correct date span?
  goodspan = model$dates$leftday >= ymd("1994-10-01") & model$dates$leftday <= ymd("2002-01-01")
  mids = mids[,goodspan]
  dyadthetas = alltheta[,goodspan,mydyads]
  dyadloo_predeval(mids, dyadthetas)
}

eval_models = function(modeldirs, output_file) {
  stop("OLD CODE")
## Do all the conflict prediction evaluations for many models,
## and save results to a csv file.

  # modeldirs = readLines(modeldirs_filename)
  mid = load_mids()
  r = ldply(modeldirs, function(f) eval_model(mid,f), .progress='text')
  d = data.frame(modelname=modeldirs, r)
  write.csv(d, file=output_file, row.names=FALSE)

  cat(sprintf("Saved to %s\n\n", output_file))
  print(d)
}


holdout_predeval = function(train_theta, train_mid, test_theta, test_mid, ...) {

  stop("OLD CODE")

## Do dyad-holdout model training and evaluation.

# train_theta is a 3d array
# train_mid is matrix (dyad x time)
# similarly for test

# Give it a single 'lambda' to get simple fit & predictions.
# Without 'lambda', it will do glmnet's default path,
# and pick the best on the test set.  So be careful to not cheat!!

  # Flatten out so dyad-time combinations are rows, frames are columns
  trainX = t(do.call(cbind, lapply(1:dim(train_theta)[3], function(dnum) train_theta[,,dnum])))
  testX  = t(do.call(cbind, lapply(1:dim(test_theta)[3],  function(dnum) test_theta[,,dnum])))
  trainY = as.integer(as.vector(t(train_mid)) >= 3)
  testY  = as.integer(as.vector(t(test_mid)) >= 3)
  stopifnot(sum(testY)>0 && sum(trainY) > 0)

  m = glmnet(trainX, trainY, family='binomial', alpha=0, ...)
  predprob = predict(m, testX, type='response')
  path_stats = ldply(1:length(m$lambda), function(itr) {
                p = predprob[,itr]
                p = ifelse(p==1, 1 - 1e-5, ifelse(p==0, 1e-5, p))
                data.frame(ll=sum(log(p[testY==1])) + sum(log(1-p[testY==0])),
                           acc=mean( (p>0.5) == testY ))
  })
  best_i = which.max(path_stats$ll)
  best_lambda = m$lambda[best_i]
  # plot((m$lambda), path_stats$ll)
  list(best_lambda=best_lambda, predprob=predprob[,best_i], model=m)
}


dyadloo_predeval = function(midvalues, dyadthetas, ...) {

  stop("OLD CODE")

# 'midvalues' is  output from load_mids_against_datefile()
# has MID values against our notion of time

## do the entire Dyad-LOO predictive evaluation
## this calls holdout_predeval() as a subroutine

  dir_dyads = dimnames(dyadthetas)[[3]]
  ndyad = dim(dyadthetas)[3]
  undir_dyads = get_undir_dyad_strings(dir_dyads)

  uniq_undir_dyads = unique(undir_dyads)
  n_undir = length(uniq_undir_dyads)
  stopifnot(n_undir == ndyad/2)

  loo_aucs = ldply(uniq_undir_dyads, function(holdout) {
    cat(sprintf("holdout %s\n", holdout))
    test = which(undir_dyads == holdout)
    train= which(undir_dyads != holdout)

    # train1 = sample(train, floor(length(train)/2))
    # train2 = setdiff(train, train1)
    # tuning_fit = holdout_predeval(dyadthetas[,,train1], midvalues[train1,], dyadthetas[,,train2], midvalues[train2,])

    final_fit  = holdout_predeval(dyadthetas[,,train], midvalues[train,], dyadthetas[,,test], midvalues[test,], lambda=1)

    stopifnot(length(test)==2)
    testdyad_probs = matrix(final_fit$predprob, ncol=length(test))
    testY  = as.integer(as.vector(t(midvalues[test,])) >= 3)
    # print(table(testY))
    auc = calc_auc(as.vector(testdyad_probs), testY)

    data.frame(test_dyad=holdout, auc=auc, num_conflict_timesteps=sum(testY==1))
  })
  loo_aucs
}

