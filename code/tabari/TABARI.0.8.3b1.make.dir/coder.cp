// 																																		TABARI Project
//___________________________________________________________________________________
// 																																	 			coder.cp

//  This file contains the routines for coding events from the parsed text

//  Notes:
//  1. eventArray is filled sequentially until expandCompounds() is called, at which
//     point it should be tranversed as a list using the nextEvt field.
//
//	2. Order that various contingencies are handled:
//			a. Expand paired events							makeEvent()=>putEvent()
//			b. Duplicate events for compound phrases				makeEvent()
//			c. Resolve subordinant and dominant events			evaluateEvents()
//			d. Expand coded compound actors						expandCompoundCodes()
//			e. Eliminate duplicates							checkEventDups()
//			f. Eliminate events where source = target				checkSameActors()
//
//	3. Syntactic agents are currently not implemented (coded agents are)
//
//  ISSUES
//	1.The information about the issues to be coded is stored in a linked-list
//		of issueHeadStructs that begins at issueHead.  See the header file for the contents.
//
//	2.The codes themselves are stored in a linked lists of issueListStruct that come off 
// 	  the plist pointer in the header.	This grows as needed (it will only have a single
//	  element except when TYPE = "ALL" is used.  Rather than continually re-allocating
//    this list, it is just zeroed out at the end of coding each record and re-used; 
//    so when the program terminates, it is the size of the largest number of codes
//    found.  This is a slight waste of memory but it is exceedingly unlikely that 
//    these lists will become sufficiently large as to threaten the program.
//
//	3.The "issues" code string is generated once, then concatenated to all of the
//		records generated from the text. 
 
//__________________________________________________________________________________
//
//	Copyright (c) 2002-2012  Philip A. Schrodt.  All rights reserved.
//
// 	Redistribution and use in source and binary forms, with or without modification,
// 	are permitted under the terms of the GNU General Public License:
// 	http://www.opensource.org/licenses/gpl-license.html
//
//	Report bugs to: schrodt@psu.edu

//	The most recent version of this code is available from the KEDS Web site:
//		http://eventdata.psu.edu

//	For plausible indenting of this source code, set the tab size in your editor to "2"

//___________________________________________________________________________________
// 																																						Headers

#include "TABARI.h"

	CoderClass Coder;

//___________________________________________________________________________________
// 																																   Global Variables

extern TabariFlagsClass TabariFlags;
extern TokenStoreClass 	TokenStore;
extern LiteralsClass 		Literals;
extern RootsClass 			Roots;
extern PhrasesClass 		Phrases;
extern ReadFilesClass		ReadFiles;		
extern CodeStoreClass 	CodeStore; 
extern ParserClass			Parser;		
extern ProcessorClass		Processor;		

//___________________________________________________________________________________
//																															actorStruct utilities

bool CoderClass:: actorEqual(actorStruct a, actorStruct  b)
{
	if ((a.actor == b.actor) && 
			(a.agent == b.agent) && 
			(a.index == b.index)) return true;
	else return false;
} // actorEqual

bool CoderClass:: actorEmpty (actorStruct a)
{
	if (!a.actor && !a.agent && !a.index) return true;
	else return false;
} // actorEmpty

void CoderClass:: zeroActor (actorStruct &a)
// set both elements of a to zero
{
	a.actor = 0;
	a.agent = 0;
	a.index = 0;
	a.where = 0;
	a.rootidx = 0;
	a.agtidx  = 0;
} // zeroActor

void CoderClass:: setActor (actorStruct &a, actorStruct  b)
// set a = b
{
	a.actor = b.actor;
	a.agent = b.agent;
	a.index = b.index;
	a.where = b.where;
	a.rootidx = b.rootidx;
	a.agtidx  = b.agtidx;
} // setActor

//___________________________________________________________________________________

void CoderClass:: setcodeMode(char *s)
// sets the coding mode (called from .options)
{
	if (strstr(s,"SEN")) codeMode = 1;				// code one event per sentence
	else 	if (strstr(s,"ALL")) codeMode = 2;	// code all verbs
	else codeMode = 0;												// code one event per clause
	
} // setCodeMode
	
void CoderClass:: coderError(const char *s, int ierror)
// Write the error to <errorfile>
{
	Processor.writeError("Coder error: ", (char *)s, ierror);
}  // coderError

bool CoderClass:: setClauseBounds(void)
//	set the conjunction boundaries istartClause and iendClause.  If iendClause = -1,
//  checks from the beginning of sentence; otherwise sets to the next clause 
//	after iendClause.  Returns false if there are no more clauses.
//	Note that there is no bounds checking because syntArray[0] and syntArray[iLex]
//	always have tags
{
	if (!Parser.fhasClauses || TabariFlags.fIgnoreConj) {		// set bounds at 0 and iLex
		if (iendClause >=0 ) return false;			// we've already checked this one
		else {
			istartClause = 0;
			iendClause   = Parser.iLex;
			return true;
		}
	}		
																	// find next clause boundaries
	if (iendClause >= 0) {
		istartClause = iendClause + 1;
		while ((istartClause <= Parser.iLex) && 
					 (!Parser.hasHeadTag(istartClause,Clause)))	++istartClause;
		if (istartClause > Parser.iLex) return false;		// no more clauses
		if (iendClause > Parser.iLex) coderError("setClauseBounds1: iendClause > Parser.iLex",CLAUSE_END_ERROR);
	}
	else istartClause = 0;
	
	iendClause = istartClause;
	while (!Parser.hasTailTag(iendClause,Clause)) ++iendClause;
	if (iendClause > Parser.iLex) coderError("setClauseBounds2: iendClause > Parser.iLex",CLAUSE_END_ERROR);
	return true;

} // setClauseBounds

void CoderClass:: getActorStructCodes(actorStruct &actFound, int loc )
// attach codes to actFound based on the root at loc. 
{
tokptr pt = NULL;
toktype istart;

	while (Parser.hasHeadTag(loc,Pronoun)) {  // this was a deferenced pronoun, so shift loc
		loc = Parser.getReference(loc);					// "while" handles multiple dereferencing
	}

	if (!Parser.syntArray[loc].iroot) { 
		Parser.parseError((char *)"empty actor root code",ACTOR_CODE_ERROR);
		zeroActor(actFound);
		return;
	}
	actFound.rootidx = Parser.syntArray[loc].iroot;
	istart = Roots.rootArray[Parser.syntArray[loc].iroot].ppat->icode;
	CodeStore.nextActorCode (actFound.actor, actFound.agent, &pt, istart);
	if (pt) actFound.index = istart;  // location of a complex code; otherwise zero
 	else actFound.index = 0; 
 	actFound.where = loc;
} // getActorStructCodes

void CoderClass:: findActor (int loc, int &locFound, actorStruct &actFound)
// Find the beginning of next actor in syntArray with index >= loc; 
// returns location and code if found; locfound is -1 if not found
{
bool found = false;

	while ((loc <= iendClause) && !found) {   // look for actor
		if ((Actor == Parser.syntArray[loc].wtype)  &&
				 Parser.hasHeadTag(loc,Actor)) {  // check that actor *begins* after the verb
			found = true;
			getActorStructCodes(actFound,loc);
			findAgent(loc, actFound); 
		}
		else ++loc;
	}

	if (found) 	locFound = loc;
	else 				locFound = -1;

} // Find_Actor 

void CoderClass:: findAgent (int loc, actorStruct &actFound)
// Find the beginning of an actor immediately adjacent to loc, checking after first, then prior 
// returns head location and code if found; locfound is -1 if not found
{
bool found = false;
int tloc =  getActorTail(loc);
bool hasFormer = false;

	// check for "FORMER" constructions
	if ((Adjctv == Parser.syntArray[tloc+1].wtype)  &&          // check for 'former' after actor 
			(Roots.rootFORMER == Parser.syntArray[tloc+1].iroot)) {  
			hasFormer = true;
			++tloc;
	}
	if ((loc>0) && (!hasFormer) &&
			(Adjctv == Parser.syntArray[loc-1].wtype)  &&          // check for 'former' prior to actor 
			(Roots.rootFORMER == Parser.syntArray[loc-1].iroot)) {  
			hasFormer = true;
	}

 // check for agent following actor
	if ((Agent == Parser.syntArray[tloc+1].wtype)  &&
			 Parser.hasHeadTag(tloc+1,Agent)) {  // check for agent after actor 
		found = true;
		actFound.agtidx = Parser.syntArray[tloc+1].iroot;
//		Processor.fprob << "fAgt:Mk1 " << actFound.agtidx << endl;
		if (hasFormer) actFound.agent = CodeStore.codeElite;
		else actFound.agent = Roots.rootArray[Parser.syntArray[tloc+1].iroot].ppat->icode;
	}
	else {  // check for agent prior to actor
		if (Prep == Parser.syntArray[loc-1].wtype) loc--;  // optional skip over preposition
		if ((Agent == Parser.syntArray[loc-1].wtype)  &&
				 Parser.hasTailTag(loc-1,Agent)) {  // check for agent prior to actor
																						// this code is a modified version of getActorHead
			int istart = loc - 1;

			while ((istart>=0) && (!Parser.hasHeadTag(istart,Agent))) --istart;
			if (istart >= 0) {
				found = true;
				actFound.agtidx = Parser.syntArray[istart].iroot;
				if ((istart>0) && (!hasFormer) &&
						(Adjctv == Parser.syntArray[istart-1].wtype)  &&          // check for 'former' prior to actor 
						(Roots.rootFORMER == Parser.syntArray[istart-1].iroot)) {  
						hasFormer = true;
				}
				if (hasFormer) actFound.agent = CodeStore.codeElite;
				else actFound.agent = Roots.rootArray[Parser.syntArray[istart].iroot].ppat->icode;
				// 	actFound.agloc = istart;
			}
		}
	}

	if (!found) {  // probably redundant if initialization wasn't wiped out but let's be sure...
		actFound.agent = 0; 
		actFound.agtidx = 0;
	}
} // findAgent 

void CoderClass:: backActor (int loc, int &locFound, actorStruct &actFound)
// Find the beginning of next actor in syntArray with index <= loc; 
// returns location of initial word and code if found; zero if not found
{
bool found = false;

	while ((loc >= istartClause) && !found) {    // look for an actor
		if (Actor == Parser.syntArray[loc].wtype)	{ 
			loc = getActorHead(loc,found); // adjust in case we backed into this
			if (!found) {   // headtag may have been deactivated in nullSubord
				locFound = -1;
				return;
			}
			getActorStructCodes(actFound,loc);
			findAgent(loc, actFound); 
		}
		else --loc;
	}
	if (found) 	locFound = loc; 
	else 				locFound = -1;

} // backActor 

int CoderClass:: getActorHead(int loc, bool &fmatch)
// moves the entire actor root into temp; returns location of root head;
// set fmatch = true.
// This does not do error checking -- it assumes the tags exist
// 03.04.20: however, looks like there is a bunch of asserts and bounds checking
//           in the 'do' block remaining from some earlier debugging [pas]
{	
int istart, ka;

	if (Parser.hasHeadTag(loc,Pronoun)) {  // this was a deferenced pronoun, so shift loc
		loc = Parser.getReference(loc);
	}
	istart = loc;
	while ((istart>=0) && (!Parser.hasHeadTag(istart,Actor))) --istart;
	if (istart<0) {
		fmatch = false;
		return loc;
	}		

	ka = istart-1; // transfer actor indices to tempArray
	do {
		++ka;
		tempArray[iTemp++] = ka;
		if ((ka > Parser.iLex) || (iTemp >= MAX_TEMP))	{
			if (iTemp >= MAX_TEMP) {
				coderError("Excessive words between nonconsecutive elements of a phrase",MAX_PHRASE_ERROR);
				break; // ### <09.11.12> this seems to be the correct way out, but haven't analyzed it fully. Earlier code is below, but I'm not sure it was ever invoked
//				fmatch = false;
//				return loc;
			}
			WriteAlert("Coder: gAH bounds error"); // this indicates a tagging error: shouldn't hit it
			Pause();  // ### probably should drop this... <09.11.12>
			break;
		}		
    } while (!Parser.hasTailTag(ka,Actor));

	fmatch = true;
	return istart;
} // getActorHead

