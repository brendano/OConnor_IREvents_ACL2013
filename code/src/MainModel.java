
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.Serializable;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import util.Arr;
import util.BasicFileIO;
import util.FastRandom;
import util.MCMC;
import util.ThreadUtil;
import util.Timer;
import util.U;
import util.Util;

import com.google.common.base.Function;
import com.google.common.collect.HashMultimap;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Multimap;
import com.google.common.collect.Sets;
import com.google.common.io.Files;

import edu.stanford.nlp.util.ArrayUtils;
import edu.stanford.nlp.util.Pair;


public class MainModel implements Serializable {
	static final long serialVersionUID = -1L;
	FastRandom rand;

	Vocabulary pathVocab;
	Vocabulary contextVocab;
	Vocabulary actorVocab;
	Vocabulary dyadVocab;
	List<EventTuple> eventTuples;
	Map<String,Integer> datestrToTimenum;
//	LinkedHashSet<Pair<Integer,Integer>> allDyads;
	List<Pair<Integer,Integer>> dyadInfos;  // source,target IDs for each dyad (by dyadid)
	List<ContextInfo> contextInfos; // indexed by contextid
	int[][] dyadTime_ContextID;   // #Dyads x T, each value is a contextid
	Multimap<Integer,Integer> dyadToContextIDs;
	Map<Pair<Integer,Integer>, Integer> dyadIDbyPair;
	
	int numFrames = 5; 		// number of latent frames
	int numEvents = -1;  	// number of event tuples (eventTuples.size())
	int numContexts = -1;  	// number of ctx types (contextVocab.size())
	int numDims = -1;       // for the idealpoint/scale model
	int numPathTypes = -1;
	int numTimes = -1;
	int numDyads = -1;
	int[] frameIndicators;  // size E, value \in 0...F: mixture indicator for each tuple

	// tuple CGS count tables
	double[][] cContextFrame; // size (#ctxs x #frames)
	double[][] cPathFrame; // size (#pathtypes x #frame)
	double[] cFrame;
	double[] cContext;
	double[] cPath;
	
//	ContextModel.LN_Scale contextModel;
	ContextModel contextModel;
	
	// Parameters
	double frameConc = 1;   // dirichlet concentration for P(f | context). LN doesn't use.
	double pathConc = 100;   // dirichlet concentration for P(path | f)
	
	RuntimeOptions opts;
	
	void readProperties(Properties properties) throws InstantiationException, IllegalAccessException {
		opts = new RuntimeOptions();
		opts.readProperties(properties);
		
		UtilHere.setFieldInteger(this, properties, "numFrames");
		UtilHere.setFieldInteger(this, properties, "numDims");
		if (properties.containsKey("seed")) {
			rand = new FastRandom(Integer.valueOf((String) properties.get("seed")));
		}
		
		String cmName = (String) properties.get("contextModel");
		if (cmName==null) throw new RuntimeException("need a context model");
		Class cmClass = U.getNestedClass(ContextModel.class, cmName);
		if (cmClass==null) throw new RuntimeException("bad contextModel " + cmName);
		contextModel = (ContextModel) cmClass.newInstance();
		contextModel.rand = rand;
		
		if (properties.containsKey("etaVar")) {
			((ContextModel.GeneralLN) contextModel).etaVar = Arr.rep(
					Double.valueOf((String) properties.get("etaVar")),
					numFrames-1);
		}
		UtilHere.setFieldDouble(contextModel, properties, "transVar");
		UtilHere.setFieldDouble(contextModel, properties, "priorScaleVar");
		UtilHere.setFieldDouble(contextModel, properties, "priorIndepPositionVar");
		UtilHere.setFieldDouble(contextModel, properties, "priorAlphaVar");
		
		if (properties.containsKey("numThreads")) {
			int n = Integer.valueOf((String) properties.get("numThreads"));
			ThreadUtil.createPool(n);
		}
	}
	
