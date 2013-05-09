import java.io.Serializable;
import java.util.Properties;

public class RuntimeOptions implements Serializable {
	static final long serialVersionUID = -1L;

	// Runtime options. Debug-friendly defaults for extremely short runtime.  Please override in properties.
	int maxIter 			= 100;
	int saveEvery 			= 50;
	int displayEvery		= 50;
	int trackEvery 		  	= 10;
	int concResampleEvery	= 50;
	int numSamplesForDisplay= 1;
	int burnin = -1;
	
	int phaseGlobalEnd = -1;
	int phaseScaleEnd  = -1;
	String frozenTopicsFile = null;
	boolean frozenTopics() { return frozenTopicsFile != null; }
	
	
	int numTopPaths = 10;
	int numExampleSentences = 2;
	
	String pathFile;
	String outputDir;
	String dateFile;
	
	public void readProperties(Properties properties) {
		pathFile = properties.getProperty("pathFile");
//		outputDir = properties.getProperty("outputDir");
		dateFile = properties.getProperty("dateFile");
		
		if (properties.containsKey("maxIter")) {
			maxIter = (int) Math.round(Double.valueOf(properties.getProperty("maxIter")));
		}
		if (maxIter % 10 == 0) maxIter += 1;
		if (properties.containsKey("burnin")) {
			burnin = (int) Math.round(Double.valueOf(properties.getProperty("burnin")));
		}
		if (properties.containsKey("saveEvery")) {
			saveEvery = (int) Math.round(Double.valueOf(properties.getProperty("saveEvery")));
		}
		if (properties.containsKey("displayEvery")) {
			displayEvery = (int) Math.round(Double.valueOf(properties.getProperty("displayEvery")));
		}
		if (properties.containsKey("numSamplesForDisplay")) {
			numSamplesForDisplay = (int) Math.round(Double.valueOf(properties.getProperty("numSamplesForDisplay")));
		}
		if (properties.containsKey("concResampleEvery")) {
			concResampleEvery = (int) Math.round(Double.valueOf(properties.getProperty("concResampleEvery")));
		}
		if (properties.containsKey("phaseGlobalEnd")) {
			phaseGlobalEnd = (int) Math.round(Double.valueOf(properties.getProperty("phaseGlobalEnd")));
		}
		if (properties.containsKey("phaseScaleEnd")) {
			phaseScaleEnd = (int) Math.round(Double.valueOf(properties.getProperty("phaseScaleEnd")));
		}
		if (properties.containsKey("frozenTopicsFile")) {
			String filename = ((String) properties.get("frozenTopicsFile")).trim();
			if (!filename.isEmpty()) {
				frozenTopicsFile = filename;
			}
		}
	}
}