int CoderClass:: getActorTail(int loc)
// Returns location of actor root tail;
// This does not do error checking -- it assumes the tail tag exists
{	
	if (Parser.hasHeadTag(loc,Pronoun)) {  //  deferenced pronoun, so tail is same as head
		return loc;
	}
	while ((loc < iendClause) && (!Parser.hasTailTag(loc,Actor))) ++loc;
	return loc;
} // getActorTail

int CoderClass:: getCompound(int loc, bool &fmatch)
// moves the entire compound phrase into temp; returns location of phrase head;
// Version called from checkPattern; sets fmatch = true. 
// Returns -1, which flags the absence of a compound, if corresponding tags don't exist or don't correspond 
{	
int ka;
int istart;  
	
	if (Parser.hasHeadTag(loc, Compound)) {  // checking forward from the head
		if (!Parser.checkTailTag(loc,Compound)) return -1;  // no corresponding tail tag
		istart = loc; 
	}
	else { // check back from the tail 
		istart= getCompound(loc); // calling a different variant of the function, not recursive
		if (istart < 0) return -1; // no corresponding head tag
	}

	ka = istart-1; // transfer indices of compound phrase to tempArray
	do {
		++ka;
		if (iTemp >= MAX_TEMP) break;  // just ignore the rest of the match
		tempArray[iTemp++] = ka;			
	} while (!Parser.hasTailTag(ka,Compound));  // no need to check indices; we know tags are okay

	fmatch = true;
	return istart;
} // getCompound

int CoderClass:: getCompound(int loc)
// returns location of phrase head only;version called from makeEvent
// Returns -1 if no corresponding tag can be found
{	
int istart = loc;
toktype tagidx;

//	Parser.writeParsing("Called from getCompound");  // debug
	tagidx = Parser.getTailTag(loc, Compound);
	if (!tagidx) return -1;
	while ((istart>=0) && (!Parser.checkHeadTag(istart,Compound,tagidx))) --istart;
	return istart;

} // getCompound

void CoderClass:: saveState(void)
// save prior to evaluating synset or conditional	
{  
	fcond = true;
	condloc =  locSynt;
	condtemp = iTemp;
	condlen  = lenalt;
	condfskip = fskipMode;
} // saveState

void CoderClass:: restoreState(void)
// save conditional state	
{  
//	fcond = false;  // this is probably redundant as this is reset elsewhere
	locSynt = condloc; 	// restore conditional state
	lenalt = condlen;
	iTemp = condtemp;
	fskipMode = condfskip;
} // restoreState

void CoderClass:: initCondit(void)
// save conditional state	
{  
	saveState();
    if (isHigh) ptPat += 2; // skip connector; go to next literal
	else ptPat -= 2;
} // initCondit

void CoderClass:: condCheck(int &state)
// see if we have alternatives in conditional if current option failed
{
	if (isHigh) {
		while ((OR_LIT != *ptPat) && (CLBRACK_LIT != *ptPat)) ptPat += 2;  // move past current pattern ### bounds check here? also could jump by += 2
		if (CLBRACK_LIT == *ptPat) return;  // last option, so match failed
	}
	else {
		while ((OR_LIT != *ptPat) && (OPBRACK_LIT != *ptPat)) ptPat -= 2;  // move past pattern ### bounds check here?
		if (OPBRACK_LIT == *ptPat) return;  // last option, so match failed
	}
    restoreState();
	if (isHigh) ptPat += 2; // skip connector; go to next literal
	else ptPat -= 2;
	if (*(ptPat+1) & Phrases.connPartNext) fullMode = false;// set match mode using new pattern
	else fullMode = true;
	state = 2;  // and keep trying!
} // condCheck

int CoderClass:: compSynset(void) 
// Compares the synset patterns at rootArray[*ptPat] with syntArray location locSynt;
// returns same values as compTarget [below]. 
{
    int		match = 0;
    patStruct *pPat = Roots.rootArray[*ptPat].ppat;

    pPat = pPat->pnext; // go to start of list
    while (pPat) {
#if DEBUG_PAT
        {instring s;			// *** debug
        toktype * ptok = pPat->phrase;
        Processor.fprob << "compSynt: " << Phrases.get(s, pPat->phrase) << endl; // *** debug: this is the phrase we are trying to match
        Processor.fprob  << "Phrase list :" << endl;
        while (*ptok) {
            Processor.fprob  << "  " << *ptok << ": " << Literals.litArray[*ptok].pchar << " -- " << *(++ptok) << endl;  // get the text and the value of the connector
            ++ptok;
        }
        }
#endif
        if ((match = checksubPattern(pPat))) {  // deliberation use of assignment
//             if (iTemp < MAX_TEMP) tempArray[iTemp++] = locSynt;  // ### doesn't want this, for some reason...phrase extraction is working fine <12.01.20>
//             if (fcond) lenalt += strlen(Literals.litArray[*ptPat].pchar);  // update length of matched conditional
//            Processor.fprob  << "Match succeeded" << endl;
            return match;
        }
        pPat = pPat->pnext;
    }
    return 0;
} // compSynset

int CoderClass:: checksubPattern(patStruct *pPat) 
// check for a subpattern in synset
{
    int 	result = 0;
    int 	state = 2;
	tokptr   saveptPat = ptPat;		// pointer to current pattern location
	tokptr   saveptThisPat = ptThisPat;	// pointer to pattern being evaluated
	int     savelocSynt = locSynt;		// current text location
    
	ptThisPat = ptPat = pPat->phrase;
#if DEBUG_PAT
    instring s;			// *** debug
    Processor.fprob << "subPat >> Phrase: " << Phrases.get(s, pPat->phrase) << endl;
#endif
    
	if (isHigh) {
		while (2 == state) {
			if (Phrases.flagSynm & *(ptPat+1)) {
//                Processor.fprob << "subPat >> evaluating synset: " << Phrases.get(s, pPat->phrase) << endl;                
                result = compSynset();
            }
        	else result = compTarget();
			state = stateCheck(result);
		}
        ptPat = saveptPat;	
        ptThisPat = saveptThisPat;	      
        locSynt = savelocSynt;
		if (state) return 2;
        else return 0;
	}
	else { // lower phrase
		while (2 == state) {
			if (Phrases.flagSynm & *(ptPat+1)) {
				int docheck = 1;
				saveState();
				while (docheck){ // deal with possibility of multiple words in synset patterns
					result = compSynset();
					if (!result) docheck = continueCheck();
					else docheck = 0;
				}
				if (!result) {
					fcond = false;
					restoreState();
				}
			} 
        	else result = compTarget();
			state = stateCheck(result);
		}
        ptPat = saveptPat;	
        ptThisPat = saveptThisPat;	      
        locSynt = savelocSynt;
		if (state) return 2;
        else return 0;
	}
} // checksubPattern

int CoderClass:: compTarget(void) 
// Compares current pattern target *ptPat with syntArray location locSynt;
// returns 
//      0   if match failed  
//      1   if partial match  
//      2   if exact match  (also returned for actors)
//
//	When a $,+ or % actor is matched, the match records the entire root, but skips only the
//	first literal rather than the entire root.  This allows a phrase to contain words that are
//	part of a root, which KEDS did not.  If the skip token ^ is used, the entire root
//  is passed over.
{
bool  fmatch = false;
int		match = 0;

#if DEBUG_PAT
	Processor.fprob << "compTarget: " << Parser.lexArray[locSynt].ptext << endl;
#endif

	if (*ptPat >= SYMBOL_LIMIT) {  // check for match of a literal
		if ((match = Parser.hasLitr(locSynt, *ptPat))) {  // deliberation use of assignment
			if (iTemp < MAX_TEMP) tempArray[iTemp++] = locSynt;  // if bounds overflow, just ignore 
			if (fcond) lenalt += strlen(Literals.litArray[*ptPat].pchar);  // update length of matched conditional
#if DEBUG_PAT
			Processor.fprob << " cT match: " << match << endl;
#endif
			return match;
		}
		else return 0;
	}
	else if ((SOURCE_LIT == *ptPat) && (Actor == Parser.syntArray[locSynt].wtype)) { 
		indexSource = getActorHead(locSynt,fmatch);
	}
	else if ((TARGET_LIT == *ptPat) && (Actor == Parser.syntArray[locSynt].wtype)) { 
		indexTarget = getActorHead(locSynt,fmatch);
	}
	else if ((ATTRIB_LIT == *ptPat) && (Actor == Parser.syntArray[locSynt].wtype)) { 
		indexAttrib = getActorHead(locSynt,fmatch);
	}
	else if ((SKIP_LIT == *ptPat) && (Actor == Parser.syntArray[locSynt].wtype)) { 
		int iscr = iTemp;
		if (isHigh) {
			getActorHead(locSynt,fmatch);
			if (fmatch) locSynt = getActorTail(locSynt);
		}
		else locSynt = getActorHead(locSynt,fmatch); // move past the actor
		iTemp = iscr;  // don't store skipped text
	}
	else if ((COMPOUND_LIT == *ptPat) && (Parser.hasHeadTag(locSynt, Compound))) {
		indexCompound = getCompound(locSynt, fmatch);
	}
	if (fmatch) return 2;
	else return 0;
} // compTarget

int CoderClass:: continueCheck(void)
// move in text without changing target and mode
// returns 0 at end of text, 2 otherwise  
{
	if (isHigh) {
		++locSynt;
		if (locSynt > iendClause) return 0;
	}
	else {
		--locSynt;
		if (locSynt < 0) return 0;
	}
#if DEBUG_PAT
	Processor.fprob << "cC: locSynt " << locSynt << endl;
#endif
	return 2;
} // continueCheck

int CoderClass:: nextCheck(void) 
// increment text and pattern: change target and mode
// returns 
//	  0   end of text   
//    1   end of pattern  
//    2   otherwise
{
	if (isHigh) {
		if (fcond) {
			if ((OR_LIT == *(ptPat+2) || (CLBRACK_LIT == *(ptPat+2)))) {  // skip remaining pattern if at end of internal pattern
				fcond = false;
				while (CLBRACK_LIT != *ptPat) ++ptPat;  // move to close of pattern ### increment by 2?
			}
		}
//        Processor.fprob << "nC: ptPat conn " << *(ptPat+1) << " w/mask " << (*(ptPat+1) & Phrases.connFullSkip) << endl;
		if (*(ptPat+1) & Phrases.connFullSkip) fskipMode = true;	// set skip mode using current pattern
		else fskipMode = false;
		ptPat += 2;
		if (!*ptPat) return 1; // end of pattern
		++locSynt;
//        Processor.fprob << "nC: locSynt " << locSynt << " phrase element " << (ptPat - ptThisPat) << endl;
		if (locSynt > iendClause) return 0; // end of text
	}
	else {
		ptPat -= 2;
		if (ptPat < ptThisPat) return 1; // beginning of pattern
		if (fcond) {
			if ((OR_LIT == *ptPat) || (OPBRACK_LIT == *ptPat)) {  // skip remaining pattern
				fcond = false;
				while (OPBRACK_LIT != *ptPat) ptPat -= 2;  // move to close of pattern 
				ptPat -= 2; 										// move to element prior to conditional
			}
		}
		if (*(ptPat+1) & Phrases.connFullSkip) fskipMode = true;	// set skip mode using new pattern
		else fskipMode = false;
		--locSynt;
		if (locSynt < 0) return 0; // beginning of text
	}
	if (*(ptPat+1) & Phrases.connPartNext) fullMode = false;// set match mode using new pattern
	else fullMode = true;
#if DEBUG_PAT
	Processor.fprob << "nC: locSynt " << locSynt << " phrase element " << (ptPat - ptThisPat) << endl;
#endif
	return 2;	
} // nextCheck