	MainModel() {
		pathVocab = new Vocabulary();
		contextVocab = new Vocabulary();
		actorVocab = new Vocabulary();
		dyadVocab = new Vocabulary();
		eventTuples = Lists.newArrayList();
		datestrToTimenum = Maps.newHashMap();
		rand = new FastRandom();
	}
	
	static class EventTuple implements Serializable {
		static final long serialVersionUID = -1L;

		public int context;
		public int path;
		public int time;
		public int dyad;
		public int sourceActor = -1;
		public int targetActor = -1;
		public String docid;
		public String sentid;
	}
	static class ContextInfo implements Serializable {
		static final long serialVersionUID = -1L;
		
		public int sourceActor = -1;
		public int targetActor = -1;
		public int dyad = -1;
		public int time = -1;
	}
	
	public void readData() throws Exception {
		readDates();
		readTuples();
	}
	public void readDates() throws Exception {
		for (String line : BasicFileIO.openFileLines(opts.dateFile)) {
			String[] parts = line.split("\t");
			int timenum = Integer.valueOf(parts[0]);
			String date = parts[1];
			datestrToTimenum.put(date, timenum);
		}
	}
	public void readTuples() throws FileNotFoundException {
		U.p("Reading event tuples");
		LinkedHashSet<Pair<Integer,Integer>> allDyads = Sets.newLinkedHashSet();

		for (String line :  BasicFileIO.openFileLines(opts.pathFile)) {
			String[] parts = line.split("\t");
			String docid = parts[0];
			String sentid = parts[1];
			String date = parts[2];
			String sourceActor = parts[3];
			String targetActor = parts[4];
			String path = parts[5];
			String contextKey = U.sf("%d %s %s", datestrToTimenum.get(date), sourceActor, targetActor);
			EventTuple e = new EventTuple();
			e.context = contextVocab.num(contextKey);
			e.path = pathVocab.num(path);
			e.sourceActor = actorVocab.num(sourceActor);
			e.targetActor = actorVocab.num(targetActor);
			assert datestrToTimenum.containsKey(date);
			e.time = datestrToTimenum.get(date);
			e.docid = docid;
			e.sentid = sentid;
			eventTuples.add(e);
			
			allDyads.add(U.pair(e.sourceActor, e.targetActor));
		}
		
		numDyads = allDyads.size();
		dyadInfos = Lists.newArrayList();
		dyadIDbyPair = Maps.newHashMap();
		for (Pair<Integer,Integer> dyadPair : allDyads) {
			dyadInfos.add(dyadPair);
			dyadVocab.num(U.sf("%s %s %d %d",
					actorVocab.name(dyadPair.first), actorVocab.name(dyadPair.second),
					dyadPair.first, dyadPair.second));
		}
		for (int dyad=0; dyad<numDyads; dyad++) {
			dyadIDbyPair.put(dyadInfos.get(dyad), dyad);
		}

		U.p("done");
	}
	


