#!/bin/zsh
#PBS -l ncpus=32
#PBS -q batch
#PBS -j oe

source ~/fake_pbs.env
cd $PBS_O_WORKDIR
source $HOME/profile
ja
set -ux

makeprop() {
  numFrames=$1
  model=$2
  numThreads=$3

  if [ $model = v ]; then
    contextModel="ScaleSmoother"
    extra="phaseGlobalEnd=999999"
  elif [ $model = is ]; then
    contextModel="ScaleSmoother"
    extra="phaseScaleEnd=999999"
  elif [ $model = ss ]; then
    contextModel="ScaleSmoother"
    extra=""
  elif [ $model = sf ]; then
    contextModel="FrameSmoother"
    extra=""
  else
    echo WTF
    exit -1
  fi

cat <<-EOF
numThreads = $numThreads
numFrames = $numFrames
pathFile = data/v7.pathfil.dthresh=500.pthresh=10
dateFile = data/datefile.week
contextModel = $contextModel
$extra
numDims = 1
maxIter = 100000
displayEvery = 500
saveEvery = 100
concResampleEvery = 100
transVar = 1
etaVar = 1
priorScaleVar = 1
priorIndepPositionVar = 100
priorAlphaVar = 100
burnin = 9000
EOF
}

#################################################

go() {

  chain=$1
  numFrames=$2
  model=$3
  numThreads=$4

  runprefix=s/$my_jobid.v7.$model.k=$numFrames.c=$chain
  propfile=$runprefix.prop
  makeprop $numFrames $model $numThreads > $propfile

  outdir=$runprefix.out
  logfile=$runprefix.log
  mkdir -p $outdir
  cp $propfile $outdir/run.prop

  (
    set -x
    env GCTHREADS=$numThreads ./fast.sh MainModel $propfile $outdir
    ~/ptab/post/nicepaths.sh $outdir
    python ~/ptab/post/average.py $outdir 9000
    python ~/ptab/verbdict/score.py internal_diff $outdir/mean
  ) 2>&1 | cat > $logfile
}

# go sg 100 sf 32 &

# for c in {1..5}; do
#   go $c 20 sf 6 &
# done

# for c in {1..4}; do
#   go $c 50 v 8 &
# done

# for c in {1..4}; do
#   go $c 50 sf 8 &
# done

# go 1 100 v 16 &
# go 2 100 v 16 &


wait

#################################################
ja -chlst