int CoderClass:: stateCheck(int result)
// implements response to result based on mode, then returns state of pattern check 
// returns 
//      0   pattern failed   
//      1   pattern succeeded  
//      2   continue checking
// Note that is translates into a simple lookup table, which might (or might not)
// be faster
{
#if DEBUG_PAT
	Processor.fprob << "stChk: fullMode " << fullMode << " skipMode " << fskipMode << " result " << result << endl;
#endif
	
	if (fskipMode) {
		if (fullMode) {
			if (2 == result) return nextCheck();
			else return continueCheck();
		}
		else {
			if (result) return nextCheck();
			else return continueCheck();
		}
	} 
	else {
		if (fullMode) {
			if (2 == result) return nextCheck();
			else return 0;
		}
		else {
			if (result) return nextCheck();
			else return 0;
		}
	} 
} // stateCheck

bool CoderClass:: checkPattern(patStruct *pPat, int loc, wordtype wtype ) 
//	This is the core routine for checking verb patterns.  It does the following
//		1. Finds the verb marker in the pattern phrase
//		2. Attempts to match the literals following the verb ("upper phrase")
//		3. If successful, attempts to match the literals prior to the verb ("lower phrase")
//       (note that "upper" and "lower" are in reference to the array indices, and
//        unfortunately this is the opposite of the orientation of the parsed display...)
//
//	The matching of the upper phrase begins immediately after the *end* of the verb root.
//	The matching of the lower phrase begins immediately prior to the start of the verb root
//
//	The syntArray indices of the literals in the matched string, including the roots, are 
//	put in the tempArray, with the upper string first, followed by -1, followed by the
//	lower string, followed by -2.  The variables  indexSource, indexTarget, indexAttrib, 
//	and indexCompound can also be set by this.
//
// <09.01.02> ### this probably should not match across clause boundaries, right?
{
tokptr 	pverb;			// save location of verb
int 	result = 0;
int 	state = 2;

#if DEBUG_PAT
    instring s;			// *** debug
    Processor.fprob << "\nchkPat >> Phrase: " << Phrases.get(s, pPat->phrase) << endl;
#endif

	tempArray[0] = loc;  // record verb as first element of tempArray
	iTemp = 1;
    lenalt = 0; 				// length of text matched in alternatives
    fcond = false;  		// doing conditional evaluation?
	ptThisPat = ptPat = pPat->phrase;
	locSynt = loc;

	while (*ptPat && (*ptPat != VERB_LIT)) ptPat +=2; // find the verb in the pattern
	if (!*ptPat) return false;	// no verb in pattern :this should have been caught at input
	pverb = ptPat;									// save value

	while ((Halt != Parser.syntArray[locSynt].wtype) && 
				 (!Parser.hasTailTag(locSynt,wtype))) ++locSynt; // set highloc to the end of the wtype root; 
//	assert(Halt != Parser.syntArray[locSynt].wtype);	// if fails, tail wasn't marked so something is wrong...

// do the upper (forward) match
//    Processor.fprob << "Coder:cP -- upper match\n"; // *** debug
	isHigh = true;
	state = nextCheck();
	while (2 == state) {
        if (Phrases.flagSynm & *(ptPat+1)) {
//            Processor.fprob << "Coder:chkPat -- Synset detected\n"; // *** debug
            result = compSynset();
        } 
        else {
            if (OPBRACK_LIT == *ptPat) initCondit();	
            result = compTarget();
        }
        state = stateCheck(result);
        if (!state && fcond) condCheck(state);
	}
	if (!state) return false;
	
	if (iTemp > MAX_TEMP - 2) {
// 	  Processor.fprob << "cP:Mk1\n";
	  iTemp = MAX_TEMP - 2;  // could occur with very long phrases, so overwrite part of this
	}
	tempArray[iTemp++] = -1;		// mark end of upper phrase
	if (pverb == pPat->phrase) {
		tempArray[iTemp++] = -2;		// mark end of lower phrase
		return true;  						// no lower phrase, so we're done
	}

// Now do the lower phrase...
//    Processor.fprob << "Coder:cP -- lower match\n"; // *** debug
	isHigh = false;
	ptPat = pverb;
	locSynt = loc;
	state = nextCheck();
	while (2 == state) {
        if (Phrases.flagSynm & *(ptPat+1)) {
            int docheck = 1;
            saveState();
            while (docheck){ // deal with possibility of multiple words in synset patterns
                result = compSynset();
                if (!result) docheck = continueCheck();
                else docheck = 0;
            }
            if (!result) {
                fcond = false;
                restoreState();
            }
  //          else            ptPat -= 2;  // skip the synflag and go to the slot containing the connector
        } 
		else {
            if (CLBRACK_LIT == *ptPat) initCondit();	
            result = compTarget();
        }
		state = stateCheck(result);
		if (!state && fcond) condCheck(state);
	}
	if (!state) return false;

	if (iTemp > MAX_TEMP - 1) iTemp = MAX_TEMP - 1;  // could occur with very long phrases, so overwrite part of this
	tempArray[iTemp++] = -2;		// mark end of lower phrase
	return true;
	
} // checkPattern

void CoderClass:: setDefaultSource(void)
// sets theSource to the default
{
//    Processor.fprob << "Got a defsrc\n";
	theSource.rootidx = Roots.rootDefActor;  // DEFAULT_ACTOR root
 	theSource.actor = idxcodeDefSrc; 
 	theSource.agent = 0; 
 	theSource.index = 0; 
 	theSource.where = MAX_SYNT - 1;  // this is the placeholder    
    fRegularSource = true;  // well, sort of regular...signals that we've set a source
} // setDefaultSource

void CoderClass:: setDefaultTarget(void)
// sets the Target to the default
{
//    Processor.fprob << "Got a deftar\n";
	theTarget.rootidx = Roots.rootDefActor; 
 	theTarget.actor = idxcodeDefTar; 
 	theTarget.agent = 0; 
 	theTarget.index = 0; 
 	theTarget.where = MAX_SYNT - 1;  // this is the placeholder    
    fRegularTarget = true;    
} // setDefaultTarget

void CoderClass:: getRegularSource(int index)
// Regular source finder; this is used if the source is not found by a pattern
// index is the location of the verb, which is used if the default source PRIOR option is true
{
int		loc  = istartClause;  // start search from beginning of clause
int 	locfound = 0;       	// forces while loop to execute at least once
bool 	found = false;
actorStruct	iAct;

//    Processor.fprob << "fhasdefsrc: " << fhasDefltSrc << "\n";
	while (!found && (locfound >= 0)) 
	{
		findActor(loc, locfound, iAct);
		if (locfound >= 0) {
            if (fDefPRIOR && (locfound > index)) {  // SOURCE option: only look for actor prior to verb
                setDefaultSource();
                return;
            }
			if (!actorEqual(iAct, theTarget)) {
				setActor(theSource, iAct);
				found = true;
				fRegularSource = true;
 //               Processor.fprob << "found source: " << theSource.actor << " at " << locfound << "\n";
			}
			else loc = locfound + 1;  // already have that one, so try another
		}
	}
	if ((!found) && fhasDefltSrc) setDefaultSource();
    return;

} // getRegularSource 

void CoderClass:: getRegularTarget (int index)
{
int		loc;
int 	locfound = 0;	// forces while loop to execute once
bool 	found = false;
actorStruct	iAct;

	loc = index + 1;   // check after the verb }
	while (!found && (locfound >= 0)) {
		findActor(loc, locfound, iAct);
		if (locfound >= 0) { 
			if (!actorEqual(iAct, theSource)) {
				setActor(theTarget,iAct);
				found = true;
				fRegularTarget = true;
			}
			else loc = locfound + 1;
		}
	}
//	Processor.fprob << "      gDT:Mk1 " << theSource.agtidx << "  " << theTarget.agtidx << endl; // debug
	if (found) return;
    if (fDefAFTER) { // TARGET option: only look for actor prior to verb
        setDefaultTarget();
        return;
    }

	loc = index - 1;         // try looking before the verb
	locfound = 0; 
	while (!found && (locfound >= 0)) {  // ### check this condition
		backActor(loc, locfound, iAct);
		if (locfound >= 0) {
			if (!actorEqual(iAct, theSource)) {
				setActor(theTarget,iAct);
				found = true;
			}
			else loc = locfound - 1;
		}
	}

//	Processor.fprob << "      gDT:Mk2 " << theSource.agtidx << "  " << theTarget.agtidx << endl; // debug
	if (found) return;
	else				                                // if there is a compound actor make that the target
	if ((theSource.index) ||
	    (Parser.syntArray[theSource.where].flags & Parser.setCompd))
	   setActor(theTarget, theSource);
    else if (fhasDefltSrc) setDefaultTarget();

} // getRegularTarget 

bool CoderClass:: checkAttribCode (patStruct * pPat)
// Check for complexity, discard and null codes in attribution patterns.  
// Returns false if null code, true if discard or complex code
{
int ka = TokenStore.getTokenValue(pPat->icode);
	// cout << "cACAe\n";

	if (ka == CodeStore.indexNull) return false ; 
	else if (ka == CodeStore.indexDiscard) {
		Parser.fHasDiscard = true;
		// cout << "cACAx2\n";
		return true;
	}
	else if (ka == CodeStore.indexComplex) {
		Parser.fIsComplex = true;		
		Parser.fCplxStrg  = true;
		strcpy(Parser.sComplex,"Complex code in attribution phrase");
		// cout << "cACAx3\n";
		return true;
	}
	return false; // ### is this correct?? -- should we ever get here? <05.04.14>
} // checkAttribCode

void CoderClass:: getAttribActor (int loc)
// Find the attribution actor for phrase at loc:
// 0. do nothing if this has already been set by a pattern;
// 1. look before the phrase, stopping at a comma
// 2. look for first actor in the sentence
{
bool found = false;
int locfound;

	if(!actorEmpty(theAttrib)) return; // attribution already set by a pattern

	while ((loc >= istartClause) && 
				 (Comma != Parser.syntArray[loc].wtype) && !found) {    // look for an actor
		if (Actor == Parser.syntArray[loc].wtype)	{ 
			loc = getActorHead(loc,found); // adjust in case we backed into this
			if (found) getActorStructCodes(theAttrib,loc);
			else --loc;
		}
		else --loc;
	} // while
	if (found) return;  // got it before the phrase, so we're done
											// otherwise look from the beginning of the sentence
	iendClause   = Parser.iLex;// consider entire sentence
	findActor(0, locfound, theAttrib);

} // getAttribActor 