	/** call only after tuple data is read **/
	public void setupModel() {
		numEvents = eventTuples.size();
		numContexts = contextVocab.size();
		numPathTypes = pathVocab.size();
		numTimes = Sets.newHashSet(datestrToTimenum.values()).size();
		U.pf("numevents %d numctxs %d numpathtypes %d\n", numEvents, numContexts, numPathTypes);
		U.pf("numtimes %d numdyads %d numactors %d\n", numTimes, numDyads, actorVocab.size());
		frameIndicators = new int[numEvents];
		cContextFrame = new double[numContexts][numFrames];
		cPathFrame = new double[numPathTypes][numFrames];
		cFrame= new double[numFrames];
		
		cContext = new double[numContexts];
		cPath = new double[numPathTypes];
		doDataCounts();
		
		actorVocab.lock();
		contextInfos = Lists.newArrayList();
		dyadTime_ContextID = new int[numDyads][numTimes];
		Arr.fill(dyadTime_ContextID, -1);
		
		for (int c=0; c<numContexts; c++) {
			ContextInfo ci = new ContextInfo();
			ci.time = Integer.valueOf(contextVocab.name(c).split(" ")[0]);
			ci.sourceActor = actorVocab.num(contextVocab.name(c).split(" ")[1]);
			ci.targetActor = actorVocab.num(contextVocab.name(c).split(" ")[2]);
			ci.dyad = dyadIDbyPair.get(U.pair(ci.sourceActor, ci.targetActor));
			contextInfos.add(ci);
			dyadTime_ContextID[ci.dyad][ci.time] = c;
		}
		
		dyadToContextIDs = HashMultimap.create();
		for (int context=0; context<numContexts; context++) {
			dyadToContextIDs.put(contextInfos.get(context).dyad, context);
		}
		
		contextModel.setupModel(this);
		
		if (opts.frozenTopics()) {
			cPathFrame = Arr.readDoubleMatrix(opts.frozenTopicsFile);
			assert cPathFrame.length == numPathTypes;
			assert cPathFrame[0].length == numFrames;
		}
	}
	public void doDataCounts() {
		for (EventTuple e : eventTuples) {
			cContext[e.context]++;
			cPath[e.path]++;
		}
	}

	public void sampleIteration(boolean first, boolean track) {
		double runningLL = 0;
		double unnormField[] = new double[numFrames];
		for (int i=0; i < numEvents; i++) {
			EventTuple e = eventTuples.get(i);

			if (!first) {
				decrementCounts(e, frameIndicators[i]);	
			}			
			double psum = frameUnnormField(e, unnormField);
			int f = rand.nextDiscrete(unnormField, psum);
			incrementCounts(e, f);
			frameIndicators[i] = f;
				
			if (track)
				runningLL += Math.log(unnormField[f]/psum);
		}
		if (track)
			U.pf("runningLL %.3f\n", runningLL);
	}
	
	

	/** output field into @param weights, a numFrames-sized vector.
	 * @return the weight sum. **/
	public double frameUnnormField(EventTuple e, double[] weights) {
		double psum=0;
		for (int f=0; f < numFrames; f++) {
			// P(f |ctx,path)
			// \propto P(f | ctx) P(path | f) = (#FrameCtx/#Ctx) (#PathFrame/#Frame)
			double frameFactor = contextModel.framePriorFactor(e.context, f);
			double pathFactor  = 
				(cPathFrame[e.path][f] + pathConc/numPathTypes) / (cFrame[f] + pathConc);
			double w = frameFactor * pathFactor;
			weights[f] = w;
			psum += w;
		}
		return psum;
	}
	
	public void incrementCounts(EventTuple e, int frame) {
		updateCounts(e, frame, 1);
	}
	public void decrementCounts(EventTuple e, int frame) {
		updateCounts(e, frame, -1);
	}
	public void updateCounts(EventTuple e, int frame, int delta) {
		cContextFrame[e.context][frame] += delta;
		cFrame[frame] += delta;
		if (!opts.frozenTopics()) {
			cPathFrame[e.path][frame] += delta;
		}
	}

