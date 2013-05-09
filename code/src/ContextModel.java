import java.io.Serializable;
import java.util.List;
import java.util.concurrent.Callable;

import util.Arr;
import util.FastRandom;
import util.GaussianInference;
import util.LNInference;
import util.MVNormal2;
import util.OnlineNormal1dWeighted;
import util.ThreadUtil;
import util.U;
import util.Util;

import com.google.common.collect.Lists;
import com.google.common.primitives.Ints;

import edu.stanford.nlp.util.Pair;


/**
 * undirected GM over RV groups is
 * 
 * contextPrior <-> etaMean <-> eta <-> textcounts
 *    
 * So there are three different places of interaction, each potentially having
 * factor functions for GS, gradients & logprobs for MAP, etc.
 * 
 * For LDA, everything is all thrown in together (very simple).
 * 
 * For LN, gets divided up by subclasses.
 * 
 * eta <-> textcounts       .... controlled by the GeneralLN superclass.
 * etaMean <-> eta          .... controlled by the GeneralLN superclass.
 * contextPrior <-> etaMean .... controlled by GeneralLN's specific subclass.
 * 
 * the interface's three high-level functions are implicated in the following RV factors.
 * 
 * framePriorFactor()   .... eta <-> textcounts
 * inferContextPriors() .... contextPrior <-> etaMean <-> eta
 * inferEtas()          .... etaMean <-> eta <-> textcounts
 * 
 */ 
public class ContextModel implements Serializable {
	static final long serialVersionUID = -1L;
	
	static enum Phase { GLOBAL, SCALE, TIME };
	
	MainModel m;
	public FastRandom rand = null;   // needs to be set from outside before doing stuff
	
	/** have to use an empty constructor so reflection is happy */
	public ContextModel() {
	}
	
	///////// the interface

	public void setupModel(MainModel mainModel) {
		this.m = mainModel;
	}

	/**  g(f) \propto P(f | ctx) */
	public double framePriorFactor(int context, int f) { throw new RuntimeException("not implemented"); }
	
	/** Fit the context model, conditional on eta.  e.g. for LN, P(mu,alpha,etc. | eta) 
	 * This function is reponsible for setting etaMean when done.
	 */ 
	public void inferContextPriors(Phase phase) { throw new RuntimeException("not implemented"); }
	
	/** Fit the etas, conditional on context and text.  P(eta | etaMean, f)
	 * where etaMean is the sum of alpha,mu, etc. from the context priors. 
	 * This function is responsible for setting eta when done.
	 */
	public void inferEtas(int startC, int endC) { throw new RuntimeException("not implemented"); }
	
	public void inferEtas() { throw new RuntimeException("not implemented"); }

	/** P(z, eta | etamean,etavar) */
	public double llEtaAndFrameind() { return Double.NEGATIVE_INFINITY; }
	/** P(coefs comprising etamean | hyperparameters) */
	public double llContextParams() { return Double.NEGATIVE_INFINITY; }
	
	///////// ok that's it for the interface.  implementations below.
	
	///////////////////////////////////////////////////////////////
	
	public static class LDA extends ContextModel {
		static final long serialVersionUID = -1L;

		@Override
		public double framePriorFactor(int context, int f) {
			// P(f|ctx) = #(Ctx,Frame) / #Ctx
			//          = (#(Ctx,Frame) + a) / (#Ctx + a0)   with priors  
			//    \propto (#(Ctx,Frame) + a)                 drop normalizer
			return m.cContextFrame[context][f] + m.frameConc / m.numFrames;
		}
		@Override
		public void inferContextPriors(Phase phase) {
			// no-op for LDA. assume MainModel is tracking the counts.
		}
		@Override
		public void inferEtas() { 
			// no-op for LDA.
		}
		@Override
		public double llContextParams() {
			// thetas are collapsed outb
			return 0;
		}
		@Override
		public double llEtaAndFrameind() {
			return m.frameLL(m.frameConc);
		}
	}
	
	/**
	 * superclass for all softmax-based, logistic-normal models.
	 * all about eta & theta.
	 * This class has implementation for the etaMean <-> eta interaction.
	 * but it has nothing about contextPriors <-> etaMean.
	 * it is the responsibility of subclasses to handle that relationship.
	 */
	public static class GeneralLN extends ContextModel {
		static final long serialVersionUID = -1L;