void CoderClass:: doAttrib(void)
// Check for attribution  
// Stops at the first non-null-coded phrase encountered 
{
int index = Parser.iLex;  // check from the end of the sentence
patStruct *	pPat;
// instring s;			// *** debug

	zeroActor(theAttrib);
	while (index >= 0) {
		if (Parser.hasTailTag(index,Attrib)) { 
			while ((index>=0) && (!Parser.hasHeadTag(index,Attrib))) --index;  // find head of phrase
			if (index < 0) {
				coderError("No head tag on attribution phrase",HEAD_ATTRIB_ERROR);  // should not hit this...
				return;
			}		

			pPat = Roots.rootArray[Parser.syntArray[index].iroot].ppat;
//			cout << "\nDA:cT1: " << Phrases.get(s, Roots.rootArray[Parser.syntArray[index].iroot].phrase) << endl;

			while (pPat->pnext) {		// go through phrases
				pPat = pPat->pnext;
//				 cout << "DA:cT2: " << Phrases.get(s, pPat->phrase) << endl;
				if (checkPattern(pPat, index, Attrib)) {  // pattern matches
					++pPat->used;
					if (checkAttribCode(pPat)) return;  // complex or discard code
					if (indexAttrib>=0) getActorStructCodes(theAttrib, indexAttrib); // attrib was set in a pattern
					else getAttribActor(index);		// get the actor using default rules
					// cout << "Attribution located at " << theAttrib.where << endl;
					return;
				} // if
			} // while

			// if no match, try the main phrase
			if (Roots.rootArray[Parser.syntArray[index].iroot].ppat->icode) {
				// CodeStore.getEventString(s,Roots.rootArray[Parser.syntArray[index].iroot].ppat->icode);
				// cout << "DA:cT3: " << s << endl;
				if (checkAttribCode(Roots.rootArray[Parser.syntArray[index].iroot].ppat)) return;  // complex or discard code
				getAttribActor(index);							// get the actor
				// cout << "Attribution located at " << theAttrib.where << endl;
				return;
			} 
		} // if tailtag
		--index;
	} // while index

} // doAttrib

void CoderClass:: checkTime(void)
// Check for time-shift phrases.  
// Currently stops at the first phrase encountered
{
int index = 0;
patStruct *	pPat; 
    litstring s; // ### debug

	while (Parser.syntArray[index].wtype != Halt) {
		if (Parser.hasHeadTag(index,Time)) {
			pPat = Roots.rootArray[Parser.syntArray[index].iroot].ppat;
//			Processor.fprob  << "\nCC:cT1: " << Phrases.get(s, Roots.rootArray[Parser.syntArray[index].iroot].phrase) << endl; // ### debug

			while (pPat->pnext) {		// go through phrases
				pPat = pPat->pnext;
//				Processor.fprob  << "CC:cT2: " << Phrases.get(s, pPat->phrase) << endl;// ### debug
				if (checkPattern(pPat, index, Time)) {  // pattern matches
					++pPat->used;
					if (shiftDate(pPat)) return;   // continue search if a null code
				}
			}

			// if no match, try the main phrase
			if (Roots.rootArray[Parser.syntArray[index].iroot].ppat->icode) {
				CodeStore.getEventString(s,Roots.rootArray[Parser.syntArray[index].iroot].ppat->icode); // ### debug
//				Processor.fprob  << "CC:cT3: " << s << endl;// ### debug
				if (shiftDate(Roots.rootArray[Parser.syntArray[index].iroot].ppat)) return;
				else while (!Parser.hasTailTag(index,Time)) ++index;  // skip remainder of phrase
			}
		}
		++index;
	} // while psyn

} // checkTime

bool CoderClass:: shiftDate (patStruct * pPat)
// Shift date based on pPat.  Return true except on a null code
{
    bool isIndex;
    int ka = CodeStore.getTimeCode (pPat->icode, isIndex);
    
	if (sTimeShift[0]) return true;  // date has already been shifted, e.g. on a recode
//    Processor.fprob << "CC:sD2: " << ka << "  " << isIndex << endl;
	
	if (isIndex) {  // deal with the codes
		if (CodeStore.indexDiscard == ka) {
			Parser.fHasDiscard = true;
			return true;
		}
		else if (CodeStore.indexComplex == ka) {
			Parser.fIsComplex = true;		
			Parser.fCplxStrg  = true;
			strcpy(Parser.sComplex,"Complex code in time-shift phrase");
			return true;
		}
		else return false;  // null code
	}
    
	Processor.julianDate += ka;
	JulianString(sShiftDate,Processor.julianDate);
 //   Processor.fprob << "CC:sD3: " << sShiftDate<< endl;
    if (ReadFiles.fOutputShift) {  // add option time shift phrase
        char sa[4];
        sprintf(sa,"%d",ka);
        strcpy(sTimeShift,"SHIFTED "); 
        strcat(sTimeShift,sa);
    }
	return true;
} // shiftDate

bool CoderClass:: checkPassive(int index)
// check for passive voice using the following rules:
// 1. Auxilary verb one or two words prior to verb
// 2. BY_ follows verb by one, two or three words;
// 3. Source and target have not been set by patterns (this is checked prior to
//    the call to this function in makeEvent())
// Notes
// 1. Passive voice detection also forces the subject of any later verbs to
//    be re-evaluated -- see if(fisPassive) segment in doVerb
// 2. Rule [2] was changed at the suggestion of VanB -- it originally required
//    the BY_ to follow immediately after the verb -- to allow for the possibility
//    of modifiers, e.g. "ISRAEL WAS CRITICIZED EARLIER BY SYRIA..." or "PAKISTAN WAS PROMISED ECONOMIC AID BY SYRIA..."
//    This problem was causing quite a few passive constructions to be missed. [1-3] word restriction is a bit 
//    arbitrary but a spot check of various records indicates it should work most of the time. <10.04.11>
{
	bool passive = false;
	if (Byword != Parser.syntArray[index+1].wtype && Byword != Parser.syntArray[index+2].wtype && Byword != Parser.syntArray[index+3].wtype) return false;
	if (index>0 && Auxil == Parser.syntArray[index-1].wtype)
		passive = true;
	if (index>1 && Auxil == Parser.syntArray[index-2].wtype)
		passive = true;
	
	if( passive )
	{
		fisPassive = true;
	}
	
	return passive;
} // checkPassive

void CoderClass:: doVerb (int index)
// Process root verb at index.  If match occurs in a pattern containing alternatives,
// continue to try patterns until candidate pattern length is less than the actual
// match 
{
patStruct *	pPat; 
bool	fgotpat = false;		// signals final successful match
bool	fgotalt = false;    // matched pattern containing alternatives
bool  fmake = false;      // okay to make event
int oldnEvents = nEvents; // current value of nEvents
int maxlen = 0;						// actual length of longest matched pattern

#if DEBUG_VERB
instring s;			// *** debug
#endif

	zeroActor(theTarget);
	if(fisPassive) { 				// reset source if previous verb was passive
		zeroActor(theSource);
		fisPassive = false;
	}		
	pPat = Roots.rootArray[Parser.syntArray[index].iroot].ppat;
//	Processor.fprob  << "\ndV1 >> Verb: " << Phrases.get(s, Roots.rootArray[Parser.syntArray[index].iroot].phrase) << endl;

	while ((pPat->pnext) && (!fgotpat)) {		// go through phrases
		pPat = pPat->pnext;
#if DEBUG_VERB
		Processor.fprob << "dV2 >> Phrase: " << Phrases.get(s, pPat->phrase) << endl;
		Processor.fprob  << "dV2 >>         length = " << pPat->length << "  maxlen = " << maxlen << endl;
#endif
		if ((fgotalt) && (pPat->length <= maxlen)) {	// done checking phrases that might be longer
			fgotpat = true;															// than a match containing alternatives
			break;
		} 
		indexSource 	= -1;
		indexTarget 	= -1;
		indexAttrib 	= -1;
		indexCompound = -1;
		if (checkPattern(pPat, index, Verb)) {
//		  cout << "dV3 >> matched " << endl;
		  if (lenalt) {		// matched a pattern containing alternatives
//		  cout << "CC:dV3:lenalt = " << lenalt << endl;
		  	fgotalt = true;
		  	if (pPat->length + lenalt > maxlen) {
		  		fmake = true;
		  		maxlen = pPat->length - pPat->maxalt + lenalt;  // actual matched length
		  	}
		  }
		  else {   // fixed pattern matched, so we're finished
		  	fgotalt = false;
				fgotpat = true;		  	
		  	fmake = true;
		  }
			if (fmake) {
				++pPat->used;
				if (indexSource >= 0) { 	// if source was set via $ or %, move to theSource
					getActorStructCodes(theSource, indexSource); 
					findAgent(indexSource, theSource);
				}
				if (indexTarget >= 0) { // same for +
					getActorStructCodes(theTarget, indexTarget);	
					findAgent(indexTarget, theTarget);
			}
				if (indexAttrib >= 0) getActorStructCodes(theAttrib, indexAttrib);	// same for @
				if (indexCompound >= 0) {																						// same for %
					getActorStructCodes(theSource, indexCompound);  // one or the other is supposed to be empty in a meaningful pattern
					getActorStructCodes(theTarget, indexCompound);	
				}	
				nEvents = oldnEvents;   // reset this in case an alternative pattern is being over-ridden
//				Processor.fprob << "  doVerb:Mk3 " << theSource.agtidx << "  " << theTarget.agtidx << endl; // debug
				makeEvent(pPat, index);
				fmake = false;
//				Processor.fprob << "  doVerb:Mk4 " << theSource.agtidx << "  " << theTarget.agtidx << endl; // debug
			}
		}
#if DEBUG_VERB
		else cout << "dV3 >> failed " << endl;
#endif
	}

									// if no match, try the main verb

	if ((!fgotpat) && (Roots.rootArray[Parser.syntArray[index].iroot].ppat->icode)) {
		indexSource 	= -1;
		indexTarget 	= -1;
		indexAttrib 	= -1;
		indexCompound = -1;
		tempArray[0] = index;	// record verb
		tempArray[1] = -1;  	// add terminators
		tempArray[2] = -2;  	
		iTemp = 3;						// remaining transfers to tempArray will be above this
#if DEBUG_VERB
		CodeStore.getEventString(s,Roots.rootArray[Parser.syntArray[index].iroot].ppat->icode);
		cout << "dV3 >> Event string: " << s << endl;
#endif
		makeEvent(Roots.rootArray[Parser.syntArray[index].iroot].ppat, index);
	}
} // doVerb 

void CoderClass:: checkVerbs(void)
// Main verb processing loop
// This handles the various contingencies of the SET: CODE BY command, albeit a
// bit subtly.  These are:
//	CODE BY CLAUSE 	: Stop coding in a clause once a non-subordinate event is found [default]
//	CODE BY SENTENCE: Stop coding in a sentence once a non-subordinate event is found
//	CODE ALL 			  : Code events from all verbs
// This also resets the source when an actor occurs immediately after the conjunction
// Notes:
//	1.	The  (iendClause - istartClause < 2) contingency deals with situations where the
//			"clause" is actually an unrecognized compound noun, e.g. "sheep and goats."  The
//			constant '2' is used on the logic that any true clause needs at least a verb and
//			an object. 
{
int index;
int startEvent;
int ievt;
bool fgotEvent;

	zeroActor(theSource);
	fisPassive = false;
	istartClause = 0;			// this is actually set in setClauseBounds
	iendClause = -1;			// this signals setClauseBounds that this is the first clause
	
	startEvent = -1;		
	while (setClauseBounds()) {			// go through clauses
		if (iendClause - istartClause < 2) continue;						// See note 1 above
		if ((istartClause) &&  																	// check re-setting the source after the first clause
			  (Parser.hasHeadTag(istartClause,Actor))) getActorStructCodes(theSource, istartClause); // clause begins with actor, so reset source
		zeroActor(theTarget);		// always reset target
		index = istartClause;
		fgotEvent = false;
		if (0 == codeMode) startEvent = nEvents;			// for CODE BY CLAUSE, set eventArray index before clause is evaluated
			while (index <= iendClause) {     // go through and handle the verbs
				if (Parser.skipSegment(index,Subord)) continue;		// skip across subordinate clauses
				if (Parser.hasHeadTag(index,Verb)) {
//	      Processor.fprob << "    cVrb:Mk1 " << theSource.agtidx << "  " << theTarget.agtidx << endl; // debug
					doVerb(index);
//      	Processor.fprob << "    cVrb:Mk2 " << theSource.agtidx << "  " << theTarget.agtidx << endl; // debug
					if (codeMode < 2) {		// check status except for CODE ALL
						for (ievt = startEvent+1; ievt <= nEvents; ++ievt) { // check status of event(s)
							if (eventArray[ievt].fisevt && !eventArray[ievt].fissub) {
								fgotEvent = true;
								if (1 == codeMode) return;		// for CODE BY SENTENCE, stop coding
								else break;   	// get out of the for loop
							}
						}
					}
				} // if (Verb)
				if (fgotEvent) break;   // also get out of the while index loop
				++index;
			}  // while index
	} // setClauseBounds
} // checkVerbs


