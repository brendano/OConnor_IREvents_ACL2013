package struct;

import util.U;

/** This lives with one sentence **/
public class Mention {
	public int headToken;
	public int origHeadToken;
	public int startToken;
	public int endToken;
	public String actorCode;
	public String toString() {
//		return U.sf("[%d %d %d]", headToken, startToken, endToken);
		return U.sf("[%d %d %d %s]", headToken, startToken, endToken, actorCode);
	}
	
}