		double etas[][]; // logit version.   size #Ctx x (#Frame-1).  etas[c] \in R^#Frame
		double thetas[][]; // probabilities. size #Ctx x #Frame.    thetas[c] \in Simplex(#Frame)
		
		// eta_k | ctxmodel ~ N(etaPriorMean, etaVar)
		double etaMean[][]; // deterministic from the context model
		double etaVar[];
		double etaVarPriorValue = 1;
		double etaVarPriorStrength = 1;   // these priors don't matter much since >10k datapoints of evidence
		
		@Override
		public void setupModel(MainModel mainModel) {
			super.setupModel(mainModel);
			
			assert etaVar != null;
//			Arrays.fill(etaVar, 10); // wanna start overdispersed.  starting too low is sticky
//			Arrays.fill(etaVar, 2);
			etas   = new double[m.numContexts][m.numFrames-1];
			thetas = new double[m.numContexts][m.numFrames];
			etaMean= new double[m.numContexts][m.numFrames-1];
			for (int c=0; c<m.numContexts; c++) {
				for (int k=0; k<m.numFrames-1; k++) {
					etas[c][k] = rand.nextGaussian();
				}
				thetas[c] = Arr.softmax1(etas[c]);
			}
		}
		
		@Override
		public double framePriorFactor(int context, int f) {
			return thetas[context][f];
		}
		
		//////////////////////////////////////////////////////////
		///// eta inference stuff (shared for all LN models) /////
				
		@Override
		public void inferEtas(int startC, int endC) {
			int numAccept = 0;
			endC = Math.min(endC, m.numContexts);
			// Call this only after etamean has been set up correctly.
			for (int c=startC; c < endC; c++) {
//				double[] newEta = LNInference.sampleSlice(10, m.cContextFrame[c], etaMean[c], etaVar);
//				double[] newEta = LNInference.estNewton(20, etas[c], m.cContextFrame[c], m.cContext[c], etaMean[c], etaVar);
//				double[] newEta = LNInference.sampleLaplace(m.cContextFrame[c], m.cContext[c], etaMean[c], etaVar, ThreadUtil.rand());
//				double[] newEta = LNInference.sampleLaplaceDiagonal(m.cContextFrame[c], m.cContext[c], etaMean[c], etaVar, FastRandom.rand());
//				etas[c] = newEta;
//				thetas[c] = Arr.softmax1(newEta);
				
				LNInference.OneMH result = LNInference.sampleOneMH(etas[c], m.cContextFrame[c], m.cContext[c], etaMean[c], etaVar, FastRandom.rand());
				if (result.wasAccepted) {
					etas[c] = result.newValue;
					thetas[c] = Arr.softmax1(etas[c]);
					numAccept++;
				}
				
			}
			U.p("eta_accept_ratio " + 1.0*numAccept / (endC-startC));
		}
		
		@Override
		public void inferEtas() {
//			inferEtas_Serial();
			inferEtas_Par();
		}
		
		void inferEtas_Serial() {
			inferEtas(0, m.numContexts);
		}
		
		void inferEtas_Par() {
			final int step = 1000;
	    	List<Callable<Integer>> tasks = Lists.newArrayList();
			for (final Integer cstart : Ints.asList(Arr.rangeInts(0, m.numContexts, step))) {
				tasks.add(new Callable<Integer>() {
					@Override
					public Integer call() throws Exception {
						inferEtas(cstart, cstart+step);
						return null;
					}
				});
			}
			ThreadUtil.runAndWaitForTasks(tasks);
		}

		void learnEtaVar() {
			// Requires etamean to be set correctly.
			
			double[] sumSqDevs = new double[m.numFrames-1];
			for (int c=0; c < m.numContexts; c++) {
				for (int f=0; f<m.numFrames-1; f++) {
					sumSqDevs[f] += Math.pow(etas[c][f] - etaMean[c][f], 2);
				}
			}
//			double etaVarAll = GaussianInference.samplePosteriorVariance(
//					Arr.sum(sumSqs), m.numContexts*m.numFrames, 1, 1e-3, rand);
//			Arr.fill(etaVar, etaVarAll);
			for (int f=0; f<m.numFrames-1; f++) {
				etaVar[f] = GaussianInference.samplePosteriorVariance(sumSqDevs[f], m.numContexts, 1, 1e-3, rand);
				if (etaVar[f] > 10000) etaVar[f] = 10000;
			}
			U.pf("etaVar %s\n", Arr.sf("%.3f", etaVar));
			// code for single shared etaVar
//			etaVar = GaussianInference.samplePosteriorVariance(sumSq, m.numContexts*(m.numFrames-1), etaVarPriorValue, etaVarPriorStrength);
//			U.pf("etaVar %f\n", etaVar);
		}
		