//___________________________________________________________________________________
// 																																						Issues

void CoderClass:: codeIssues (void)
// get any issues codes
{
issueHeadStruct * pHd = &issueHead; // current issue header
issueListStruct * pLt; 							// current issue list
int index = 0;
int kat;

	while (index <= Parser.iLex) {   // look for an issue
		if (Parser.hasHeadTag(index,Issue)) {  
			kat = (int) Roots.rootArray[Parser.syntArray[index].iroot].iclass;
			pHd = &issueHead;
			while (pHd->kategory != kat) {
				pHd = pHd->pnext;
			}
			if ((!pHd->found) || (pHd->doAll)) {  // see if we want to pick this up
				if (pHd->doNumber) {
					if ((index) && (Number == Parser.syntArray[index-1].wtype)) {  // we've got a syntactic numeric issue
						pHd->total += atoi(Parser.lexArray[index-1].ptext);
					}
					else pHd->total += Roots.rootArray[Parser.syntArray[index].iroot].ppat->length; // get the value of a coded numeric issue
				}
				else {
					pLt = pHd->plist;	
					while ((pLt) && (pLt->icode)) pLt = pLt->pnext;  // find the first empty slot in the code list

					if (!pLt) {		// need to make a new one
						pLt = pHd->plist;  // traverse again but stop before null pointer; this code is executed very rarely...
						if (pLt) {
							while (pLt->pnext) pLt = pLt->pnext;  // find the empty slot
							pLt->pnext = new issueListStruct;			// make a new list item
							pLt = pLt->pnext;
						}
						else {
							pHd->plist = new issueListStruct;			// make a new list item
							pLt = pHd->plist;
						}
						pLt->pnext = NULL;										// mark the termination
					}				
					pLt->icode = Roots.rootArray[Parser.syntArray[index].iroot].ppat->icode; 
				}
				pHd->found = true;
			}
		}
		++index;
	}  // while index

} // getIssues

void CoderClass:: makeIssueString (void)
// assemble to issues codes string
{
issueHeadStruct * pHd = &issueHead; 
issueListStruct * pLt; 
char * sc;             	// pointer to code storage
char *s = sIssueCodes;	// output string

try {	
		while (pHd) {  // go through the active issues
			if (pHd->found) {
				if (pHd->doNumber) {  // transfer integer value
					sprintf(s,"%d",pHd->total);  // write total to sIssueCodes
					while (*s) ++s;							 // move to end of string
					if ((s - sIssueCodes) > MAX_TAB_INPUT-8) throw 1;
				}
				else {   // transfer code
					pLt = pHd->plist;
					while ((pLt) && (pLt->icode)) {  // get the list of codes for the issue
						sc = CodeStore.getCodePtr(pLt->icode);
						while (*sc) *s++ = *sc++;   // transfer the code
						if ((s - sIssueCodes) > MAX_TAB_INPUT-8) throw 1;  // get out if this looks too long
						*s++ = ' ';									// blank delimit it
						pLt = pLt->pnext;
					}
				}
			}
			else if (pHd->icode) { // transfer the default code
				sc = CodeStore.getCodePtr(pHd->icode);
				while (*sc) *s++ = *sc++;   // transfer the code
				if ((s - sIssueCodes) > MAX_TAB_INPUT-8) throw 1;
			}
		pHd = pHd->pnext;
		*s++ = '\t';
	}
	*(--s) = '\0';		// write over final tab to terminate string
}

catch (int i) {  // exceeded the capacity of the output string.  Damage may have already been done, but at least this will limit it
	ShowWarningError("Too many issues codes were found in record",Processor.sRecID,"Record is incorrectly coded.",shError12);	
	sIssueCodes[MAX_TAB_INPUT-8] = '\0';
}

} // makeIssueString

void CoderClass:: zeroIssues (void)
// zero-out the issues lists
{
issueHeadStruct * pHd = &issueHead; // current issue header
issueListStruct * pLt; // current issue header

	while (pHd) {  // go through list of issue headers
		pHd->found = false;
		if (pHd->doNumber) pHd->total = 0;
		else {
			pLt = pHd->plist;  // go through code list
			while (pLt) {
				pLt->icode = 0;
				pLt = pLt->pnext;
			}
		}
		pHd = pHd->pnext;
	}
} // zeroIssues


//___________________________________________________________________________________
// 																																						Freqs

void CoderClass:: codeFreqs (void)
// get freq codes
{
int index = 0;

	for (index = 0; index < iFreq; ++index) Literals.kountFreq[index] = 0;
	
	index = 0;
	while (index <= Parser.iLex) {  // check all words
		if (Parser.lexArray[index].ilit > 0) Literals.countFreqs(&Parser.literaList[Parser.lexArray[index].ilit]);
		++index;
	}  // while index

} // codeFreqs


//___________________________________________________________________________________
// 																																		   Non-events

void CoderClass:: checkNonEvents(void)
// Create a record consisting of the first two distinct actors and a non-event code
// This set eventStrgs[0] directly without setting anything in eventArray[]
// eventStrgs[0].verbroot == 0 is the signal that a non-event has been coded
{

//	WriteLine("CC:cNE Mk1"); // *** debug
	zeroActor(theSource);
	getRegularSource(theSource.where);
	zeroActor(theTarget);
	getRegularTarget(theSource.where);

	if (actorEmpty(theSource) || actorEmpty(theTarget)) return;  // ### <09.01.12> need to set the problem string here 

	if ((Parser.syntArray[theSource.where].flags & Parser.setCompd) ||
			(Parser.syntArray[theTarget.where].flags & Parser.setCompd))  {  
		// syntactic compound: just get the first source and target in the compound
		int idxSource; 
		int idxTarget;
		actorStruct tempSource;
		actorStruct tempTarget;
		bool fcmpdSrc, fcmpdTar;
		
		fcmpdSrc = (bool) (Parser.syntArray[theSource.where].flags & Parser.setCompd) && Parser.checkTailTag(theSource.where,Compound);
		if (fcmpdSrc) idxSource = getCompound(theSource.where);
		else          idxSource = theSource.where;
		if (idxSource<0) idxSource = theSource.where;  // compound tags didn't correspond
		getActorStructCodes(tempSource,idxSource); 
	
		fcmpdTar = (bool) (Parser.syntArray[theTarget.where].flags & Parser.setCompd) && Parser.checkTailTag(theTarget.where,Compound);
		if (fcmpdTar) idxTarget = getCompound(theTarget.where);  // ###probably should save this; also then we only store in temp once
		else          idxTarget = theTarget.where;
		if (idxTarget<0) idxTarget = theTarget.where;  // compound tags didn't correspond

		getActorStructCodes(tempTarget,idxTarget);
	} // if compound 

// Load a non-event record into eventStrgs[0] based on pPat, syntArray[index], source and target actors
// eventArray[] is not used for non-events 

	eventStrgs[0].sourceCode = theSource.actor;
	eventStrgs[0].srcagtCode = theSource.agent;
	eventStrgs[0].targetCode = theTarget.actor;
	eventStrgs[0].taragtCode = theTarget.agent;
	eventStrgs[0].attribCode = 0;
	eventStrgs[0].eventCode  = 0;
	eventStrgs[0].imatch		 = 0;
	eventStrgs[0].ppat		   = 0;
	eventStrgs[0].verbroot	 = 0;
	eventStrgs[0].srcroot	   = theSource.rootidx;
	eventStrgs[0].tarroot		 = theTarget.rootidx;
	eventStrgs[0].sagtroot	 = theSource.agtidx;
	eventStrgs[0].tagtroot	 = theTarget.agtidx;
	eventStrgs[0].fisEvent   = true;
	nEvents = 0;
	fhasProblem = false;
//	WriteLine("CC:cNE Mk2"); // *** debug
} // checkNonEvents

//___________________________________________________________________________________
// 																																		Event functions

tokindex CoderClass:: storeMatch(void)
// rearrange contents of tempArray into correct order, store in TokBuf
// returns starting location
// Note that we store the lexArray index + 1 so that zero can be used as
// a terminator
{
int ka = 0;
// char * pst;
tokindex itok = TokenStore.iToken; // save start of list

#if FALSE
// debugging: show original tempArray
	cout << "##";
	while (tempArray[ka] > -2) cout << tempArray[ka++] << "  ";
	cout << endl;
	
// debugging: rearrange indices
	ka = 1;
	while (tempArray[ka] > -2) ++ka; 
	--ka; 					// output lower match
	while ((ka>=0) && (tempArray[ka] >= 0)) cout << tempArray[ka--] << " ";
	cout << "++ " << tempArray[0] << " ++ " ;  // verb location
	ka = 1; 				// output upper match
	while (tempArray[ka] > 0) cout << tempArray[ka++] << " ";
	cout << endl;
	

// debugging: show strings
	ka = 1;
	while (tempArray[ka] > -2) ++ka; 
	--ka;
	while ((ka>=0) && (tempArray[ka] >= 0)) {   // output lower match: this doesn't incorporate correct actor text ordering
		char *pst = Parser.lexArray[tempArray[ka--]].ptext;
		while (*pst != ' ') cout << *pst++;
		cout << ' ';
	}

	pst = Parser.lexArray[tempArray[0]].ptext;
	while (*pst != ' ') cout << *pst++;
	cout << ' ';

	ka = 1;
	while (tempArray[ka] > 0) {   // output upper match
		char *pst = Parser.lexArray[tempArray[ka++]].ptext;
		while (*pst != ' ') cout << *pst++;
		cout << ' ';
	}
	cout << endl;
#endif

// actual storage
	ka = 1;
	while (tempArray[ka] > -2) ++ka; 
	--ka; 												// store lower match
	while ((ka>=0) && (tempArray[ka] >= 0)) {
		if (!Parser.hasTailTag(tempArray[ka],Actor)) TokenStore.putToken(tempArray[ka--]+1);
		else {  // put lower match actor text in correct order
			int istart = ka;
			int kb = ka;
			while ((kb>0) && (!Parser.hasHeadTag(tempArray[kb],Actor))) --kb;
			ka = kb - 1;
			while (kb <= istart) TokenStore.putToken(tempArray[kb++]+1);			
		}
	}
//	TokenStore.putToken(tempArray[0]+1); // store verb

	ka = tempArray[0] - 1; // transfer indices of verb phrase to tempArray
	do {
		++ka;
		if (Verb == Parser.syntArray[ka].wtype) TokenStore.putToken(ka+1); // store verb
	} while (!Parser.hasTailTag(ka,Verb));

	ka = 1;// output upper match
	while (tempArray[ka] > 0) TokenStore.putToken(tempArray[ka++]+1);   
	TokenStore.putToken(0);   // terminate list
	
	return itok;

} // storeMatch