	public void resampleConcs() {
		Function <double[],Double> fLL = new Function<double[],Double>() {
			@Override
			public Double apply(double[] input) {
				return frameLL(Math.exp(input[0]));
			}
		};
		Function <double[],Double> pLL = new Function<double[],Double>() {
			@Override
			public Double apply(double[] input) {
				return pathLL(Math.exp(input[0]));
			}
		};
		
		U.p("resampling pathconc");
		List<double[]> pHistory = MCMC.slice_sample(pLL, new double[]{Math.log(pathConc)}, new double[]{1}, 30);
		double newPathConc  = Math.exp(pHistory.get(pHistory.size()-1)[0]);
		U.pf("pathconc  %.6g -> %.6g\n", pathConc, newPathConc);
		this.pathConc  = newPathConc;

		// TODO re-enable for LDA
//		if (contextModel instanceof ContextModel.LDA) {
//			U.p("resampling frameconc");
//			List<double[]> fHistory = Util.slice_sample(fLL, new double[]{Math.log(frameConc)}, new double[]{1}, 30);
//			double newFrameConc = Math.exp(fHistory.get(fHistory.size()-1)[0]);
//			U.pf("frameconc %.6g -> %.6g\n", frameConc, newFrameConc);
//			this.frameConc = newFrameConc;
//		}

	}
	/** this does NOT use model's concentration; instead the passed-in value. **/
	public double frameLL(double _frameConc) {
		double ll = 0;
		for (int ctx=0; ctx < numContexts; ctx++) {
			ll += Util.dirmultSymmLogprob(cContextFrame[ctx], cContext[ctx], _frameConc/numFrames);
		}
		return ll;
	}
	/** this does NOT use model's concentration; instead the passed-in value. **/
	public double pathLL(double _pathConc) {
		double ll = 0;
		for (int f=0; f < numFrames; f++) {
			double vec[] = Arr.getCol(cPathFrame,f);
			ll += Util.dirmultSymmLogprob(vec, cFrame[f], _pathConc/numPathTypes);
		}
		return ll;
	}
	

	/** deep copies the important data **/
	public MainModel copy() {
		MainModel m = new MainModel();
		m.cContext 		= ArrayUtils.copy(cContext);
		m.cContextFrame 	= ArrayUtils.copy(cContextFrame);
		m.cFrame 		= ArrayUtils.copy(cFrame);
		m.cPath 		= ArrayUtils.copy(cPath);
		m.cPathFrame 	= ArrayUtils.copy(cPathFrame);
		m.frameIndicators = ArrayUtils.copy(frameIndicators);

		m.numFrames = numFrames;
		m.numEvents = numEvents;
		m.numContexts = numContexts;
		m.numPathTypes = numPathTypes;
		
		m.pathConc = pathConc;
		m.frameConc = frameConc;
		
		m.contextVocab = contextVocab;
		m.pathVocab = pathVocab;

		m.eventTuples = eventTuples;
		
		m.opts = opts;
		
		return m;
	}
	
	/**
	 * indicators is vector length N, values {0, 1, ..., K-1}
	 * fields is matrix (N x K)
	 */
	static void addIndicatorsIntoFields(int[] srcIndicators, double[][] tgtFields) {
		assert tgtFields.length == srcIndicators.length;
		for (int i=0; i < srcIndicators.length; i++) {
			tgtFields[i][srcIndicators[i]] += 1.0;
		}
	}
	