		@Override
		public double llEtaAndFrameind() {
			double ll = 0;
			for (int c=0; c<m.numContexts; c++) {
				for (int f=0; f<m.numFrames-1; f++) {
					ll += Util.normalLL(etas[c][f], etaMean[c][f], etaVar[f]);
				}
			}
			for (int i=0; i<m.numEvents; i++) {
				double[] theta = thetas[m.eventTuples.get(i).context];
				ll += Math.log(theta[m.frameIndicators[i]]);
			}
			return ll;
		}
	}
	

	public static class FrameSmoother extends GeneralLN {
		// etamean_kt = alpha_k + beta_kt
		// beta_kt ~ N(beta_k,t-1, tau^2)
		
		double[] globalCoefs;    // alpha, size K
		double[][][] betas;        // size #Dyads x T x K
		
		double transCoef = 1;  // for AR(1)
		double transVar = -1;   // for AR(1)
		double priorAlphaMean = 0;
		double priorAlphaVar = -1;

		@Override
		public void setupModel(MainModel m) {
			super.setupModel(m);
			globalCoefs = new double[m.numFrames-1];
			betas = new double[m.numDyads][m.numTimes][m.numFrames-1];
		}
		
		@Override
		public void inferContextPriors(Phase phase) {
			learnEtaVar();
			learnAlphasFromResidual();
			learnBeta();
			learnTransVar();
			updateEtaMean();
			learnEtaVar();
			
			U.p("alpha " + Arr.sf("%.3g", globalCoefs));
			U.p("etaVar " + Arr.sf("%.3g", etaVar));
			U.p("transVar " + transVar);
			U.pf("framecounts "); U.p(m.cFrame);
		}
		
		void learnBeta() {
//			learnBeta_Ser();
			learnBeta_Par();
		}
		void learnBeta_Ser() {
			for (int dyad=0; dyad < m.numDyads; dyad++) {
				learnBeta_OneDyad(dyad);
			}
		}
		void learnBeta_Par() {
			ThreadUtil.processMinibatches(m.numDyads, 100, new ThreadUtil.MinibatchProcedure() {
				@Override
				public void apply(int start, int end) {
					for (int dyad=start; dyad<end; dyad++) {
						learnBeta_OneDyad(dyad);
					}
				}
			});
		}
		
		void learnBeta_OneDyad(int dyad) {
			for (int k=0; k<m.numFrames-1; k++) {
				learnBeta_OneDyadFrame(dyad, k);
			}
		}
		void learnBeta_OneDyadFrame(int dyad, int k) {
			double[] observations = Arr.rep(Double.POSITIVE_INFINITY, m.numTimes);
			for (int c : m.dyadToContextIDs.get(dyad)) {
				int t = m.contextInfos.get(c).time;
				observations[t] = etas[c][k];
			}
			
			GaussianInference.FilterResult1d fr = GaussianInference.kalmanFilter(
					observations,
					transCoef, transVar, 1, etaVar[k], 
					Arr.rep(0, m.numTimes), Arr.rep(globalCoefs[k], m.numTimes), 0, 100);
			
			double[] sample = GaussianInference.backwardSample(fr, 1, FastRandom.rand());
			
			for (int t=0; t < m.numTimes; t++) {
				betas[dyad][t][k] = sample[t];
			}
		}
		
		static class DenseEtaTheta {
			public double[][][] denseEtas;   // size #Dyads x T x (K-1)
			public double[][][] denseThetas; // size #Dyads x T x K
		}
		
