
import java.io.File;
import java.io.IOException;

//import org.apache.lucene.analysis.Analyzer;
//import org.apache.lucene.analysis.standard.StandardAnalyzer;
//import org.apache.lucene.document.Document;
//import org.apache.lucene.document.Field;
//import org.apache.lucene.document.StringField;
//import org.apache.lucene.document.TextField;
//import org.apache.lucene.index.DirectoryReader;
//import org.apache.lucene.index.IndexReader;
//import org.apache.lucene.index.IndexWriter;
//import org.apache.lucene.index.IndexWriterConfig;
//import org.apache.lucene.index.IndexWriterConfig.OpenMode;
//import org.apache.lucene.index.Term;
//import org.apache.lucene.search.BooleanClause;
//import org.apache.lucene.search.BooleanQuery;
//import org.apache.lucene.search.IndexSearcher;
//import org.apache.lucene.search.ScoreDoc;
//import org.apache.lucene.search.TermQuery;
//import org.apache.lucene.search.TopDocs;
//import org.apache.lucene.store.FSDirectory;
//import org.apache.lucene.util.Version;

import util.BasicFileIO;
import util.U;

public class Index {
//	private IndexReader _indexReader;
//	private Analyzer _analyzer;
//	
//	String indexDir;
//	public Index(String indexDir) {
//		this.indexDir = indexDir;
//	}
//	
//	public IndexReader reader() throws IOException {
//		if (this._indexReader==null) {
//			this._indexReader = DirectoryReader.open(FSDirectory.open(new File(indexDir)));
//		}
//		return this._indexReader;
//	}
//
//	public IndexSearcher searcher() throws IOException {
//		return new IndexSearcher(reader());
//	}
//	
//	public Analyzer analyzer() {
//		if (_analyzer==null)
//			_analyzer = new StandardAnalyzer(Version.LUCENE_40);
//		return _analyzer;
//	}
//	
///////////////////////////////////////
//	
//	public TopDocs doQuery(String docid, String sentid, String srcCode, String tgtCode) throws IOException {
//		BooleanQuery query = new BooleanQuery();
//		query.add(new TermQuery(new Term("docid", docid)), BooleanClause.Occur.MUST);
//		query.add(new TermQuery(new Term("sentid", sentid)), BooleanClause.Occur.MUST);
//		if (srcCode != null)
//			query.add(new TermQuery(new Term("srcCode", srcCode)), BooleanClause.Occur.MUST);
//		if (tgtCode != null)
//			query.add(new TermQuery(new Term("tgtCode", tgtCode)), BooleanClause.Occur.MUST);
//				
//		IndexSearcher searcher = new IndexSearcher(reader());
//		return searcher.search(query, 10);
//	}
//
//	public void printTupleHits(TopDocs hits) {
//		for (ScoreDoc d : hits.scoreDocs) {
//			Document lDoc;
//			try {
//				lDoc = reader().document(d.doc);
//			} catch (IOException e) {
//				e.printStackTrace();
//				continue;
//			}
//			String toks[] = lDoc.get("senttext").split(" ");
//			
//			int s = Integer.parseInt(lDoc.get("srcTokID"));
//			int t = Integer.parseInt(lDoc.get("tgtTokID"));
//			
////				U.pf("\n");
//			U.pf("%s\t%s %s:%d\t%s %s:%d\n", U.red(lDoc.get("path")), 
//					lDoc.get("srcCode"), toks[s], s,
//					lDoc.get("tgtCode"), toks[t], t);
//			U.pf("%s [%s %s]\n", lDoc.get("senttext"), lDoc.get("docid"), lDoc.get("sentid"));
//					
//			
////				U.pf("=== %s\t%s\t%s\n", lDoc.get("docid"), lDoc.get("sentid"), lDoc.get("senttext"));
////				U.pf("%s\n", lDoc.get("pairtext"));
////			U.pf("\t%s %s\t%s %s\t%s\n", 
////					lDoc.get("srcCode"), lDoc.get("srcTokID"), 
////					lDoc.get("tgtCode"), lDoc.get("tgtTokID"),
////					lDoc.get("path"));
//		}
//	}
//	
//	
//	
///////////////////////////////////////
//	
//
//	public void doIndexing(String pathFilename) throws IOException {
//		Analyzer analyzer = new StandardAnalyzer(Version.LUCENE_40);
//		IndexWriterConfig iwc = new IndexWriterConfig(Version.LUCENE_40, analyzer);
////		iwc.setCodec(new SimpleTextCodec());
//		
//		iwc.setOpenMode(OpenMode.CREATE_OR_APPEND);
//		iwc.setRAMBufferSizeMB(500.0);
//		IndexWriter writer = new IndexWriter(FSDirectory.open(new File(indexDir)), iwc);
//		
//		indexPathExtraction(writer, pathFilename);
//		writer.close();
//	}
//
//	private void indexPathExtraction(IndexWriter writer, String inputFilename) throws IOException {
//		U.pf("Indexing %s... ", inputFilename); System.out.flush();
//		
//		int nSent=0, nTuple=0;
//		
//		String curDocID=null, curSentID=null, curSentText=null;
////		String curSentPathsDump = "";
//		for (String line : new BasicFileIO.LineIter(BasicFileIO.openFileToReadUTF8(inputFilename))) {
//			if (line.trim().isEmpty()) continue;
//			if (line.startsWith("===")) {
//				nSent++;
//				// === APW_ENG_20010101.0027       S40     Human rights groups pushed Clinton on Friday to sign the treaty .
//				String parts[] = line.split("\t");
//				curDocID = parts[0].split(" ")[1];
//				curSentID= parts[1];
//				curSentText = parts[2];
//				
////				curSentPathsDump += line + "\n";
//			} else if (line.startsWith("PAIR")) {
//				nTuple++;
//				//PAIR    NGO groups:2    USAELI Clinton:4        |nsubj->|push/VERB|dobj,<-
////				curSentPathsDump += line + "\n";
//				
//				String parts[] = line.split("\t");
//				String 	srcCode = parts[1].split(" ")[0];
//				String[] srcInfo = parts[1].split(" ")[1].split(":");
//				int 	srcTokID = Integer.parseInt(srcInfo[srcInfo.length-1]);
//
//				String 	tgtCode = parts[2].split(" ")[0];
//				String[] tgtInfo = parts[2].split(" ")[1].split(":");
//				int 	tgtTokID = Integer.parseInt(tgtInfo[tgtInfo.length-1]);
//				
//				String path = parts[3];
//				
//				assert curDocID != null;
//				
//				// index the path instance
//
//				Document lDoc = new Document();
//				lDoc.add(new StringField("docid", curDocID, Field.Store.YES));
//				lDoc.add(new StringField("sentid", curSentID, Field.Store.YES));
//				lDoc.add(new TextField("senttext", curSentText, Field.Store.YES));
//				
//				lDoc.add(new StringField("pairtext", line, Field.Store.YES));
//				
//				lDoc.add(new StringField("srcCode", srcCode, Field.Store.YES));
//				lDoc.add(new StringField("srcTokID", Integer.toString(srcTokID), Field.Store.YES));
//				lDoc.add(new StringField("tgtCode", tgtCode, Field.Store.YES));
//				lDoc.add(new StringField("tgtTokID", Integer.toString(tgtTokID), Field.Store.YES));
//				
//				lDoc.add(new StringField("path", path, Field.Store.YES));
//				
//				writer.addDocument(lDoc);
//			}
//			
//		}
//		
//		U.pf("%d sentences, %d tuples\n", nSent, nTuple);
//	}
//	
//	
//	public static void main(String[] args) throws Exception {
//		Index index = new Index(args[1]);
//		if (args[0].equals("index")) {
////			Index index = new Index(args[1]);
//			for (int i=2; i < args.length; i++) {
//				String file = args[i];
//				index.doIndexing(file);				
//			}
//			
//		} else if (args[0].equals("query")) {
//			index.doQuery(args[2], args[3], null, null);
//			
//		} else {
//			assert false;
//		}
//	}
	

}