void CoderClass:: copyEvent (int ka, int kb)
// set eventArray[ka] = eventArray[kb]
// ### could this be done [reliably] with a memcpy() and sizeof()?
// ### also look at setActor
{
	eventArray[ka].source = eventArray[kb].source;
	eventArray[ka].srcagt = eventArray[kb].srcagt;
	eventArray[ka].srcidx = eventArray[kb].srcidx;
	eventArray[ka].target = eventArray[kb].target;
	eventArray[ka].taragt = eventArray[kb].taragt;
	eventArray[ka].taridx = eventArray[kb].taridx;
	eventArray[ka].attrib = eventArray[kb].attrib;
	eventArray[ka].event  = eventArray[kb].event;
	eventArray[ka].ppat   = eventArray[kb].ppat;
	eventArray[ka].verbroot = eventArray[kb].verbroot;
	eventArray[ka].srcroot  = eventArray[kb].srcroot;
	eventArray[ka].sagtroot	= eventArray[kb].sagtroot;
	eventArray[ka].tagtroot	= eventArray[kb].tagtroot;
	eventArray[ka].tarroot  = eventArray[kb].tarroot;
	eventArray[ka].imatch   = eventArray[kb].imatch;
	eventArray[ka].srcloc	= eventArray[kb].srcloc;
	eventArray[ka].tarloc	= eventArray[kb].tarloc;
	eventArray[ka].evtloc   = eventArray[kb].evtloc;
	eventArray[ka].fisevt   = eventArray[kb].fisevt;
	eventArray[ka].fissub   = eventArray[kb].fissub;
	eventArray[ka].fisdom   = eventArray[kb].fisdom;
	eventArray[ka].nextEvt  = eventArray[kb].nextEvt;
} // copyEvent

void CoderClass:: putEvent (patStruct * pPat, int index, actorStruct actSource, actorStruct actTarget )
// Load an event record into eventArray[nEvents] based on pPat, syntArray[index], source and target actors 
// Handles paired, subordinant and dominant codes; also increments nEvents
{
toktype ievent;
toktype ipair;
eventStruct * pevt;
tokindex match = 0;

	CodeStore.getEventCode(ievent,ipair,pPat->icode);
	if (TabariFlags.fhasAttribList) {  // get the global attribution code
		if (theAttrib.index) {
			tokptr pt = NULL;
			CodeStore.nextActorCode (theAttrib.actor, theAttrib.agent, &pt, theAttrib.index); // just get the first actor
		}
	}

	if (ReadFiles.fOutputMatch) match = storeMatch();

	++nEvents;
	if ((nEvents >= MAX_EVENTS) ||
			(ipair && (nEvents >= MAX_EVENTS-1))) { 									// overflow, so bail out
		coderError("No additional storage in CoderClass::putEvent",EVT_OVRFLOW_ERROR);
		--nEvents;
		return;
	}
	if (nEvents > 0) 	eventArray[nEvents-1].nextEvt = nEvents;  // pointer from the last entry
	pevt = &eventArray[nEvents];

	pevt->source = actSource.actor;
	pevt->srcagt = actSource.agent;
	pevt->srcidx = actSource.index;
	pevt->srcloc = actSource.where;
	pevt->target = actTarget.actor;
	pevt->taragt = actTarget.agent;
	pevt->taridx = actTarget.index;
	pevt->tarloc = actTarget.where;
	pevt->evtloc = index;
	pevt->ppat     = pPat;
	pevt->verbroot = Parser.syntArray[index].iroot;  // verb root index
	pevt->srcroot  = actSource.rootidx;
	pevt->tarroot  = actTarget.rootidx;
	pevt->sagtroot = actSource.agtidx;
	pevt->tagtroot = actTarget.agtidx;
	pevt->imatch 	 = match;
	pevt->fisevt   = true;
//	Processor.fprob << "pEvt:Mk1 " << actSource.agtidx << endl;  // debug
	CodeStore.decodeEventCode(pevt->event, pevt->fissub, pevt->fisdom,ievent);
	if (TabariFlags.fhasAttribList) pevt->attrib = theAttrib.actor;  // get the global attribution code
	else pevt->attrib = 0;

	if (ipair) {  // deal with paired verb codes
		++nEvents;
		++pevt;
		eventArray[nEvents-1].nextEvt = nEvents;
		pevt->source = actTarget.actor;
		pevt->srcagt = actTarget.agent;
		pevt->srcidx = actTarget.index;
		pevt->srcloc = actTarget.where;
        pevt->srcroot= actTarget.rootidx;
		pevt->sagtroot=actTarget.agtidx;
		pevt->target = actSource.actor;
		pevt->taragt = actSource.agent;
		pevt->taridx = actSource.index;
		pevt->tarroot= actSource.rootidx;
		pevt->tagtroot=actSource.agtidx;
        pevt->tarloc = actSource.where;
        pevt->evtloc = index;
		pevt->ppat 	 = pPat;
		pevt->verbroot = Parser.syntArray[index].iroot;  // verb root index
		pevt->imatch 	 = match;
		pevt->fisevt = true;
		CodeStore.decodeEventCode(pevt->event, pevt->fissub, pevt->fisdom, ipair);
		if (TabariFlags.fhasAttribList) pevt->attrib = theAttrib.actor;  // get the global attribution code
	  else pevt->attrib = 0;
	}
	pevt->nextEvt = -1;  // mark the last entry
} // putEvent

void CoderClass:: makeEvent (patStruct * pPat, int index)
// Create an event record[s] from pPat, syntArray[index]
{
int idxSource, idxTarget;
int ka;

//	Parser.writeParsing("Called from makeEvent");  // debug
	if (CodeStore.indexNull == TokenStore.getTokenValue(pPat->icode)) return;   // exit if this is a null code 

	if (actorEmpty(theSource)) getRegularSource(index);
	if (actorEmpty(theTarget)) getRegularTarget(index);

	if (actorEmpty(theSource) || actorEmpty(theTarget)) return;  

	if (fRegularSource && fRegularTarget && checkPassive(index)) {  // passive voice; swap source and target
		actorStruct tempAct;
		setActor(tempAct,theSource);
		setActor(theSource,theTarget);
		setActor(theTarget,tempAct);
	}
 //   Processor.fprob << "C::mEv loc " << theSource.where << "  " << theTarget.where << "  " << index << endl; // debug

	if (!(Parser.syntArray[theSource.where].flags & Parser.setCompd) &&
			!(Parser.syntArray[theTarget.where].flags & Parser.setCompd))  {  // simple source and target
		putEvent(pPat, index, theSource, theTarget);
	}
	else {  
		try {// syntactic compound
			actorStruct tempSource;
			actorStruct tempTarget;
			bool fcmpdSrc, fcmpdTar;
			
			fcmpdSrc = (bool) (Parser.syntArray[theSource.where].flags & Parser.setCompd) && Parser.checkTailTag(theSource.where,Compound);
			if (fcmpdSrc) idxSource = getCompound(theSource.where);
			else          idxSource = theSource.where;
			if (idxSource<0) idxSource = theSource.where;  // compound tags didn't correspond

			tempSource.where = 1;							// go through loop at least once
			while (tempSource.where >= 0) {		// loop through sources
				if (idxSource == MAX_SYNT-1) setActor(tempSource, theSource);  // theSource is the default source
                else {
                    getActorStructCodes(tempSource,idxSource); 
                    findAgent(idxSource, tempSource);    
                }
		
				fcmpdTar = (bool) (Parser.syntArray[theTarget.where].flags & Parser.setCompd) && Parser.checkTailTag(theTarget.where,Compound);
				if (fcmpdTar) idxTarget = getCompound(theTarget.where);  // ###probably should save this; also then we only store in temp once
				else          idxTarget = theTarget.where;
				if (idxTarget<0) idxTarget = theTarget.where;  // compound tags didn't correspond

				tempTarget.where = 1;									// go through loop at least once
				while (tempTarget.where >= 0) {				// loop through targets
                    if (idxTarget == MAX_SYNT-1) setActor(tempTarget, theTarget);  // theTarget is the default target
                    else {
                        getActorStructCodes(tempTarget,idxTarget);
                        findAgent(idxTarget, tempTarget);
					}
					putEvent(pPat, index, tempSource, tempTarget);
				 
					if (fcmpdTar) {
						++idxTarget;			// find next root
						while (!Parser.syntArray[idxTarget].iroot ||
									 (Parser.syntArray[idxTarget].wtype != Actor)) {
									 ++idxTarget;  
									 if (idxTarget >= Parser.iSynt) throw COMPOUND_TAG_ERROR; // can overflow if compound was disrupted by null marking...
						}
						ka = idxTarget;
						while (!Parser.hasTailTag(ka, Actor)) {
							++ka;  // look for the end of a multi-word root
							if (ka >= Parser.iSynt) throw COMPOUND_TAG_ERROR; // can overflow if compound was disrupted by null marking...
						}
						if (Parser.hasTailTag(ka, Compound)) fcmpdTar = false;  // last one 
					}
					else tempTarget.where = -1;  // signal the end of the while loop
				}
				
				if (fcmpdSrc) {
					++idxSource;			// find next root
					while (!Parser.syntArray[idxSource].iroot ||
								(Parser.syntArray[idxSource].wtype != Actor)) {
									++idxSource; 
									if (idxSource >= Parser.iSynt) throw COMPOUND_TAG_ERROR; // can overflow if compound was disrupted by null marking...
								}
					ka = idxSource;
					while (!Parser.hasTailTag(ka, Actor)) { // look for the end of a multi-word root
						++ka;
						if (idxSource >= Parser.iSynt) throw COMPOUND_TAG_ERROR;
					}
				if (Parser.hasTailTag(ka, Compound)) fcmpdSrc = false;  // last one
				}
				else tempSource.where = -1;  // signal the end of the while loop
			}
		}
		catch (int err) {
			if (COMPOUND_TAG_ERROR == err) return; // ### <08.06.20> currently we don't do anything with this...there is no obvious way to recover
			else return; // ### shouldn't hit this...
		}	
	} // else syntactic

	if (!fRegularSource) zeroActor(theSource);	// erase actor if it was set by a pattern
	if (!fRegularTarget) zeroActor(theTarget);	

} // makeEvent

void CoderClass:: recoverCodes(void)
// This is used only if there is an overflow: it replaces all coded compounds with 
// the first code.  Also just sets the nextEvt pointers to the array order.
// Cleans out duplicates, so this could actually result in fewer than MAX_EVENTS
// events.
{
tokptr pt = NULL;
int ilast;   // previous entry in eventArray
int ka;

	ka = 0;
	while (ka >= 0)  {   // expand the compound sources
		if (eventArray[ka].srcidx) {
			pt = NULL;
			CodeStore.nextActorCode (eventArray[ka].source, eventArray[ka].srcagt, &pt, eventArray[ka].srcidx);
			eventArray[ka].srcidx = 0;  // indicate we've processed this
		}
		if (eventArray[ka].taridx) {
			pt = NULL;
			CodeStore.nextActorCode (eventArray[ka].target, eventArray[ka].taragt, &pt, eventArray[ka].taridx);
			eventArray[ka].taridx = 0;  // indicate we've processed this
		}
		ka = eventArray[ka].nextEvt;
	}
	nEvents = MAX_EVENTS - 1;
	eventArray[nEvents].nextEvt = -1;  // mark the last entry
	
																		// eliminate the duplicates
	ka = 1;
	ilast = 0;
	while (ka <= nEvents) {
		if (checkEventDups(ka)) eventArray[ilast].nextEvt = eventArray[ka].nextEvt;  // skip the list
		else ilast = ka;
		++ka;
	}
	
} // recoverCodes