		/** use the current etas and thetas from the sparse matrixes, but sample the rest. */ 
		DenseEtaTheta inferDenseEtaTheta() {
			DenseEtaTheta r = new DenseEtaTheta();
			r.denseEtas = new double[m.numDyads][m.numTimes][m.numFrames-1];
			r.denseThetas = new double[m.numDyads][m.numTimes][m.numFrames];
			
			for (int d=0; d<m.numDyads; d++) {
				for (int t=0; t<m.numTimes; t++) {
					int c = m.dyadTime_ContextID[d][t];
					if (c != -1) {
						// copy
						r.denseEtas[d][t]   = etas[c];
						r.denseThetas[d][t] = thetas[c];
					} else {
						// infer: there's no observation, so just sample from the eta prior.
						double[] etaMean = Arr.pairwiseAdd(globalCoefs, betas[d][t]);
						r.denseEtas[d][t] = Util.normalDiagSample(etaMean, etaVar, FastRandom.rand());
						r.denseThetas[d][t] = Arr.softmax1(r.denseEtas[d][t]);
					}
				}
			}
			return r;
		}
		
		void updateEtaMean() {
			for (int c=0; c < m.numContexts; c++) {
				int time = m.contextInfos.get(c).time;
				int dyad = m.contextInfos.get(c).dyad;
				for (int f=0; f < m.numFrames-1; f++) {
					etaMean[c][f] = globalCoefs[f] + betas[dyad][time][f];
				}
			}
		}

		void learnTransVar() {
			OnlineNormal1dWeighted r = new OnlineNormal1dWeighted();
			for (int t=1; t<m.numTimes; t++) {
				for (int dyad=0; dyad<m.numDyads; dyad++) {
					for (int k=0; k<m.numFrames-1; k++) {
						double diff = betas[dyad][t][k] - betas[dyad][t-1][k];
						r.add(diff);
					}
				}
			}
			U.pf("transvar suff stats "); U.p(r);
			if (r.var() > 1e-50) {
				transVar = r.var();
			}
		}
		
		void learnAlphasFromResidual() {
			for (int f=0; f<m.numFrames-1; f++) {
				double[] resid = Arr.getCol(etas, f);
				Arr.pairwiseSubtractInPlace(resid, Arr.getCol(etaMean, f));
				Arr.addInPlace(resid, globalCoefs[f]);
				globalCoefs[f] = GaussianInference.samplePosteriorMean(
						resid, etaVar[f], 0, priorAlphaVar, rand);
			}
		}
		
		/** P( etamean components | hyperpriors ) */
		public double llContextParams() {
			double ll = 0;

			for (int t=1; t<m.numTimes; t++) {
				for (int dyad=0; dyad<m.numDyads; dyad++) {
					for (int k=0; k<m.numFrames-1; k++) {
						ll += Util.normalLL(betas[dyad][t][k], transCoef* betas[dyad][t-1][k], transVar);
					}
				}
			}

			return ll;
		}

	}
	
	public static class ScaleSmoother extends GeneralLN {
		
		// D: number of latent dimensions.  K: number of frames (event types)
		double[] globalCoefs;    // alpha, size K
		double[][] frameScales;  // gamma, size K x D
		double[][][] dyadTimeScales;   // p's, size #dyads x #timesteps x D
		
		double priorAlphaVar = -1;
		double priorScaleVar = 1;
		double transCoef = 1;  // for AR(1)
		double transVar = 1;   // for AR(1)
		double priorIndepPositionVar = -1; // only for the non-AR variant
		
		@Override
		public void setupModel(MainModel mm) {
			super.setupModel(mm);
			globalCoefs = new double[m.numFrames-1];
			frameScales = new double[m.numFrames-1][m.numDims];
			dyadTimeScales = new double[m.numDyads][m.numTimes][m.numDims];
		}

		void learnIndepPositions() {
//			learnIndepPositions(0, m.numContexts);
			learnIndepPositions_Par();
		}
		void learnIndepPositions_Par() {
			ThreadUtil.processMinibatches(m.numContexts, 1000, new ThreadUtil.MinibatchProcedure() {
				@Override
				public void apply(int start, int end) {
					learnIndepPositions(start, end);
				}
			});
		}
		void learnIndepPositions_Ser() {
		}
		void learnIndepPositions(int startC, int endC) {
			double[] priorMean = Arr.rep(0, m.numDims);
			double[][] priorPrec = Arr.diag(Arr.rep(1.0/priorIndepPositionVar, m.numDims));

			for (int c=startC; c<endC; c++) {
				double[] Y = new double[m.numFrames-1];
				for (int k=0; k<m.numFrames-1; k++) {
					Y[k] = etas[c][k] - globalCoefs[k];
				}
				double[] newDyadPosition = GaussianInference.samplePosteriorLinregCoefs(
						frameScales, Y, etaVar,
						priorMean, priorPrec,
						FastRandom.rand());
				dyadTimeScales[m.contextInfos.get(c).dyad][m.contextInfos.get(c).time] = newDyadPosition;
			}
		}