	public void train() throws IOException {
		double t0 = (double) System.currentTimeMillis();
		Timer tr = new Timer();
		
		sampleIteration(true,false);
		if (opts.concResampleEvery>=0) resampleConcs();
//		for (int i=1; i<0; i++) {
//			sampleIteration(true,true);
//			resampleConcs();
//			U.pf("totalLL %.3f\n", pathLL(pathConc) + contextModel.logProb());
//		}
//		contextModel.inferEtas();
		
		for (int iter=1; iter < opts.maxIter; iter++) {
			U.pf("ITER %d\n", iter);
						
			tr.tick("z sample");
			for (int inner=0; inner<3; inner++) {
				sampleIteration(false, false);
			}
			tr.tock();
			tr.tick("etas");
			contextModel.inferEtas();
			tr.tock();
			tr.tick("contextmodel");
			contextModel.inferContextPriors(
					iter <= opts.phaseGlobalEnd ? ContextModel.Phase.GLOBAL :
					iter <= opts.phaseScaleEnd ? ContextModel.Phase.SCALE :
						ContextModel.Phase.TIME);
			tr.tock();
			
			if (opts.concResampleEvery>0 && iter % opts.concResampleEvery == 0) {
				tr.tick("concs");
				resampleConcs();
				tr.tock();
			}

			if (iter < 100 || iter % 20 == 0) {
				tr.tick("ll compute");
				double wordLL = pathLL(pathConc);
				U.pf("wordLL %.3f\n", wordLL);
//				if (contextModel instanceof ContextModel.LDA)
//					U.pf("ldaLL %.3f\n", frameLL(frameConc)+pathLL(pathConc));
				double etaLL = contextModel.llEtaAndFrameind();
				double cmLL = contextModel.llContextParams();
				U.pf("etaLL %.3f\n", etaLL);
				U.pf("cmLL %.3f\n", cmLL);
				U.pf("totalLL %.3f\n", wordLL + cmLL + etaLL);
				tr.tock();				
			}
			
			tr.tick("model storage etc");
			if (opts.displayEvery>0 && iter % opts.displayEvery == 0) {
				DumpModel.display(this);
			}
			boolean burninNow = opts.burnin>0 && iter < opts.burnin-1; 
			if (!burninNow && opts.saveEvery>0 && (iter % opts.saveEvery == 0)) {
//				saveModel(iter);
				DumpModel.modelTextDump(this, opts.outputDir + "/model." + iter);
			}
			tr.tock();
			
			double elapsed = System.currentTimeMillis() - t0;
			U.pf("Timing: %.1f ms total, %.1f ms/iter, %.1f iter/hr\n", 
					elapsed, elapsed / (iter+1),  (iter+1) / (elapsed/1000/3600)  );
			tr.report(elapsed);
			Timer.timer().report(elapsed);
		}
		U.p("Iterations finished");
		ThreadUtil.threadPool.shutdown();
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////
	
	
	public void displayContextSample() {
		for (int c=0; c < numContexts; c += 5000) { 
			
			U.pf("c=%d\t%s", c, Arr.sf("%.0f", cContextFrame[c]));
			if (contextModel instanceof ContextModel.GeneralLN) {
				U.pf("\t%s", Arr.sf("%.3f", ((ContextModel.GeneralLN) contextModel).etas[c]));
	//			U.pf("\t%s", Arrays.toString(((ContextModel.LNmodel) contextModel).etaMean[c]));
			}
			U.pf("\n");
			
		}
	}
	
	public void saveInitial() throws IOException {
		Files.createParentDirs(new File(opts.outputDir + "/bla"));
		contextVocab.dump(opts.outputDir + "/context.vocab");
		pathVocab.dump(opts.outputDir + "/path.vocab");
		actorVocab.dump(opts.outputDir + "/actor.vocab");
		dyadVocab.dump(opts.outputDir + "/dyad.vocab");
		File df = new File(opts.dateFile);
		Files.copy(df, new File(opts.outputDir + "/datefile"));
	}
	public void saveModel(int iter, String pfx) throws IOException {
		String prefix = opts.outputDir + "/" + pfx + "." + iter; 
		Files.createParentDirs(new File(prefix));
		String filename = prefix + ".ser.gz";
		U.pf("Saving to %s ", filename); System.out.flush();
		BasicFileIO.writeSerializedObject(prefix + ".ser.gz", this);
		U.pf("done\n");
	}
	public void saveModel(int iter) throws IOException {
		saveModel(iter, "model");
	}
	public void saveEtas() {
		if (contextModel instanceof ContextModel.GeneralLN) {
			ContextModel.GeneralLN cm = (ContextModel.GeneralLN) contextModel;
			Arr.write(cm.etas, 		opts.outputDir + "/tmp.etas");
		} else {
			// um, skip?
		}
	}
	
	/////////////////////////////////////////////////////////////////

	public static void main(String[] args) throws Exception {
		MainModel m = new MainModel();
		Properties props = U.loadProperties(args[0]);
		U.p("Properties\t" + props);
		m.readProperties(props);
		m.opts.outputDir = args.length>1 ? args[1] : "out";
		U.p("Saving to " + m.opts.outputDir);
		
		Files.createParentDirs(new File(m.opts.outputDir + "/bla"));		
		m.readData();
		m.setupModel();
		m.saveInitial();
		m.train();
	}

}