bool CoderClass:: checkEventDups(int ievt)
// Check for duplicates.  Returns true if a duplicate is found
{
int ka = 0;

	while (ka < ievt) {
		if ((eventArray[ievt].source == eventArray[ka].source) && 
				(eventArray[ievt].srcagt == eventArray[ka].srcagt) && 
				(eventArray[ievt].target == eventArray[ka].target) && 
				(eventArray[ievt].taragt == eventArray[ka].taragt) && 
				(eventArray[ievt].event  == eventArray[ka].event))
		break;
		++ka;
	}
	if (ka < ievt) return true;
	else return false;

} // checkEventDups

bool CoderClass:: checkSameActors(int ievt)
// Check if source and target codes are identical; returns true if they are
// Always returns false if fSameActors = true
{
//	Processor.fprob << "cSA:Mk0 " << idxcodeDefSrc << " " << idxcodeDefTar << " " << CodeStore.indexBlank << endl; // debug
	if (eventArray[ievt].source <= CodeStore.indexBlank) return true;
	if (eventArray[ievt].target <= CodeStore.indexBlank) return true;
	
	if (TabariFlags.fSameActors) return false;
	
  if ((eventArray[ievt].source == eventArray[ievt].target) && 
			(eventArray[ievt].srcagt == eventArray[ievt].taragt))
			return true;
	else return false;
} // checkSameActors

void CoderClass:: expandCompoundCodes(void)
// Expand the coded compounds using the compound lists in CodeStore
// Note: This eliminates most of the duplicates and same-actor cases, but it leaves
// those that result from the initial expansion of a reflexive compound; I didn't
// see a clean way of doing that so they get cleared out later.
{
tokptr pt = NULL;
int ilast = -1;   // previous entry in eventArray
int inext;				// current next pointer
int ka, ievt = nEvents;
//int kb; // *** debug

	for (ka = 0; ka <= nEvents; ++ka) {   // expand the compound sources
		if (ilast >= 0) eventArray[ilast].nextEvt = ka;  // modify the previous pointer
		ilast = ka;
		inext = eventArray[ka].nextEvt;
		if (eventArray[ka].srcidx) {
			pt = NULL;
			CodeStore.nextActorCode (eventArray[ka].source, eventArray[ka].srcagt, &pt, eventArray[ka].srcidx);
			eventArray[ka].srcidx = 0;  // indicate we've processed this
			while (pt) {
				++ievt;
				if (ievt >= MAX_EVENTS) {		// abandon expansion, keeping what we've got
					coderError("\aOverflow in expandCompoundActors",CMP_OVRFLOW_ERROR);
					recoverCodes();
					return;
				}
				eventArray[ilast].nextEvt = ievt;  // get previous event pointing to this one as next event
				copyEvent(ievt,ka); // eventArray[ievt] = eventArray[ka];
				CodeStore.nextActorCode (eventArray[ievt].source, eventArray[ievt].srcagt, &pt, eventArray[ka].srcidx); // srcidx is not actually used now
				eventArray[ievt].srcidx = 0;  // simple code
				eventArray[ievt].nextEvt = inext;
				ilast = ievt;
			}
		}
	} // for
	
	nEvents = ievt;
	ilast = -1;
	ka = 0; 
	while (ka >= 0)  {   // expand the compound targets following the list ordering
#if FALSE
		cout << "\nTarget ka = " << ka << endl;  // *** debugging code
		for (kb = 0; kb<=ievt; ++kb) {
			cout << kb << "  " << CodeStore.getCodePtr(eventArray[kb].source);
			cout << "  " <<  CodeStore.getCodePtr(eventArray[kb].target);
			cout <<  "  " << eventArray[kb].nextEvt << endl;
		}
		cout << "ilast = " << ilast << endl;
#endif
		if (ilast >= 0) eventArray[ilast].nextEvt = ka;
		ilast = ka;
		inext = eventArray[ka].nextEvt;
		if (eventArray[ka].taridx) {
			pt = NULL;
			CodeStore.nextActorCode (eventArray[ka].target, eventArray[ka].taragt, &pt, eventArray[ka].taridx);
			eventArray[ka].taridx = 0;  // indicate we've processed this
			// ### is there a clean way to do a checkSameActors() here? -- this initial conversion isn't covered...
			while (pt) {
				++ievt;
				if (ievt >= MAX_EVENTS) {		// abandon expansion, keeping what we've got
					coderError("\aOverflow in expandCompoundActors",CMP_OVRFLOW_ERROR);
					recoverCodes();
					return;
				}
				copyEvent(ievt,ka); // eventArray[ievt] = eventArray[ka];
				CodeStore.nextActorCode (eventArray[ievt].target, eventArray[ievt].taragt, &pt, eventArray[ka].taridx); // taridx is not actually used now
				eventArray[ievt].taridx = 0;  // simple code
				if (checkSameActors(ievt) || checkEventDups(ievt))	--ievt;
				else {
					eventArray[ilast].nextEvt = ievt;  // get previous event pointing to this one as next event
					eventArray[ievt].nextEvt = inext;
					ilast = ievt;
				}
			}
		}
		ka = inext;
	} // while
	
	nEvents = ievt;
} // expandCompoundCodes

void CoderClass:: evaluateEvents(void)
// Evaluates the discard, complex, null, subordinant and dominant events, then 
// compacts eventArray
{
bool fnonSub=false;
bool fhasDom=false;
bool fhasNull=false;
int ka;

	for (ka=0; ka<=nEvents; ++ka) {

		if (CodeStore.indexNull == eventArray[ka].event) {  // eliminate the null events
			eventArray[ka].fisevt = false;
			eventArray[ka].fissub = true;  // don't make this a non-subordinant event
			fhasNull = true;
		}

		if (CodeStore.indexDiscard == eventArray[ka].event)  { // break on discard events
			fhasDiscard = true;
			break;
		}

		if ((CodeStore.indexComplex == eventArray[ka].event)  ||  // break on complex events
				(CodeStore.indexComplex == eventArray[ka].source) || 
				(CodeStore.indexComplex == eventArray[ka].target)) {
			fhasComplex = true;
			break;
		}

		if (eventArray[ka].fisdom) {
			fhasDom = true;
			fnonSub = true;
			break;
		}
		if (!fnonSub && !eventArray[ka].fissub) fnonSub = true;
	}
	
	if (fhasDom) {  // cancel everything except dominant events
		for (ka=0; ka<=nEvents; ++ka) {
			if (!eventArray[ka].fisdom) eventArray[ka].fisevt = false;
		}	
	}
	else if (fnonSub) { // cancel all the subordinant events
		for (ka=0; ka<=nEvents; ++ka) {
			if (eventArray[ka].fissub) eventArray[ka].fisevt = false;
		}
	}

// eliminate all of the non-events.  

	if (fhasDom || fnonSub || fhasNull) {   //  sometimes this will run without changing the array,
		int incr = 0; 												//  but it is a cheap routine in that situation
		ka = 0;
		while (ka <= nEvents) {
			if (eventArray[ka].fisevt) { // eventArray[incr++] = eventArray[ka++];
				copyEvent(incr,ka);
				++incr;
			}
			++ka;		
		}
		nEvents = incr - 1;
		if (nEvents >= 0) eventArray[nEvents].nextEvt = -1;  // mark last entry
	} 

} // evaluateEvents

void CoderClass:: makeEventStrings (void)
//	Constructs the event record strings using eventArray; produces them in the list order
{
int	kevent = -1;
int inext = 0;

	while (inext >= 0) {
		if (!checkSameActors(inext) && !checkEventDups(inext)) {
			++kevent;
			eventStrgs[kevent].sourceCode   = eventArray[inext].source;
			eventStrgs[kevent].srcagtCode   = eventArray[inext].srcagt;
			eventStrgs[kevent].targetCode   = eventArray[inext].target;
			eventStrgs[kevent].taragtCode   = eventArray[inext].taragt;
			eventStrgs[kevent].attribCode   = eventArray[inext].attrib;
			eventStrgs[kevent].eventCode    = eventArray[inext].event;
			eventStrgs[kevent].imatch       = eventArray[inext].imatch;
			eventStrgs[kevent].ppat		    = eventArray[inext].ppat;
			eventStrgs[kevent].verbroot		= eventArray[inext].verbroot;
			eventStrgs[kevent].srcroot      = eventArray[inext].srcroot;
			eventStrgs[kevent].tarroot		= eventArray[inext].tarroot;
			eventStrgs[kevent].sagtroot     = eventArray[inext].sagtroot;
			eventStrgs[kevent].tagtroot     = eventArray[inext].tagtroot;
			eventStrgs[kevent].srcloc		= eventArray[inext].srcloc;
			eventStrgs[kevent].tarloc       = eventArray[inext].tarloc;
			eventStrgs[kevent].evtloc       = eventArray[inext].evtloc;
			eventStrgs[kevent].fisEvent     = true;
		}
		inext = eventArray[inext].nextEvt;
	} // while 
	nEvents = kevent;

} //  makeEventStrings

void CoderClass:: setProblemString(char *s)
// sets the problem description, restore value of iToken
{
	strcpy(sProblem, s);
	fhasProblem = true;
	TokenStore.iToken = itempToken;  // restore value
} // setProblemString


void CoderClass:: setProblemString(const char *s)
// sets the problem description, restore value of iToken
{
	strcpy(sProblem, s);
	fhasProblem = true;
	TokenStore.iToken = itempToken;  // restore value
} // setProblemString


void CoderClass:: codeEvents(void)
// primary coding function -- just calls the working functions in the correct order
// and sets ProblemString if events aren't found
// ### At some point, add a table to the documentation explaining the various 
//		ProblemString conditions
{	
instring s;

	nEvents = -1;
	fhasProblem = false;
	sProblem[0] = '\0';
	sTimeShift[0] = '\0';
    sShiftDate[0] = '\0'; 
	fhasDiscard = false;
	fhasComplex = false;
	fRegularSource = false;
	fRegularTarget = false;
	itempToken = TokenStore.iToken;  // save current value; storeMatch() uses this

	if (Parser.iparseError) {  // parser found errors; don't process further
		Parser.getParseError(s);
		setProblemString(s);
		return;
	}
//		Parser.writeParsing("Called from codeEvents 1 ");  // *** debug

	// cout << "cEe\n";
	if (TabariFlags.fhasIssues) zeroIssues();
	
//    Processor.fprob << "cEe " << TabariFlags.fhasTimeList << endl; *** debug
	if (TabariFlags.fhasTimeList) checkTime();
	
	// cout << "cE1\n";
	if (TabariFlags.fhasAttribList) {
		// cout << "cE1a\n";
		doAttrib();
		// cout << "cE1b\n";
		Parser.nullSubordinate();  // okay to null the subordinate clauses now
	}
	// cout << "cE1x\n";

//	 Parser.writeParsing("Called from codeEvents 2");  // *** debug

	if (Parser.fHasDiscard) {
		if (*Processor.sentText) setProblemString("Discard code found in text");
		else setProblemString("Record contains no text");
		return;
	}

	if (Parser.fIsComplex) {			// failed the complexity test		
		if (Parser.fCplxStrg) {			// show the reason if it was generated
			strcpy(s, "Complex text: ");
			strcat(s,Parser.sComplex);
			setProblemString(s);
		}
		else setProblemString("Text failed complexity conditions");
		// ### record should be written to complex file here
		return;
	}

//	Processor.fprob << "\n=== cEvt: " << Processor.sRecID << "  " << Processor.sDocID  << "===" << endl; // debug: stash of these
//	Processor.fprob << "cEvt:Mk1s " << eventStrgs[0].sagtroot << "  " << eventArray[0].sagtroot << endl; // debug
//	Processor.fprob << "cEvt:Mk3 " << nEvents  << endl; // debug


	checkVerbs();

	if (nEvents < 0) {
		setProblemString("No events found in text");
		if (TabariFlags.fNonEvents) checkNonEvents();
		return;
	}

	evaluateEvents();
	
	if (nEvents < 0) {
		setProblemString("Only null events found in text");
		if (TabariFlags.fNonEvents) checkNonEvents();
		return;
	}

	if (fhasDiscard) {
		setProblemString("Discard code found in verb pattern");
		return;
	}

	if (fhasComplex) {
		setProblemString("Complex code found in verb pattern");
		return;
	}

	expandCompoundCodes();
	
	makeEventStrings();

	if (nEvents < 0) {
		setProblemString("No valid events found in text");
		return;
	}

	if (TabariFlags.fhasIssues) {
		codeIssues();
		makeIssueString();
	}

	if (TabariFlags.fhasFreqs) codeFreqs();

	TokenStore.iToken = itempToken;  // restore value
} // codeEvents