		@Override
		public void inferContextPriors(Phase phase) {

			learnEtaVar();
			
			if (phase==Phase.GLOBAL) {
				learnJustAlphas();
			}
			else if (phase==Phase.SCALE) {
				learnAlphasFromResidual();
				learnFrameScales_Par();
				learnIndepPositions();
			}
			else if (phase==Phase.TIME){
				learnAlphasFromResidual();
				learnFrameScales_Par();
				learnContextPositions_Par();
				learnTransVar();
			}
			updateEtaMean();
			learnEtaVar();

			U.p("alpha " + Arr.sf("%.3g", globalCoefs));
			U.p("etaVar " + Arr.sf("%.3g", etaVar));
			U.p("transVar " + transVar);
			U.pf("framecounts "); U.p(m.cFrame);
			U.pf("framescales "); U.p(frameScales);
		}
		void learnAlphasFromResidual() {
			// assumes etamean is set correctly (for the old version of the coefs).
			// where 'x' is "all other coefs besides alpha",
			//  etamean = a_old + x_old  ==>  x_old = etamean - a_old  
			//  eta ~ N(a_new + x_old)
			//  (eta - x_old) ~ N(a_new)
			//  (eta - etamean + aold) ~ N(a_new)
			// thus a_new is the posterior mean of (eta - etamean + aold)
			for (int f=0; f<m.numFrames-1; f++) {
				double[] resid = Arr.getCol(etas, f);
				Arr.pairwiseSubtractInPlace(resid, Arr.getCol(etaMean, f));
				Arr.addInPlace(resid, globalCoefs[f]);
				globalCoefs[f] = GaussianInference.samplePosteriorMean(
						resid, etaVar[f], 0, priorAlphaVar, rand);
			}
		}
		void renormalizeHack() {
			Arr.standardize(globalCoefs);
			double[] arr = Arr.convertToVector(frameScales);
			Arr.standardize(arr);
			frameScales = Arr.convertToMatrix(arr, m.numDims);
		}
		/** update all etamean's based on whatever the current params are */
		void updateEtaMean() {
			for (int c=0; c < m.numContexts; c++) {
				int time = m.contextInfos.get(c).time;
				int dyad = m.contextInfos.get(c).dyad;
				for (int f=0; f < m.numFrames-1; f++) {
					double x = Arr.innerProduct(frameScales[f], dyadTimeScales[dyad][time]);
					etaMean[c][f] = globalCoefs[f] + x;
				}
			}
		}

		/**
		Run FFBS for each dyad: multivariate observations across frame dims.
		each timestep's eta is one multivariate observation.
		Uses current frameScales, globalCoefs, and etas.
		*/
		void learnContextPositions_Ser() {
			assert m.numDims==1;
			final double[] frameScales_vec = Arr.getCol(frameScales, 0);
			for (int dyad=0; dyad < m.numDyads; dyad++) {
				learnContextPosition_OneDyad(frameScales_vec, dyad);
			}
		}

		void learnContextPosition_OneDyad(double[] frameScales_vec, int dyad) {
			// setup the dyad-specific timeseries, with NA's
			double[][] observations = Arr.rep(Double.POSITIVE_INFINITY, m.numTimes, m.numFrames-1);
			for (int c : m.dyadToContextIDs.get(dyad)) {
				int t = m.contextInfos.get(c).time;
				observations[t] = etas[c];
			}
			assert m.numDims==1;  // TODO think we need D-dimensional KF to do this
			
			// calculate and store result
			GaussianInference.FilterResult1d fr = GaussianInference.kalmanFilter(
					observations,
					transCoef, transVar, frameScales_vec, etaVar, 
					Arr.rep(0, m.numTimes), Arr.rep(globalCoefs, m.numTimes), 0, 100);
			
			double[] sample = GaussianInference.backwardSample(fr, 1, FastRandom.rand());
			
			for (int t=0; t < m.numTimes; t++) {
				dyadTimeScales[dyad][t][0] = sample[t];
			}
		}
		
