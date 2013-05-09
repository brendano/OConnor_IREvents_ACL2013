import java.io.IOException;
import java.util.List;

//import org.apache.lucene.search.TopDocs;

import util.Arr;
import util.BasicFileIO;
import util.U;
import util.Util;
import edu.stanford.nlp.math.ArrayMath;

public class DumpModel {
	
	public static void modelTextDump(MainModel m, String prefix) throws IOException {
		U.p("output to prefix " + prefix);
		Arr.write(m.cContextFrame, prefix+".cContextFrame");
		Arr.write(m.cContext, prefix+".cContext");
		Arr.write(m.cPathFrame, prefix+".cPathFrame");
		Arr.write(m.cFrame, 	prefix+".cFrame");
		Arr.write(m.frameIndicators, prefix+".frameIndicators");

		if (m.contextModel instanceof ContextModel.GeneralLN) {
			ContextModel.GeneralLN cm = (ContextModel.GeneralLN) m.contextModel;
			Arr.write(cm.etaMean,	prefix+".etaMean");
			Arr.write(cm.etas, 		prefix+".etas");
			Arr.write(cm.etaVar,    prefix+".etaVar");
		}
		
		if (m.contextModel instanceof ContextModel.ScaleSmoother) {
			ContextModel.ScaleSmoother cm = (ContextModel.ScaleSmoother) m.contextModel;
			Arr.write(cm.frameScales, 	prefix+".frameScales");
			Arr.write(cm.globalCoefs, 	prefix+".globalCoefs");
			Arr.write(Arr.convertToVector(cm.dyadTimeScales), prefix+".dyadTimeScales");
		}
		else if (m.contextModel instanceof ContextModel.FrameSmoother) {
			ContextModel.FrameSmoother cm = (ContextModel.FrameSmoother) m.contextModel;
			Arr.write(cm.globalCoefs, 	prefix+".globalCoefs");
			Arr.write(Arr.convertToVector(cm.betas), prefix+".betas");
			ContextModel.FrameSmoother.DenseEtaTheta d = cm.inferDenseEtaTheta();
			Arr.write(Arr.convertToVector(d.denseEtas), prefix+".denseEtas", 4);
			Arr.write(Arr.convertToVector(d.denseThetas), prefix+".denseThetas", 3);
		}
		else {
			assert false : "todo";
		}
	}
	
	static Index _index;
	public static Index index() {
		return null;
//		if (_index==null) {
//			_index = new Index("pathex/index");
//		}
//		return _index;
	}
	
	public static void display(MainModel m) {
		for (int f=0; f < m.numFrames; f++) {
			U.pf("\n\n====== f=%d\n", f);
			int[] tops = Util.topKIndices(Arr.getCol(m.cPathFrame, f), m.opts.numTopPaths);
			
			for (int path : tops) {
				U.pf("%s count=%.1f\n", m.pathVocab.name(path), m.cPathFrame[path][f]);			
			}

			int[] tupleSample = null;
//			tupleSample = Util.topKIndices(Util.getCol(frameFields, f), opts.numExampleSentences);
			
//			if (m.frameIndicators != null) {
//				assert m.frameIndicators.length == m.eventTuples.size();
//				List<Integer> indsWithFrame = Arr.where(m.frameIndicators, f);
//				
//				int sampleSize = Math.min(2, indsWithFrame.size());
//				tupleSample = new int[sampleSize];
//				int indSample[] = new int[sampleSize];
//				ArrayMath.sampleWithoutReplacement(indSample, indsWithFrame.size());
//				for (int ii=0; ii < indSample.length; ii++) {
//					tupleSample[ii] = indsWithFrame.get(indSample[ii]);
//				}
//			}
//			
//			for (int i : tupleSample) {
//				MainModel.EventTuple t = m.eventTuples.get(i);
//				String d = m.contextVocab.name(t.context);
//				U.pf("\n");
//				if (m.frameFields != null) U.pf("conf %.3f\n", m.frameFields[i][f]);
//				
//				try {
//					TopDocs hits = index().doQuery(t.docid, t.sentid, d.split(" ")[1], d.split(" ")[2]);
//					index().printTupleHits(hits);
//				} catch (IOException e) {
//					U.p("IOException on index, continuing anyway");
//				}
//			}
		}
		
	}
	


	public static void main(String args[]) throws Exception {		
		String mode = args[0];
		String modelFilename = args[1];
		String outputPrefix = modelFilename.replace(".ser.gz", "");
		MainModel m = (MainModel) BasicFileIO.readSerializedObject(modelFilename);
		if (mode.equals("dump"))
			modelTextDump(m, outputPrefix);
		else if (mode.equals("show"))
			display(m);
		else assert false : "wtf";
	}

}