void CoderClass:: addCompositeCode(char *s, toktype itokactor, toktype itokagent)
// add the combined actor and agent codes to end of s 
{
char * sact = CodeStore.getCodePtr(itokactor); // get pointers to the codes
char * sagt = CodeStore.getAgentCodePtr(itokagent);
char * ps;
litstring stemp;
	
	
//	Processor.fprob << "  C::aCC mk1 " << sact << "  " << sagt << endl;
	if ('~' == *sagt) {  // prefix actor
		strcat(s,sact);
		ps = strstr(sact,sagt+1);   // check if we already include the code
		if ((ps) && 
			 (0 == (sact - ps) % 3))     // check that codes are aligned at 3-chars
				return;										// don't include redundant agent codes
		strcat(s, sagt+1);  // skip the ~
	}
	else {   // postfix actor
		strcpy(stemp,sagt);
		stemp[strlen(stemp)-1] = '\0'; // remove the ~ from the copy
		ps = strstr(sact,stemp); // check for redundant agent codess
		if (!ps || (0 != (sact - ps) % 3)) strcat(s,stemp); 		
		strcat(s,sact);
	}

} // createCompositeCode

char * CoderClass:: getEventString(char *s, int ievt)
// Converts eventStrg[ievent] is a formatted output string in s; returns pointer to same
// Notes:
// 1. Per comments in the manual, this is the routine that should be modified with you want
//    to change the ordering of the output fields
// 2. The following output is handled in  ProcessorClass:: writeEvents(void):
//    record IDs, patterns, and issues codes
//
{
	if ((ievt < 0) || (ievt > nEvents)) {
		coderError("Invalid index in getEventString",EVT_STRG_ERROR);
		s[0] = '\0';
		return s;
	}

	if (sShiftDate[0]) strcpy(s, sShiftDate);
    else strcpy(s, Processor.sRecDate);

	strcat(s,"\t");
//	Processor.fprob << "             C::gES mk0 " << CodeStore.getAgentCodePtr(1) << endl;
//	Processor.fprob << "C::gES mk1 " << s << endl;
//	Processor.fprob << "C::gES mk2 " << eventStrgs[ievt].sourceCode << "  " << eventStrgs[ievt].srcagtCode << endl;
//	Processor.fprob << "C::gES mk2.1 " << eventStrgs[0].tagtroot << "  " << eventArray[0].tagtroot << endl; // debug
	if (eventStrgs[ievt].srcagtCode) {
		addCompositeCode(s, eventStrgs[ievt].sourceCode,eventStrgs[ievt].srcagtCode);
//		Processor.fprob << "C::gES mk2t " << s << endl;
	}
	else	strcat(s,CodeStore.getCodePtr(eventStrgs[ievt].sourceCode));
//	Processor.fprob << "C::gES mk3 " << s << endl;
	strcat(s,"\t");
	if (eventStrgs[ievt].targetCode) {
		if (eventStrgs[ievt].taragtCode) addCompositeCode(s, eventStrgs[ievt].targetCode,eventStrgs[ievt].taragtCode);
	  else	strcat(s,CodeStore.getCodePtr(eventStrgs[ievt].targetCode));
	}
	else strcat(s,CodeStore.getCodePtr(CodeStore.indexBlank));
//	Processor.fprob << "C::gES mk4 " << s << endl;
	if (TabariFlags.fhasAttribList) {
		strcat(s,"\t");
		if (eventStrgs[ievt].attribCode) strcat(s,CodeStore.getCodePtr(eventStrgs[ievt].attribCode));
		else strcat(s,CodeStore.getCodePtr(CodeStore.indexBlank));
	}
	strcat(s,"\t");
	strcat(s,CodeStore.getCodePtr(eventStrgs[ievt].eventCode));
	if (ReadFiles.fOutputLabels) {  // add label
		if (ReadFiles.fOutputMatch) strcat(s,"\t("); // put label in () if adding phrase
		else											 strcat(s,"\t");
		strcat(s,CodeStore.getCodeLabel(eventStrgs[ievt].eventCode));
		if (ReadFiles.fOutputMatch) strcat(s,")");
	}	

// add optional ACTOR and AGENT phrases
	if (ReadFiles.fOutputActRoots || ReadFiles.fOutputAgtRoots ) {  // add root phrases 
		instring 	sphr;
		int left = sizeof(instring); // used to detect string overflow:  ### NOT IMPLEMENTED

		left -= (strlen(s) + 5);	// leave room for '\t's,'\0'
		if (ReadFiles.fOutputActRoots) {  // add source actor root phrases 
			strcat(s,"\t");
			if (ReadFiles.fOutputParent) {
				if (Roots.rootArray[eventStrgs[ievt].srcroot].iclass) Phrases.get(sphr, Roots.rootArray[Roots.rootArray[eventStrgs[ievt].srcroot].iclass].phrase);  // get the parent source phrase
				else Phrases.get(sphr, Roots.rootArray[eventStrgs[ievt].srcroot].phrase); // no parent, so get root
			}
			else Phrases.get(sphr, Roots.rootArray[eventStrgs[ievt].srcroot].phrase);  // get root phrase
			// check the length here
			strcat(s,sphr);
		}
		if (ReadFiles.fOutputAgtRoots ) {  // add source agent root phrases
			if (eventStrgs[ievt].sagtroot) {
				Phrases.get(sphr,Roots.rootArray[eventStrgs[ievt].sagtroot].phrase);  // get the agent phrase
			// check the length here
	  		strcat(s,sphr);
			}
			else strcat(s,"\t---");
		}
		if (ReadFiles.fOutputActRoots) {  // add target actor root phrases 
			strcat(s,"\t");
			if (ReadFiles.fOutputParent) {
				if (Roots.rootArray[eventStrgs[ievt].tarroot].iclass) Phrases.get(sphr, Roots.rootArray[Roots.rootArray[eventStrgs[ievt].tarroot].iclass].phrase);  // get the parent source phrase
				else Phrases.get(sphr, Roots.rootArray[eventStrgs[ievt].tarroot].phrase); // no parent, so get root
			}
			else Phrases.get(sphr, Roots.rootArray[eventStrgs[ievt].tarroot].phrase);  // get the source phrase
			// check the length here
			strcat(s,sphr);
		}
		if (ReadFiles.fOutputAgtRoots ) {  // add target agent root phrases
			if (eventStrgs[ievt].tagtroot) {
				Phrases.get(sphr,Roots.rootArray[eventStrgs[ievt].tagtroot].phrase);  // get the agent phrase
                // check the length here
                strcat(s,sphr);
			}
			else strcat(s,"\t---");
		}
	}
	
    if (ReadFiles.fOutputShift) {  // add option time shift phrase
        strcat(s,"\t");
        strcat(s,sTimeShift);
    }
    
// add optional MATCH phrase
	if (ReadFiles.fOutputMatch) { 
		tokptr ptok =  TokenStore.getTokPtr(eventStrgs[ievt].imatch); 
		char *ps = &s[strlen(s)];
		char *pst;
		int left = sizeof(instring); // used to detect string overflow

		left -= (strlen(s) + 2);	// leave room for '\t','\0'
		*ps++ = '\t';
		while (*ptok) { // go through list of words that were matched 
			pst = Parser.lexArray[*ptok-1].ptext; // indices were offset by 1 in storeMatch()
            if (--left <= 0) break; 		// adjust for ' '
			while (*pst != ' ') { 			// transfer text
				*ps++ = *pst++;
				if (--left <= 0) break; 	// no more room
			}
			*ps++ = ' ';
			++ptok;
		}
		*ps = '\0';   // terminate s
	}
    if (ReadFiles.fOutputLocs ) {  // add locations
        char sout[5];
        sout[0]='\t';
        sprintf(&sout[1],"%u",eventStrgs[ievt].srcloc);
        strcat(s,sout);
        sprintf(&sout[1],"%u",eventStrgs[ievt].tarloc);
        strcat(s,sout);
        sprintf(&sout[1],"%u",eventStrgs[ievt].evtloc);
        strcat(s,sout);
    }
	
	return s;
} // getEventString
	
char * CoderClass:: getNonEventString(char *s)
// generates the formatted non-event string based on eventStrg[0]; returns pointer to same
{
	strcpy(s, Processor.sRecDate);
	strcat(s,"\t");
	if (eventStrgs[0].srcagtCode) addCompositeCode(s, eventStrgs[0].sourceCode,eventStrgs[0].srcagtCode);
	else	strcat(s,CodeStore.getCodePtr(eventStrgs[0].sourceCode));
	strcat(s,"\t");
	if (eventStrgs[0].taragtCode) addCompositeCode(s, eventStrgs[0].targetCode,eventStrgs[0].taragtCode);
	else	strcat(s,CodeStore.getCodePtr(eventStrgs[0].targetCode));
	strcat(s,"\t");
	if (TabariFlags.fhasAttribList) {
		strcat(s,"\t");
		strcat(s,CodeStore.getCodePtr(CodeStore.indexBlank));
	}
	strcat(s,"\t");
	strcat(s,sBlankCode);
	if (ReadFiles.fOutputMatch) strcat(s,"\tNo event found");

	return s;
} // getNonEventString

char * CoderClass:: getPatterns(char *s, int ievt)	
// puts pattern strings into s
{
	instring 	sphr;
	instring  sverb;
	RootsClass::rootStruct root;
	
	strcpy(s,"");
	root =  Roots.rootArray[eventStrgs[ievt].srcroot]; // write source phrase
	Phrases.get(sphr,root.phrase);  // get the phrase
	strcat(s,"\t");
	strcat(s,sphr);
	root =  Roots.rootArray[eventStrgs[ievt].tarroot]; // write target phrase
	Phrases.get(sphr,root.phrase); 
	strcat(s,"\t");
	strcat(s,sphr);
	root =  Roots.rootArray[eventStrgs[ievt].verbroot]; // write verb phrase
	Phrases.get(sverb,root.phrase); 
	if (Roots.rootArray[eventStrgs[ievt].verbroot].ppat != eventStrgs[ievt].ppat ) { // if false, match was to a verb root, not a pattern
		char *ps  = &s[strlen(s)];
		char *pphr = sphr;
		char *pverb = sverb;

		Phrases.get(sphr,eventStrgs[ievt].ppat->phrase); // get pattern phrase
		*ps++ = '\t';
		while (*pphr != '*' && *pphr) *ps++ = *pphr++;			// transfer pattern text prior to verb; check for *pphr shouldn't be necessary 
		*ps++ = '<';
		while (*pverb) *ps++ = *pverb++;		// transfer verb text
		*ps++ = '>';
		pphr++;															// skip across * in pattern
		while (*pphr) *ps++ = *pphr++;			// transfer remainder of phrase 
		*ps = '\0';													// terminate s	
	}
	else {
		strcat(s,"\t");
		strcat(s,sverb);			
	}	
 return s;
}	 // writePatterns