		void learnContextPositions_Par() {
			assert m.numDims==1;
			final double[] frameScales_vec = Arr.getCol(frameScales, 0);
			
	    	List<Callable<Integer>> tasks = Lists.newArrayList();
			for (final Integer dyad : Ints.asList(Arr.rangeInts(m.numDyads))) {
				tasks.add(new Callable() {
					@Override
					public Object call() throws Exception {
						learnContextPosition_OneDyad(frameScales_vec, dyad);
						return null;
					}
				});
			}
			ThreadUtil.runAndWaitForTasks(tasks);
		}

		
		void learnTransVar() {
			OnlineNormal1dWeighted r = new OnlineNormal1dWeighted();
			for (int t=1; t<m.numTimes; t++) {
				for (int dyad=0; dyad<m.numDyads; dyad++) {
					for (int dim=0; dim<m.numDims; dim++) {
						double diff = dyadTimeScales[dyad][t][dim] - dyadTimeScales[dyad][t-1][dim];
						r.add(diff);
					}
				}
			}
			U.pf("transvar suff stats "); U.p(r);
			if (r.var() > 1e-50) {
				transVar = r.var();
			}
		}

		void learnFrameScales_Par() {
	    	// about 1/20'th the time of below
	    	final double[][] X = calcPositionPerContext();

	    	List<Callable<Integer>> tasks = Lists.newArrayList();
			for (final Integer k : Ints.asList(Arr.rangeInts(m.numFrames-1))) {
				tasks.add(new Callable<Integer>() {
					@Override
					public Integer call() throws Exception {
						learnFrameScales_OneFrame(X, k);
						return 0;
					}
				});
 			}
			ThreadUtil.runAndWaitForTasks(tasks);
		}
		
		void learnFrameScales() {
			double[][] X = calcPositionPerContext();
			for (int k=0; k<m.numFrames-1; k++) {
				learnFrameScales_OneFrame(X,k);
 			}
		}
		
		double[][] calcPositionPerContext() {
			double[][] X = new double[m.numContexts][m.numDims];
			for (int c=0; c<m.numContexts; c++) {
				for (int d=0; d<m.numDims; d++) {
					X[c][d] = dyadTimeScales[m.contextInfos.get(c).dyad][m.contextInfos.get(c).time][d];
				}
			}
			return X;
		}

		void learnFrameScales_OneFrame(double[][] X, int k) {
			double[] Y = Arr.getCol(etas, k);
			Arr.addInPlace(Y, -globalCoefs[k]);
			double[] newGamma = GaussianInference.samplePosteriorLinregCoefs(
					X, Y, etaVar[k],
					Arr.rep(0, m.numDims), Arr.diag(Arr.rep(priorScaleVar, m.numDims)), 
					FastRandom.rand());
			frameScales[k] = newGamma;
		}
		
		
		void learnJustAlphas() {
			// assumes all scale parameters are 0
			for (int f=0; f<m.numFrames-1; f++) {
				globalCoefs[f] = GaussianInference.samplePosteriorMean(
						Arr.getCol(etas, f), etaVar[f], 0, priorAlphaVar, rand);
			}
		}

		/** P( etamean components | hyperpriors ) .. really, only the AR process. */
		public double llContextParams() {
			double ll = 0;
			// framescale
			for (int f=0; f<m.numFrames-1; f++) {
				for (int d=0; d<m.numDims; d++) {
					ll += Util.normalLL(frameScales[f][d], 0, priorScaleVar);
				}
			}
			// AR process (but not the emissions!)
			for (int dyad=0; dyad<m.numDyads; dyad++) {
				for (int d=0; d<m.numDims; d++) {
					for (int t=1; t<m.numTimes; t++) {
						ll += Util.normalLL(dyadTimeScales[dyad][t][d], transCoef*dyadTimeScales[dyad][t-1][d], transVar);
					}	
				}
				
			}
			return ll;
		}

		static final long serialVersionUID = -1L;
	}

}
