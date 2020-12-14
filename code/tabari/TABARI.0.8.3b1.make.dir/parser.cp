// 																																		TABARI Project
//___________________________________________________________________________________
// 																																	 				parser.cp

//  This file contains the parsing routines

//	Text Filtering:
//	There is a lot -- see the comments in filterText.  There is possibly too much --
//	for example "(...)" is converted to " , ... , " 

// Lexical Markup:
// 1.	The primary function of makeLex() is to assign each word the list of literals
//		that match it.  This is a list because a number of substrings could match:
//		for example "ARABIANS" (as in "Saudi Arabians") could match the literals
//		ARAB  ARABIA  ARABIAN and ARABIANS.   This list is stored in literaList[], 
//		and lexArray[].ilit points to the start of the list. The end of the list is 
//		signaled by a zero cell.
//
// 2. Conditions on lexArray[].ilit:
//		0		: No literal was assigned
//		< 0	: Contains the negative of the index of a *symbol*
//		> 0	: Points to the first element of a list in literaList[]
//
// 3. literaList stores pairs of information on literals:
//			i     literal index 
//      i+1		0 if this was a partial match; 1 if it was an exact match.
//            This is used to correctly deal with _ connectors (which didn't work
//            consistently prior to version 0.5.1)
//
// Parser Markup:
// 1.	Roots are assigned using matchLex(), which uses the literals identified in the
//		lexical markup (which involve single words or fragments of words) to identify
//		multiple-word roots.  Because a word sequence could match multiple roots --
//		"SAUDI ARABIANS" might match "SAUDI", "SAUDI ARABI", and "SAUDI ARABIAN" --
//		the assignment is made to the root that has the *largest number of matched
//		characters*.
//		
//	When root is identified:
//	 a.	All of the literals that are in the root have their syntArray[].wtype 
// 		 	changed to the wordtype of the root.  This will, for example, cancel the 
//     	conjunctions in roots such as "SHOT_AND_KILLED"
//	 b. headTag of the first literal and tailTag of the last literal are tagged  
//		 	with the wordtype.
//	 c. syntArray[].iroot of the head of the root contains the root index

// Notes:

// 1.	At present the system -- using the Literals.check() function -- goes through
//		and checks each possible literal.  A lot of this could be bypassed through
//		some additional processing of the dictionaries -- in the example given above,
//    if we know that ARABIANS matches, then we already know that ARAB, ARABIA and
//		ARABIAN matches.  This requires only a small amount of additional storage
//		and the code should be straightforward.

// 2. The maximum-matched-characters criterion for root matching differs a bit 
//		from that used in KEDS, where roots and phrases were evaluated in the order
//		of the length (in characters) of the phrase and matching stopped at the 
//		first successful match.  In the absence of conditional phrases, the KEDS
//		system and the TABARI system will produce identical results.  If conditional
//		phrases are present, the two systems could produce different results, and
//		dictionaries should be updated accordingly.  The TABARI system is the more
//		accurate of the two methods -- in the sense of correctly identifying the
//		most specific phrase -- but KEDS dictionaries may occasionally be coding
//		the right phrase for the wrong reasons. 
//		(ties are still resolved by the listing order in the dictionary -- the first
//		phrase checked is selected -- and this could, on rare occasions, be used to
//		resolve ambiguous situations)

// 3. Current handling of input error conditions:
//
//		NO_TEXT_ERROR:  No text in record.  This is necessarily caused by a filter 
//                    error.  Don't code it; alert with a problem string.
//
//		STORY_TOO_SHORT:Fewer than MIN_LEX words in sentence.  This is usually caused
//                    by a filter error having incorrectly split a sentence, or else
//										the start of a feature story.  Don't code it; alert with a
//                    problem string.
//
//		STORY_TOO_LONG: More than MAX_LEX-1 words in sentence.  This is usually caused
//                    by a filter error concatenating sentences, so truncate and
//                    code it anyway.

//  4. tagArray holds additional syntactic markup information, organized in 3-word
//     blocks.  syntArray[].pheadtag holds the index to start of a linked list of "head
//     tags" indicating the start of multi-word structures, and syntArray[].ptailtag
//     "tail tags" holds the index to start of a linked list indicating the end.
//     Structure:
//       i    type of tag (e.g. noun, clause)
//       i+1  optional index associated specific heads with tails; zero if not used
//       i+2  tagArray index of next tag in the linked list associated with this word
                     	
//
//	--- Word types ---
//	Null   -- default
//  Actor
//  Agent
//  Verb
//  Time   -- time-shifting word
//  Attrib -- attribution word
//  Determ -- determiner: A_, AN_, THE_
//  Noun	 -- verb shifted to noun by determiner or capitalization; null-coded actors;
//            explicit noun		
//  Adjctv -- adjective; used to construct compound noun and adjective phrases
//  Auxil  -- auxilary verb -- WAS_, WERE_, BEEN_ -- used in passive voice detection
//  Byword -- BY_, used in passive voice detection 
//  Comma  -- comma ",", used in subordinate clause detection and compounds
//  Pronoun
//  Conj   -- AND_, BUT_
//  Prep   -- preposition; currently not used
//  Plural -- plural; currently not used
//  Number -- literals that begin with a digit except when it was part of a phrase,
//            as in "Group of 77"
//  Issue  -- word in ISSUES set
//  Synym  -- currently not used
// 
//	--- Clause tags ---
//  Clause		-- clause in compound sentence
//  Compound	-- compound noun phrase
//  Reference -- pronoun reference
//  Subord    -- subordinate clause
//  Replace   -- 
//  NullTag   -- deactivates tags in subordinate clauses 
//  Halt	    -- tag for end of text

   
//__________________________________________________________________________________
//
//	 Copyright (c) 2000 - 2012 Philip A. Schrodt.  All rights reserved.
//
// 	Redistribution and use in source and binary forms, with or without modification,
// 	are permitted under the terms of the GNU General Public License:
// 	http://www.opensource.org/licenses/gpl-license.html
//
//	Report bugs to: schrodt@psu.edu

//	The most recent version of this code is available from the KEDS Web site:
//		http://eventdata.psu.edu

//___________________________________________________________________________________
// 																																						Headers

#include "TABARI.h"

	ParserClass Parser;

//___________________________________________________________________________________
// 																																   Global Variables

extern TabariFlagsClass TabariFlags;
extern LiteralsClass 		Literals;
extern RootsClass 			Roots;
extern PhrasesClass 		Phrases;
extern ProcessorClass 	Processor;

//___________________________________________________________________________________


void ParserClass:: parseError(const char *s, int ierror)
// write the error string to <errorfile> and throw ierror 
{
	Processor.writeError("Parser error: ", (char*)s, ierror);
	iparseError = ierror;
//	throw ierror;  ### <09.01.15> well, nice, but doesn't seem to be caught anywhere at the moment
}  // parseError

void ParserClass:: getParseError(char *s)
// set texts for the problems string based on iparseError
{
	switch (iparseError) {
		case PARSE_OK: 	
				strcpy(s,"Incorrect call to Parse.getParseError");
		 		break;
		case GENERIC_ERROR: 	
				strcpy(s,"Unspecified problem encountered during parsing ");
		 		break;
		case NO_TEXT_ERROR: 	
				strcpy(s,"Input error: no text in record");
		 		break;
		case TOO_LONG_ERROR: 	// currently not used: long sentences are truncated
				strcpy(s,"Input error: too many words in sentence");
		 		break;
		case TOO_SHORT_ERROR: 	
				strcpy(s,"Input error: too few words in sentence");
		 		break;
		case HEAD_TAG_ERROR: 	
				strcpy(s,"Parser error: Tag overflow in addHeadTag");
		 		break;
		case TAIL_TAG_ERROR: 	
				strcpy(s,"Parser error: Tag overflow in addTailTag");
		 		break;
		case MARK_NOUN_ERROR: 	
				strcpy(s,"Parser error: problem with noun marking");
		 		break;
		case CONVERT_AGENT_ERROR: 	
                strcpy(s,"Parser error: no tail tag on agent");
                break;
		case COMPOUND_TAG_ERROR: 	
				strcpy(s,"Parser error: problem with compound marking");
		 		break;
		case SUBORD_TAG_ERROR: 	
				strcpy(s,"Parser error: problem with subordinant clause marking");
		 		break;
	} 		
	
} // getParseError

void ParserClass:: addSegHeadTag(int loc, toktype tag, toktype index)
// adds indexed tag to snytArray[loc].pheadtag for segments
{
tokptr pt = &syntArray[loc].pheadtag;

	if (iTag >= MAX_TAGS - 3) 
	  parseError("Tag overflow in addHeadTag",HEAD_TAG_ERROR);	// error check, sort of -- this doesn't really recover, just notifies 

	while (*pt) pt = &tagArray[*pt+2];  // find last element in tag
	*pt = iTag;													// reset the next element pointer
	tagArray[iTag++] = tag;  						// add the tag
	tagArray[iTag++] = index; 				  // add index
	tagArray[iTag++] = 0; 							// zero link to next element 

} // addHeadTag

void ParserClass:: addHeadTag(int loc, toktype tag)
// adds non-indexed tag to snytArray[loc].pheadtag
{
tokptr pt = &syntArray[loc].pheadtag;

	if (iTag >= MAX_TAGS - 3) 
	  parseError("Tag overflow in addHeadTag",HEAD_TAG_ERROR);	// error check, sort of -- this doesn't really recover, just notifies 

	while (*pt) pt = &tagArray[*pt+2];  // find last element in tag
	*pt = iTag;													// reset the next element pointer
	tagArray[iTag++] = tag;  						// add the tag
	tagArray[iTag++] = 0; 							// zero index
	tagArray[iTag++] = 0; 							// zero link to next element 

} // addHeadTag

void ParserClass:: addHeadTag(int loc, toktype tag, toktype &index)
// adds next indexed tag to snytArray[loc].pheadtag; returns value of index
{
tokptr pt = &syntArray[loc].pheadtag;

	if (iTag >= MAX_TAGS - 3) 
	  parseError("Tag overflow in addHeadTag",HEAD_TAG_ERROR);	// error check, sort of -- this doesn't really recover, just notifies 

	index = 0;
	while (*pt) {
		if ((*(pt+1) == tag) && (*(pt+1)>index)) index = *(pt+1);  // set index to maximum current value
		pt = &tagArray[*pt+2];  // find last element in tag
	}
	++index;											// set new index value
	*pt = iTag;										// reset the next element pointer
	tagArray[iTag++] = tag;  		  // add the tag
	tagArray[iTag++] = index; 		// record index
	tagArray[iTag++] = 0; 				// zero link to next element 

} // addHeadTag

void ParserClass:: addSegTailTag(int loc, toktype tag, toktype index)
// adds indexed tag to snytArray[loc].ptailtag for segments
{
tokptr pt = &syntArray[loc].ptailtag;

	if (iTag >= MAX_TAGS)  
		parseError("Tag overflow in addClTailTag",TAIL_TAG_ERROR);

	while (*pt) pt = &tagArray[*pt+2];
	*pt = iTag;
	tagArray[iTag++] = tag;  
	tagArray[iTag++] = index; 	
	tagArray[iTag++] = 0;  
	
} // addTailTag

void ParserClass:: addTailTag(int loc, toktype tag)
// adds non-indexed tag to snytArray[loc].ptailtag
{
tokptr pt = &syntArray[loc].ptailtag;

	if (iTag >= MAX_TAGS)  
		parseError("Tag overflow in addTailTag",TAIL_TAG_ERROR);

	while (*pt) pt = &tagArray[*pt+2];
	*pt = iTag;
	tagArray[iTag++] = tag;  
	tagArray[iTag++] = 0; 	
	tagArray[iTag++] = 0;  
	
} // addTailTag

void ParserClass:: addTailTag(int loc, toktype tag, toktype index)
// adds next indexed tag to snytArray[loc].ptailtag; sets value of index
{
tokptr pt = &syntArray[loc].ptailtag;

	if (iTag >= MAX_TAGS)  
		parseError("Tag overflow in addTailTag",TAIL_TAG_ERROR);

	while (*pt) pt = &tagArray[*pt+2];
	*pt = iTag;
	tagArray[iTag++] = tag;  
	tagArray[iTag++] = index; 	
	tagArray[iTag++] = 0;  
	
} // addTailTag

bool ParserClass:: hasHeadTag(int loc, toktype tag)
// returns true if snytArray[loc].pheadtag contains non-indexed tag of type "tag"
{
int idx = syntArray[loc].pheadtag;

	while (idx) { 
		if (tag == tagArray[idx]) return true;
		idx = tagArray[idx+2];
	}
	return false;	
} // hasHeadTag

bool ParserClass:: hasHeadTag(int loc, toktype tag, toktype &index)
// returns true if snytArray[loc].pheadtag contains tag of type tag and 
// return value of index.  Index is set to zero if tag not found
{
int idx = syntArray[loc].pheadtag;

	while (idx) { 
		if (tag == tagArray[idx]) {
			index = tagArray[idx+1];
			return true;
		}
		idx = tagArray[idx+2];
	}
	index = 0;
	return false;	
} // hasHeadTag

bool ParserClass:: checkHeadTag(int loc, toktype tag, toktype index)
// returns true if snytArray[loc].pheadtag contains tag of type tag and 
// value of index.
// ### note that this and checkTailTag do different things...probably should synchronize them 
{
int idx = syntArray[loc].pheadtag;

	while (idx) { 
		if ((tag == tagArray[idx]) && (index == tagArray[idx+1])) return true;
		idx = tagArray[idx+2];
	}
	return false;	
} // checkHeadTag

bool ParserClass:: hasTailTag(int loc, toktype tag)
// returns true if snytArray[loc].ptailtag contains non-indexed tag of type "tag"
{
int idx = syntArray[loc].ptailtag;

	while (idx) { 
		if (tag == tagArray[idx]) return true;
		idx = tagArray[idx+2];
	}
	return false;	
} // hasTailTag

bool ParserClass:: hasTailTag(int loc, toktype tag, toktype index)
// returns true if snytArray[loc].ptailtag contains indexed tag of type "tag:index"
{
int idx = syntArray[loc].ptailtag;

	while (idx) { 
		if ((tag == tagArray[idx]) &&  (index == tagArray[idx+1])) return true;
		idx = tagArray[idx+2];
	}
	return false;	
} // hasTailTag

toktype ParserClass:: getTailTag(int loc, toktype tag)
// returns index if snytArray[loc].ptailtag contains tag of type tag and 
// returns value of index.  Returns zero if tag not found
{
int idx = syntArray[loc].ptailtag;

	while (idx) { 
		if (tag == tagArray[idx]) {
			return tagArray[idx+1];
		}
		idx = tagArray[idx+2];
	}
	return 0;	
} // hasTailTag

bool ParserClass:: checkTailTag(int loc, toktype tag)
// confirms that a corresponding tail exists for an indexed head tag at loc
// check is needed in case nullSubordinate eliminated the compound
{
toktype tagidx;

	if (hasHeadTag(loc, tag, tagidx)) {
		++loc;
		while ((loc <= iSynt) && (!hasTailTag(loc, tag, tagidx))) ++loc;
		if (loc > iSynt) return false;
		else return true;
	}
	else return false;  // ### shouldn't hit this...
} // checkTailTag

void ParserClass:: changeHeadTag(int loc, toktype oldtag, toktype newtag)
// changes oldtag at snytArray[loc].ptailtag to newtag.  
// If tag isn't present, does nothing.  
{
int idx = syntArray[loc].pheadtag;

	while (idx) { 
		if (oldtag == tagArray[idx]) {
			tagArray[idx] = newtag;
			return;
		}
		idx = tagArray[idx+2];
	}
} // changeHeadTag

void ParserClass:: changeTailTag(int loc, toktype oldtag, toktype newtag)
// changes oldtag at snytArray[loc].ptailtag to newtag.  
// If tag isn't present, does nothing. 
{
int idx = syntArray[loc].ptailtag;

	while (idx) { 
		if (oldtag == tagArray[idx]) {
			tagArray[idx] = newtag;
			return;
		}
		idx = tagArray[idx+2];
	}
} // changeTailTag

toktype ParserClass:: getReference(int loc)
// gets a pronoun reference from the headtag of syntArray[loc]  
{
int idx = Parser.syntArray[loc].pheadtag;

	while (idx) { 
		if (tagArray[idx] >= setRefer) return (tagArray[idx] & maskRefer);
		else idx = tagArray[idx+2];
	}
	parseError("\aMissing pronoun reference tag in ParserClass:: getReference",PRONOUN_REF_ERROR); // we should never hit this...
	return 0;																		 
} // getReference

void ParserClass:: addLitr (toktype ilit, toktype icomp)
// add the literal index ilit and comparison type icomp to litArray
// ### [09.06.26]: this doesn't return any sort of error indicating that MAX_LITR has been exceeded,
//     so it continues to get called when it isn't doing anything. Not a big deal but somewhat inefficient   
{
	if (iLitr < MAX_LITRLIST - 1) {
		literaList[iLitr++] = ilit;
		literaList[iLitr++] = icomp;
	}
	else parseError("Too many words in story (iLitr > MAX_LITRLIST in ParserClass:addLitr); remaining matches ignored",TOO_LONG_ERROR);
} // addLitr

int ParserClass:: hasLitr (int index, toktype litr)
// checks for literal in the list for syntArray[index]
// returns 
//      0   if not in list, or if list doesn't exist  
//      1   if litr is in the list and is a partial match  
//      2   if litr is in the list and is an exact match 
{
	if (syntArray[index].ilit <= 0) return 0;
//	Processor.fprob << "hL >> hasLitr call " << endl;  // *** debug
	
	toktype * ptok = &literaList[syntArray[index].ilit];	
	while (*ptok) {
		if (*ptok == litr) {
			++ptok;
			if (*ptok) return 2;
			else return 1;
		}
		ptok +=2 ;
	}
	return 0;
} // hasLitr

bool ParserClass:: skipSegment(syntStruct * &psyn, wordtype wtype)
// 	check whether *psyn is a segment (i.e. clause, compound or subord) head; 
//	if true, increment psyn to point to the first word beyond the end of the 
//	segment.  Returns false if no changes in psyn
//	Notes: 
//  1. this function has *major* side-effects on psyn and is called from within 
//		 'if ()' statements, so this might not be the smartest way to do this. 
{
toktype segindex;
//	Parser.writeParsing("Called from skipSegment 1 ");  // *** debug

	if (!hasHeadTag(psyn->index,wtype, segindex)) return false;  // picks up segindex
//	cout << "PC:sS1: " << psyn->index << endl;
	while (!hasTailTag(psyn->index,wtype, segindex)) {
		++psyn;
	}
	++ psyn;
	return true;
} // skipSegment

bool ParserClass:: skipSegment(int &index, wordtype wtype)
// 	this variant works directly with the index rather than a pointer.
//	Again, the function has *major* side-effects 
{
toktype segindex;

	if (!hasHeadTag(index,wtype,segindex)) return false;
//	cout << "PC:sS2: " << index << endl;
	while (!hasTailTag(index,wtype,segindex)) {
		++index;
	}
	++index;
	return true;
} // skipSegment

void ParserClass:: filterText(void)
// handles an assortment of filtering tasks as it moves sentText to senfilt
// Punctuation filtering:
//	1. Removes '.','?','!','-' 
//	2. Replaces ';',':','(', and ')'  with ','
//	3. Space-delimits ',' and '"', '[', '{', '}',and ']' 
//	4. Eliminate commas inside numbers
//	5. Preserves periods inside numbers 
//	6. Preserves periods in capitalized abbreviations (e.g. U.S., U.N.)
//	7. Preserves apostrophes, $
//	8. Eliminates remaining punctuation
//  9. Eliminates consecutive blanks
// 10. Removes text inside /*...*/
// 11. In FBIS mode, removes text inside '[' and ']' 
// 12. In FBIS mode, skip leading non-alphabetic text 
//
// ### additional optional tasks this could do from the KEDS model [03.06.29]
// 1. Replacement rules
// 2. General Omit delimiters (/*  */ is implemented)
{
char *pstx = Processor.sentText;
char *psf	 = filtext;
int  kchar;

//strcpy(Processor.sentText ,"This, isn't a test-test \"my,friend\" $2,300.12?! (yeah, right) omit /*this*/ text"); // *** debug
	fRecCAPS = true; 
	
	if (TabariFlags.fFBISMode) {
		while ((*pstx) && ('[' != *pstx) && (!isalpha(*pstx))) ++pstx; // skip initial non-alphabetic text
	}
	while (*pstx) {
		if (fRecCAPS && (islower(*pstx))) fRecCAPS = false; // check whether this is an all caps record
		
		if (('/' == *pstx) && ('*' == *(pstx+1))) {  // remove text between /*  */
			while ((*pstx) && (('*' != *pstx) || ('/' != *(pstx+1)))) ++pstx;
			if ('*' == *pstx) pstx +=2;
		}
		else if ((TabariFlags.fFBISMode) && ('[' == *pstx)) {  // remove text between [  ] in FBISMode
			while ((*pstx) && (']' != *pstx)) ++pstx;
			if (']' == *pstx) ++pstx;  // #### error check needed here?
		}
		else if (ispunct(*pstx)) {
			if ((';' == *pstx) || (':' == *pstx)   // replace ( ) ; and : with ,
				 || ('(' == *pstx) || (')' == *pstx)) {
				*psf++ = ' ';
				*psf++ = ',';
				*psf++ = ' ';
			}
			else if (',' == *pstx) {  // handle commas
				if (!isdigit(*(pstx-1)) || !isdigit(*(pstx+1))) {  // space delimit comma; otherwise it gets eliminate
					*psf++ = ' ';
					*psf++ = ',';
					*psf++ = ' ';
				}
			}
			else if ('.' == *pstx) {  // handle periods
				if ((isdigit(*(pstx-1))) && 
						(isdigit(*(pstx+1)))) *psf++ = '.'; 				// carry through inside numbers
				else if (isupper(*(pstx-1))) *psf++ = '.'; 			// and in capitalized abbreviations
				else	*psf++ = ' ';  // replace with space
			}
			else if ('\'' == *pstx) *psf++ = '\'';
//			else if ('-' == *pstx) *psf++ = '-';  // [09.06.29] eliminate hyphens
			else if ('$' == *pstx) *psf++ = '$';
			else if ('"' == *pstx) {
				*psf++ = ' ';
				*psf++ = '"';
				*psf++ = ' ';
			}
			else *psf++ = ' ';
		++pstx;			// move to next char
		}
		else *psf++ = *pstx++;	// transfer all other text
	} // while
	
	if (*(psf-1) != ' ') *psf++ = ' ';  // make sure we've still got a terminal blank
	*psf = '\0';
	
	// eliminate consecutive blanks; make sure no word is longer than LIT_SIZE - 1
	psf = filtext;
	pstx = filtext;
	kchar = 0;				// number of non-blanks chars
	while (*pstx) {
		if (' ' == *pstx) {
			kchar = 0;		// reset counter
			if (' ' == *(pstx+1)) ++pstx; // eliminate consecutive blanks
			else *psf++ = *pstx++;
		}
		else {
			if (++kchar >= LIT_SIZE) {
				*(psf-1) = ' ';			// just blank it out -- this only occurs in junk text anyway
				kchar = 1;
			}
			*psf++ = *pstx++;	
		}
	}
	*psf = '\0';
	
//	cout << filtext << endl;  // *** debug 
} // filterText

bool ParserClass:: matchLex(int &nmatch, tokptr phrase, wordtype wtype,int ibegin)
//	attempts to match root phrase, beginning at lexArray[ibegin].  nmatch is set to the
//	total number of characters matched 
//  Note that this is called from the *syntactical* analysis level. 
//  <08.12.29>: Checks for the "S" and "'S" endings on actors and agents, and various regular verb endings
{
lexStruct * plex = &lexArray[ibegin];
lexStruct * plast = &lexArray[iLex];
bool fskip = false; // ok to skip intermediate words?
bool fmatch = false;
toktype * ptok;			// pointer to literal list
char * pendtext = 0;  // pointer to end of literal in the text

#if DEBUG_LEX
instring s; 				// *** debugging
WriteLine("mLex >> enter");		// *** debugging
#endif

	iTemp = 0;
	nmatch = 0;
	while ((*phrase) && (plex <= plast)){  // go through the phrase list
#if DEBUG_LEX
		WriteLine("mLex check : ",Phrases.get(s, phrase));		// *** debugging
#endif
		fmatch = false;
		if (-COMMA_LIT == plex->ilit) {
		return false; // [05.06.24] don't allow matches across commas ### <09.01.02> more generally, we don't want to match across *clauses*, but we don't have this information yet...
		}
		if (plex->ilit) {								// check whether there is a match anywhere in the list
			ptok = &literaList[plex->ilit];
			while (*ptok) {
#if DEBUG_LEX
				WriteLine("mLex compare: ", Literals.litArray[*ptok].pchar);		// *** debugging
#endif
				if (*phrase == *ptok) {
					fmatch = true;
					nmatch += Literals.litArray[*ptok].length;
					break;
				}
//				++ptok; ### pre-connector code
				ptok += 2;
			} 					// while (*ptok)
		}
		if (!fmatch) {		// mismatch
			if (fskip) {			// okay to skip
				++plex;		// get next entry in lexArray
				continue;
			} 
			else return false;
		}
		// check if a partial match is ok by checking connector
#if DEBUG_LEX
		WriteLine("mLex checking connector"); 
#endif
		if (iTemp >= MAX_TEMP) return false;  // bounds check
		tempArray[iTemp++] =  (int)(plex - lexArray);  // store the word index
		++phrase;	// *phrase is now the connector
		pendtext = plex->ptext + Literals.litArray[*ptok].length;  // pointer to end of literal in the text + 1
		if ((*pendtext != ' ') &&             // was the entire word matched?
//				!(*phrase & Phrases.connWild)) {
				!(*phrase & Phrases.connFullSkip)) {
			if (*phrase == Phrases.connEqual) return false; // root ended with = so endings are not allowed
			if ((Actor == wtype) || (Agent == wtype) || (Noun == wtype)) {	//  check for possible noun phrase endings
				if (*(phrase+1)) return false;   // not the last element in the phrase
				if (('S' == *pendtext) && (' ' == *(pendtext+1))) {
					++nmatch;
					return true;
				}
				else if (('S' == *(pendtext+1)) && (' ' == *(pendtext+2)) && (('\'' == *pendtext) || ('E' == *pendtext))) {
					nmatch += 2;
					return true;
				}
			}
			if (Verb == wtype) {	//  check for possible regular verb endings
				if (*(phrase+1)) return false;   // not the last element in the phrase
				if (('S' == *pendtext) && (' ' == *(pendtext+1)))  {
					++nmatch;
					return true;
				}
				else if (('E' == *pendtext) && (' ' == *(pendtext+2)) && (('D' == *(pendtext+1)) || ('S' == *(pendtext+1)) || ('N' == *(pendtext+1)))) {
					nmatch += 2;
					return true;
				}
				else if (('I' == *pendtext) && ('N' == *(pendtext+1)) && ('G' == *(pendtext+2)) && (' ' == *(pendtext+3))) {
					nmatch += 3;
					return true;
				}
				else if (('E' == *(pendtext-1)) && ('D' == *pendtext) && (' ' == *(pendtext+1))) {
					nmatch += 1;
					return true;
				}
			}
			return false;  // return since this was the last part of the phrase
		}
//		if (*phrase & Phrases.connSpace) fskip = true;		// check whether we need consecutive words [old version]
		if (*phrase & Phrases.connPartNext) fskip = true;		// check whether we need consecutive words
		else fskip = false;
		++plex;			// get next entry lexArray
		++phrase;		// get next part of phrase
	} // while
	
	if (*phrase) return false;  // phrase was not completely matched; use this rather than "true" 
                                // since otherwise multi-word, skip-connected phrases will incorrectly match.  See DEBUG-01 
	else return fmatch;

} // matchLex

void ParserClass:: makeLex(void)
// convert senfilt to lexArray
{
char *pst = filtext;
char *psa;
litstring s;
lexStruct * plex;
bool first = true;
bool skip = false;
toktype itok;

	iLex = -1;
	iLitr = 1;   // literaList[0] is unused because ilit=0 signals no literals were matched
	
	while (' ' == *pst) ++pst; // go to first non-blank char
	while (*pst) {
		++iLex;
		if (iLex >= MAX_LEX-2) { // leave room for defaults; note that if we actually hit this, forwarding would also not work
			--iLex;  	// leave room for 'Halt' record in syntArray
			break;  	// truncate 
		}
		plex = &lexArray[iLex];
		plex->ptext = pst;			// record start of text in senfilt
		plex->flags = 0;				// initialize flags
		if (!fRecCAPS) {
			if (isupper(*pst)) {		// set the various flags before moving to upper case
				if (first) {
					plex->flags = setInitCap;
					first = false;
				}
				else plex->flags = setMidCap;
			}
		}
		if (isdigit(*pst)) {  // check for <number>
 			 psa = Copy2Chr(s,pst,' ');
 			 if (!Literals.check(s)) {  // check that this wasn't a literal, e.g. as in "Group of 77"
				 plex->flags = setNumber; 
				 plex->ilit = -NUMBER_LIT;		// reverse signs on symbols
				 plex->pend = strchr(pst,' ') - 1;
				 skip = true;				// skip additional processing
			 }
		}
		else if (',' == *pst) { // set to <comma>
			 plex->flags = setComma;
			 plex->ilit = -COMMA_LIT;
			 plex->pend = pst;
			 skip = true;				// skip additional processing
		}
		else if (REPLACE_LIT == *pst) {
			 plex->flags = setReplace;
			 plex->ilit = -REPLACE_LIT;
			 plex->pend = pst;
			 skip = true;				// skip additional processing
		}

		if (!skip) {
			psa = pst;			// shift filtext to upper case now
			if (!fRecCAPS) {
				while (*psa != ' ') {
					*psa = toupper(*psa);
					++psa;
				}
			}
			psa = Copy2Chr(s,pst,' ');
			
			itok = iLitr;  							// save the starting point of possible literal list
			if (Literals.check(s)) {
				plex->ilit = itok; 
				plex->pend = pst + (psa - s);  // set pend to work with filtext ### no longer needed, right?
			}
			else {
				plex->ilit = 0;
				plex->pend = NULL; // ### no longer needed??
			}
		}
		else skip = false;   // reset skip
		
		pst = strchr(pst,' ');
		if (pst) ++pst;
		else break;  // ### <08.06.19> since space is added at end, we should never hit this, but a text from
								 // SAE containing massive numbers of non-printing chars (see Validation Record xx) led to an 
		             // error here before the 'else' was added. On the other hand,if pst == NULL we need to do
								 // *something*, so the original code was flawed. Still don't understand what was happening 
								 // here, however...
		while (' ' == *pst) ++pst; // go to next non-blank char

	} // while
	
	if (iLex < minLex) {
		iparseError = TOO_SHORT_ERROR;
		throw TOO_SHORT_ERROR;
	}
		
#if FALSE
		for (int ka=0; ka<=iLex; ++ka) {		// *** debugging output
			WriteLong("iLex ",(long)ka);
			WRITESTRING("  ptext ");
			pst = lexArray[ka].ptext;
			while (*pst != ' ') WRITECHAR(*pst++);
			WRITEEOL();
			WRITESTRING("  pend  "); 
			if (lexArray[ka].pend) WriteLine(*(lexArray[ka].pend));
			else WriteLine("null");
			WriteLong("  ilit ",lexArray[ka].ilit);
		}
		Pause();
#endif
} // makeLex

void ParserClass:: makeSynt(void)
// fill in syntArray from lexArray; assign roots
{
int 		ka;
syntStruct * psyn = syntArray;
tokindex itok;
RootsClass::rootStruct root;
int 		idxlit;		// index to start of current literal list
toktype * ptok;	 	// pointer to literal being checked
int 		nmatch;				// chars matched for current root
int 		maxmatch;			// info on root with maximum number of matched chars 
wordtype maxword;
tokindex maxtok;
bool 		inreplace = false;  // flag for inside replace markers
#if DEBUG_LEX
char 		sout[32];
#endif
	
	if (fuseForward) setForward();
	else iTag = 1;	// 1 because 0 is used as the end-of-list indicator

	for (ka = 0; ka<=iLex+1; ++ka) {  // initialize array
		syntArray[ka].index 	= ka;		// index of array
		syntArray[ka].ilit 		= lexArray[ka].ilit;		// start of literaList list
		syntArray[ka].iroot 	= 0;		// index of rootArray entry
		syntArray[ka].wtype 	= Null;     // word type
		syntArray[ka].pheadtag 	= 0;		// tag list prior to word
		syntArray[ka].ptailtag 	= 0;		// tag list following word
		syntArray[ka].flags 	= 0;		// indicator flags
	}
	iSynt = iLex+1;
	lastSynt = iSynt;
	syntArray[iSynt].wtype   = Halt;	// signal end of sentence
	syntArray[iSynt].ilit    = 0;

	while (psyn->wtype != Halt) {	 // assign types and roots
		idxlit = psyn->ilit;
#if DEBUG_LEX
		WriteLong("mSynt index : ", (long)psyn->index);
#endif
		if ((inreplace) && (-REPLACE_LIT != idxlit)) {	// don't process text inside replace markers
			++psyn;
			continue;
		}
		if ((idxlit) && (Null == psyn->wtype)) {		// second condition prevent re-assigning types in compound roots
			if (idxlit < 0) {											// assign types to numbers, commas
				idxlit = -idxlit;			// symbol, so reverse sign
#if DEBUG_LEX
				WriteLine("mSynt result >> symbol or number");
#endif
				if (COMMA_LIT == idxlit) psyn->wtype = Comma;
				else if (NUMBER_LIT == idxlit) psyn->wtype = Number;
				else if (REPLACE_LIT == idxlit) {
					psyn->wtype = Replace;
					inreplace = !inreplace;
				}
			}
			else {   // check for root
				maxmatch = 0;
				ptok = &literaList[idxlit];
				while (*ptok) {												// go through the literals list
#if DEBUG_LEX
					WRITESTRING("mSynt check'em lit :");
					WRITESTRING(Literals.litArray[*ptok].pchar);
					sprintf(sout,"  (index=%3d)",idxlit);   // *** debug
					WriteLine(sout);
#endif
				  itok = Literals.litArray[*ptok].istart;		// go through istart list
					while (itok) {
						root =  Roots.rootArray[Literals.litBuf[itok]];
						if (Null == root.wtype) {			// root has been deleted
							itok = Literals.litBuf[itok+1];		// continue through the istart list
							continue;	
						}			
#if DEBUG_LEX
					  WriteLine("mSynt check root:",Literals.litArray[*(root.phrase)].pchar);   // *** debug
#endif
						if (matchLex(nmatch,root.phrase, root.wtype, psyn->index)) { 
							if (nmatch > maxmatch) {			// save the phrase if it is longer than current best match
								for (ka = 0; ka < iTemp; ++ka) matchArray[ka] = tempArray[ka];
								iMatch = iTemp;
								maxmatch = nmatch;		// update the maximum chars matched
								maxtok = itok;				// save itok so we can retrieve root that generated this
								maxword = root.wtype;
							}
#if DEBUG_LEX
							WriteLine("mSynt >> match");
#endif
						}
#if DEBUG_LEX
						else 	WriteLine("mSynt >> failed");
#endif
						itok = Literals.litBuf[itok+1];		// continue through the istart list
					}	// while itok
//					++ptok; ### pre-continuation version
					ptok += 2;
				} // while (*ptok) 
				
				if (maxmatch) {
					toktype index;
					psyn->iroot = Literals.litBuf[maxtok]; // record the root
					++Roots.rootArray[psyn->iroot].used;				 // increment .used counter
					addHeadTag(psyn->index,maxword,index);
					for (ka = 0; ka < iMatch; ++ka)	syntArray[matchArray[ka]].wtype = maxword;
					addTailTag(matchArray[iMatch-1],maxword,index);
				}
			} // else
		} // if idxlit
#if DEBUG_LEX
		else 	WriteLong("mSynt skip:type ",(long)psyn->wtype);   
#endif
		++psyn;
	} // while psyn->wtype
	
} // makeSynt

void ParserClass:: setForward(void)
// check if forwarding can be done; transfer first actor and compound from
// previous record to area past 'Halt' record in syntArray, then move relevant
// tag information to beginning of tagArray, adjusting the pointer and links.
// Note that this operates on the syntArray and tagArray left over from the
// previous record, and must be called before these are over-written.
// Currently does not handle secondary forwards (i.e. forward from a pronoun 
// reference)
{

	idxForwActor = -1;
	idxForwCompd = -1;
	if (!fcanForward) {  // new story or skipped sequence number
		iTag = 1;	// 1 because 0 is used as the end-of-list indicator
		return;
	}
	
int saveTag = iTag;
syntStruct * psyn   = syntArray;
int itrans;						// index of cells reference will transfer into
syntStruct * ptrans;  // pointer to same
int savetrans;	

    if (lastSynt > iLex + 1) itrans = lastSynt + 1;  // start transfer beyond previous and current iSynt
    else                     itrans = iLex + 2;
    ptrans = &syntArray[itrans];
    savetrans = itrans;
  
	while ((idxForwActor < 0) && (idxForwCompd < 0) && 
					(psyn->wtype != Halt)) {   // find first actor and compound
		if (hasHeadTag(psyn->index,Compound)) {  // transfer compound
			if (idxForwCompd >= 0) {  // we already have compound, so skip over this
				while (!hasTailTag(psyn->index,Compound)) ++psyn;
			}
			else {
				idxForwCompd = itrans;  
				ptrans->pheadtag = 0;  // zero out whatever is left over here
				ptrans->ptailtag = 0;
				addHeadTag(itrans, Compound); 
				while (psyn->wtype != Halt) {
					ptrans->index = itrans;
					ptrans->iroot = psyn->iroot;  // root and wtype are the only info we can save
					ptrans->wtype = psyn->wtype;
					ptrans->ilit  = -1;   		// literal list no longer valid
					ptrans->flags =	setCompd;
					if (hasHeadTag(psyn->index, Actor)) addHeadTag(itrans, Actor); // copy actor tags
					if (hasTailTag(psyn->index, Actor)) addTailTag(itrans, Actor);

					++ptrans;
					++itrans;
					ptrans->pheadtag = 0;
					ptrans->ptailtag = 0;
					if (hasTailTag(psyn->index,Compound)) break; // normal exit for loop
					++psyn;
				}
				addTailTag(itrans-1, Compound);
			}
		} // if Compound

		if ((idxForwActor < 0) && 
		    (hasHeadTag(psyn->index,Actor)) &&
		    !(hasHeadTag(psyn->index,Pronoun))) {  // (can't handle a pronoun)
			idxForwActor = itrans;    				// transfer actor
			ptrans->index = itrans;
			ptrans->iroot = psyn->iroot;  // root and wtype are the only info we can save
			ptrans->wtype = psyn->wtype;
			ptrans->ilit  = -1;   		// literal list no longer valid
			ptrans->flags = 0;  
			ptrans->pheadtag = 0;  // zero out whatever is left over here
			ptrans->ptailtag = 0;
			addHeadTag(itrans, Actor);  
			addTailTag(itrans, Actor);
			++ptrans;
			++itrans;
		}
		++psyn;
	}

	if (idxForwCompd < 0) idxForwCompd = idxForwActor;  // if no syntactic compound found, just use the actor

	// move new entries in tagArray to beginning
toktype * ptag = &tagArray[1];
toktype * ptb = &tagArray[saveTag];
int ka = 2;

	while (ptb <= &tagArray[iTag]) *ptag++ = *ptb++;  // transfer tags to start of tagArray	
	while (ka <= (iTag - saveTag)) {	// adjust the links in tagArray
		if (tagArray[ka]) tagArray[ka] -= (saveTag - 1);
		ka += 3;
	}

	psyn = &syntArray[savetrans];	// adjust the phead and ptail pointers
	while (psyn < ptrans) {
		if (psyn->pheadtag) psyn->pheadtag -= (saveTag - 1);
		if (psyn->ptailtag) psyn->ptailtag -= (saveTag - 1);
		++psyn;
	}
	iTag = ka - 1;

} // setForward

void ParserClass:: markSubordinate(void)
//	find the subordinate clause boundaries. This looks for a comma, counts
//	words until next comma or halt, then checks if it satisfies the max, min criteria.
//	If it does, add segment tags and then nullSubordinate will null-out everything. 
{
syntStruct * psyn = syntArray;
syntStruct * pstart;
toktype subindex = 1;   // index for the subordinate clauses

/*    Processor. fprob << "pc:Ms:\n" <<  Parser.minSubord<< "  " << Parser.maxSubord << endl;
    Processor. fprob <<  Parser.minBegSubord<< "  " << Parser.maxBegSubord << endl;
    Processor. fprob <<  Parser.minEndSubord<< "  " << Parser.maxEndSubord << endl; */
	while (psyn->wtype != Halt) {   
		if (skipSegment(psyn,Compound)) continue;		// skip across compounds
		if (Comma == psyn->wtype) {
            
            pstart = psyn;
 
			if (((psyn - syntArray) >= minBegSubord) &&  // handle initial clause
                ((psyn - syntArray) <= maxBegSubord)) {
//                Processor. fprob << "syntArray " << syntArray << " psyn 0 " << psyn<< " diff " << (psyn - syntArray)  << endl;
				addSegHeadTag(syntArray->index, Subord, subindex);
				addSegTailTag((pstart-1)->index, Subord, subindex);
				++subindex;
			} // note that this is chained and we continue to look for an internal phrase now

			++psyn;															// find the next comma outside of a compound
			while ((Comma != psyn->wtype) &&
						 (Halt  != psyn->wtype)) {
				if (skipSegment(psyn,Compound)) continue;		// skip across compounds
				else ++psyn;
			}

			if ((Halt == psyn->wtype) &&								// handle terminal clause case
					((psyn - pstart) - 1 >= minEndSubord) &&
					((psyn - pstart) - 1 <= maxEndSubord)) {
//                Processor. fprob << "pStart 2 " << pstart << " psyn 2 " << psyn<< " diff " << (psyn - pstart)  << endl;
				addSegHeadTag(pstart->index, Subord, subindex);
				while (Halt != pstart->wtype) ++pstart;
				addSegTailTag((pstart-1)->index, Subord, subindex);
				++subindex;
				return;
			}
//            else Processor. fprob << "** pStart 2 " << pstart << " psyn 2 " << psyn<< " diff " << (psyn - pstart)  << endl;

				
			if ((Comma == psyn->wtype) &&								// handle intermediate clause
					((psyn - pstart) - 1 >= minSubord) &&
					((psyn - pstart) - 1 <= maxSubord)) {
//                Processor. fprob << "pStart 3 " << pstart << " psyn 3 " << psyn<< " diff " << (psyn - pstart)  << endl;
				addSegHeadTag(pstart->index, Subord, subindex);
				while (pstart < psyn) ++pstart;
				addSegTailTag(pstart->index, Subord, subindex);
				++subindex;
			}
//            else Processor. fprob << "** pStart 3 " << pstart << " psyn 3 " << psyn<< " diff " << (psyn - pstart)  << endl;

		}	
		if (Halt != psyn->wtype) ++ psyn;
	} // while psyn
} // markSubordinate

void ParserClass:: makeFBISCompound(void)
//	in FBIS_Mode, converts <Actor><Comma><Actor> to <Actor><Conj><Actor>
// ### <03.06.10> does not handle multi-word actors
// ### <03.06.12> in fact it doesn't really work at all, and can end up
//                crashing the program, probably because I end up jumping
//                over the Halt, which I knew was a potential problem...
//                currently deactivated.
{
syntStruct * psyn = syntArray;
syntStruct * psya;

	// cout << "mFCS1\n";
	while (psyn->wtype != Halt) {   
		if ((Actor == psyn->wtype) &&
		 (Comma == (psyn+1)->wtype) &&
		 (Actor == (psyn+2)->wtype)) {
		 	psya = psyn+3;
		 	while (((Comma == psya->wtype) || (Conj == psya->wtype)) &&  // check for comma-delimited list
		 	       (Actor == (psya+1)->wtype)) psya += 2;
		  if (Conj == (psya-2)->wtype) psyn = psya - 1;  // it was a list, so skip
		  else {
		  	(psyn+1)->wtype = Conj;  // change to a conjunction
		  	psyn +=2;
		  }
		 }
		if (Halt != psyn->wtype) ++psyn;
	} // while psyn
	// cout << "mSx2\n";

} // makeFBISCompound

void ParserClass:: nullSubordinate(void)
// Null-out everything within the subordinate clause boundaries. 
// This is called after the coding of syntactic attribution; if attribution is
// not being coded, the nulling is done immediately after  markSubordinate()
{
syntStruct * psyn = syntArray;
syntStruct * psya;
int idx;
int tag;

//	 cout << "nS1\n";
//	Parser.writeParsing("Called from nullSubordinate 1 ");  // *** debug
    if (hasHeadTag(psyn->index,Subord)) psyn->wtype = Null; // deal with initial clause
    
	while (Halt != psyn->wtype) {  
		if (hasHeadTag(psyn->index,Subord)) { // <08.06.16> Currently subord clauses aren't indexed since they have highest precedence. Might add this at some point in the future.
			do {
				++psyn;

				psyn->wtype = Null;

				if (psyn->pheadtag > 0) {		// has head tags, so find the tail and deactivate it
					idx = psyn->pheadtag;
					while (tagArray[idx]) {  // go through all tags
						if (NullTag != tagArray[idx]) {
							tag = tagArray[idx];
							psya = psyn;
							while ((!hasTailTag(psya->index,tag)) &&  // find the corresponding tail
											(psya->wtype != Halt)) ++psya; 
							if (psya->wtype == Halt) {
								parseError("Tail tag not found in nullSubord",SUBORD_TAG_ERROR); // shouldn't hit this, so no recovery
								Pause();
								return;
							}
						psyn->wtype = NullTag;  // signal that type has been cancelled
						changeHeadTag(psyn->index,tag, NullTag);
						changeTailTag(psya->index,tag, NullTag);  // note that NullTag rather than Null is used since tagArray segments are null-terminated
						}
						idx = tagArray[idx+2];  // tagArray is a linked list
					} // while tagArray
				} // if headtag

				if (psyn->ptailtag > 0) {		// check that all tailtags are inactive
					idx = psyn->ptailtag;
					while (tagArray[idx]) {  // go through all tags
						if ((NullTag != tagArray[idx]) &&  // this occurs if a tag was set across the subord clause, so go back and deactivate the head
						    (Subord  != tagArray[idx]))  {   // retain the subord tail						 
							tag = tagArray[idx];
							psya = psyn;
							while ((!hasHeadTag(psya->index,tag)) &&  // find the corresponding head
											(psya->index > 0)) --psya; 
							if (psya->index == 0) {
								if (!hasHeadTag(psya->index,tag)) {
									parseError("Head tag not found in nullSubord",HEAD_TAG_ERROR); // shouldn't hit this, so no recovery <05.05.09: hasn't been a problem yet...>
									Pause();
									return;
								}
							}
						changeTailTag(psyn->index,tag, NullTag);
						changeHeadTag(psya->index,tag, NullTag);  // note that NullTag rather than Null is used since tagArray segments are null-terminated
						psya->wtype = NullTag;  // signal that type has been cancelled
//					 cout << "nS2: cancelled tail\n";
						}
						idx = tagArray[idx+2];  // tagArray is a linked list
					} // while tagArray
				} // if tailtag
			
			} while (!hasTailTag(psyn->index,Subord));
		}		 

		if (Halt == psyn->wtype) return;
		++psyn;
	} // while psyn
//	 cout << "nSx\n";

} // nullSubordinate

void ParserClass:: markCompound(void)
//  Checks for a syntactic compound actor. This is triggered by 'AND' and it
//  handles actor lists of the form 			}
//   		 CA ::= <Actor> AND <actor> | <actor> , CA
//	Markup will also skip across <Agent> in locating an adjacent <Actor>, and across
//  <Adjtiv> and <Determ> in locating <Comma>.
// ### Things this may still need
//	1. Doesn't cross agent roots
//	2. Doesn't handle a Determ or Adjctv prior to the actor in the reverse search
//     <05.06.20> huh?? -- looks like it would...

{
syntStruct * psyn = syntArray;
syntStruct * psa;
syntStruct * pset; // start, end of compound
syntStruct * pend;
bool fneedConnect = false; // need comma or conj to continue phrase
toktype cmpindex = 1;
//	Parser.writeParsing("Called from markCompound 1 ");  // *** debug

	while (psyn->wtype != Halt) {
		if ((Conj == psyn->wtype) && ( 
		    (hasLitr(psyn->index, Roots.litAND) || hasLitr(psyn->index, Roots.litNOR)) ||
		     (TabariFlags.fFBISMode && (!psyn->iroot)))) {  // second test is true if this is a comma converted to conj
			pend = NULL;
			pset = NULL;
			psa = psyn + 1;		// check for actor following AND
			while ((Actor == psa->wtype) ||	
						 (Determ == psa->wtype) ||
						 (Adjctv == psa->wtype)) 
			{
				if ((Actor == psa->wtype) &&
						 hasHeadTag(psa->index,Actor)) { 				// move across entire root
					while (!hasTailTag(psa->index,Actor)) ++psa;
					pend = psa;  // mark tentative end
				}
				++psa;
			}
			if (!pend) {
				++psyn;		// no actor following, so 
				continue; // continue the while psyn loop
			}

			psa = psyn;										// now check for actor[s] prior to AND
			while ((psa >= syntArray) && 
						 ((Actor == psa->wtype) ||	
						   (Conj == psa->wtype) || (Comma == psa->wtype) ||
						 (Determ == psa->wtype) || (Adjctv == psa->wtype))) {

//				cout << "mC1: " << psa->index << " " << tagText[psa->wtype] << endl;
				if ( (Conj == psa->wtype) || (Comma == psa->wtype) ) 	fneedConnect = false; 					
				if (Actor == psa->wtype) { 				// move across entire root
					if (fneedConnect) break; 				// no connector, so extension fails
					while (!hasHeadTag(psa->index,Actor)) --psa;
					pset = psa;		// tentative start of compound
					fneedConnect = true; 					
				}
//				cout << "mC2: " << psa->index << endl;
				--psa;
			}
			
			if (pset) {   // we have both components, so mark the compound
				addSegHeadTag(pset->index, Compound, cmpindex);							
				addSegTailTag(pend->index, Compound, cmpindex);
				// null any earlier compounds marked inside the current one
				psa = pset;
				while (psa <= pend) {
					toktype idx;
					int loc;
					if (hasHeadTag(psa->index,Compound,idx) && (idx < cmpindex)) { // previous compound was set
						loc = psa->index;
						while ((Halt != psa->wtype) && !hasTailTag(loc, Compound, idx)) loc++;
						if  (Halt != psa->wtype) {
							changeHeadTag(psa->index,Compound	, NullTag); 
							changeTailTag(loc, Compound, NullTag);
						}
						else parseError("Unbalanced compound tags in markCompound",COMPOUND_TAG_ERROR);	
					}
					++psa;
				}						
				++cmpindex;	
				while (pset <= pend) {			// set the compound flags
					pset->flags |= setCompd;
					++pset;
				}						
			} 
		}	// if litAND
		++ psyn;
	} // while psyn
//	Parser.writeParsing("Called from markCompound 2 ");  // *** debug

} // markCompound

void ParserClass:: markNouns(void)
// change any "verbs" capitalized in mid-sentence or preceded by determiners to nouns
// also convert null-coded actors to nouns
{
syntStruct * psyn = syntArray;
toktype tagindex;

//	writeParsing("Parsing on entry to markNouns"); // *** debug
	while (psyn->wtype != Halt) {
		if ((Verb == psyn->wtype) && (psyn > syntArray)) {  
			if ((Determ == (psyn-1)->wtype) || 								// check for determiner before verbs
			    (lexArray[psyn->index].flags & setMidCap)) {	// check verb capitalization 
				psyn->wtype = Noun;
				if (hasHeadTag(psyn->index,Verb, tagindex)) {  // cancel the tags
					syntStruct * psya = psyn;
					changeHeadTag(psyn->index,Verb, Noun);
					while (!hasTailTag(psya->index,Verb, tagindex)) {  // move to end of multiple word phrases
						++psya;
						if (psya->index > iLex) {
							parseError("problem with verb to noun marking",MARK_NOUN_ERROR);
							--psya;
							break;  // ### [09.02.03] not the most elegant recovery, but keeps things moving.
						}		
						if ((Verb == psya->wtype) && !(hasHeadTag(psya->index,Verb))) psya->wtype = Noun;  // second test preserves any embedded verbs
					}
					changeTailTag(psya->index,Verb, Noun);
				}
			}
		}
		else if ((hasHeadTag(psyn->index,Actor,tagindex)) &&
						 (Roots.isNullActor(psyn->iroot))) {	// change null-coded actors to nouns
			changeHeadTag(psyn->index,Actor, Noun);
			psyn->wtype = Noun;
			while (!hasTailTag(psyn->index,Actor,tagindex)) {
				++psyn;
				if (psyn->index > iLex) {
					parseError("problem with null coded actor to noun marking",MARK_NOUN_ERROR);
					--psyn;
					break;  // ### see note above
				}		
				if ((Actor == psyn->wtype) && !(hasHeadTag(psyn->index,Actor))) psyn->wtype = Noun; // second test preserves any embedded actors
			}
			changeTailTag(psyn->index,Actor, Noun); 
		}
		++psyn;
	} // while psyn

} // markNouns

void ParserClass:: convertAgents(void)
// change any agents that are not associated with an actor to actors; used only if fConvAgents == TRUE
// Routine is structured as a set of tests for clarity; in fact this is just a big nested set of conditionals
{
    int aloc = 0;  // agent location
    toktype tagindex = 0; 
    
//    writeParsing("Parsing on entry to convertAgents"); // *** debug
//    Processor.fprob << "\ncA:entry actor = " << Actor << "  agent = " << Agent << endl; // *** debug
	while (aloc <= iLex) {
//        Processor.fprob << "cA: aloc " << aloc << endl; // *** debug
		if (Agent == syntArray[aloc].wtype) { 
            try {
                if ((aloc > 0) && ((Actor == syntArray[aloc-1].wtype) || (hasTailTag(aloc-1,Actor) || (Roots.rootFORMER == Parser.syntArray[aloc-1].iroot)))) { // actor prior to agent 
//                    Processor.fprob << "cA:Mk1 " << aloc << "  " << syntArray[aloc-1].wtype << "  " << syntArray[aloc].wtype << endl; // *** debug
                    hasHeadTag(aloc,Agent, tagindex); // set tagindex
                    while (!hasTailTag(aloc,Agent, tagindex)) {  // move to end of multiple word phrases
                        ++aloc;
                        if (aloc > iLex) throw CONVERT_AGENT_ERROR;
                    }
                }   
                else { // get the tail
                    int hloc = aloc;  // agent head location
                    bool fconvert = true; // do conversion?
//                    Processor.fprob << "cA:Mk2 " << aloc << "  " << syntArray[aloc-1].wtype << "  " << syntArray[aloc].wtype << endl; // *** debug
                    hasHeadTag(aloc,Agent, tagindex); // set tagindex
                    while (!hasTailTag(aloc,Agent, tagindex)) {  // move to end of multiple word phrases
                        ++aloc;
                        if (aloc > iLex) throw CONVERT_AGENT_ERROR;
                     }

                    if ((aloc < iLex) && (hasHeadTag(aloc+1,Actor))) fconvert = false; 	// actor follows agent
                    if ((fconvert) && (aloc+1 < iLex) && (Prep == syntArray[aloc+1].wtype) && (Actor == syntArray[aloc+2].wtype)) fconvert = false; // preposition followed by actor
                    if (fconvert) {   // satisfied all of the tests
                        syntArray[hloc].wtype = Actor;
                        changeHeadTag(hloc,Agent, Actor);
                        changeTailTag(aloc,Agent, Actor);
                    }
                }
            }
            catch (int ierror) {  // we're in trouble here but just report it and keep checking
                parseError("problem with agent to actor marking",CONVERT_AGENT_ERROR);
            }
        }
		++aloc;
	} // while aloc
    
} // convertAgents


void ParserClass:: markClauses(void)
//	find the compound clause boundaries.
//	Does not mark clauses when conjunction occurs in <noun><conj><adjctv>*<noun> or
//  <adjctv><conj><adjctv>
{
syntStruct * psyn = syntArray;
toktype clauseindex = 1;   

	fhasClauses = false;
	addHeadTag(psyn->index,Clause, clauseindex);
	while (psyn->wtype != Halt) {
		if (skipSegment(psyn,Compound)) continue;		// skip across compounds
		if (skipSegment(psyn,Subord)) continue;			// skip across subordinate clauses

		if ((Conj == psyn->wtype) &&				// check for clause delimited by a conjunction		
				(psyn > syntArray)    &&      	// don't mark if conjunction is first word in sentence
				((psyn+1)->wtype != Halt)) { 	 
			bool notPhrase = true;
			if ((psyn > syntArray) && (Noun == (psyn-1)->wtype)) { // check for compound noun
				syntStruct * psyna = psyn + 1;
				while ((Adjctv == psyna->wtype) && (psyna->wtype != Halt)) ++psyna;  // skip across adjectives  
			 	if (Noun == psyna->wtype) {
			 		notPhrase = false;
			 		psyn = psyna;
			 	}
			 }
			if ((psyn > syntArray) && (Adjctv == (psyn-1)->wtype)) { // check for compound adjective
			 	if (Adjctv == (psyn+1)->wtype) {
			 		notPhrase = false;
			 		psyn++;
			 	}
			}
			if (notPhrase) {				// mark clause
			 	fhasClauses = true;
				addSegTailTag((psyn-1)->index,Clause, clauseindex);
				++clauseindex;
				addSegHeadTag((psyn+1)->index,Clause, clauseindex);
				++psyn;				// increment -- this has already been labelled
			}																		

		}	
		++ psyn;
	} // while psyn
	addSegTailTag((psyn-1)->index,Clause, clauseindex);

} // markClauses

void ParserClass:: dereferencePronouns(void)
//  Dereference pronouns.  The following rules are used:
//  HE, SHE, IT	: Assign the first non-compound actor in the sentence if it comes prior 
//  HIM, HER      to pronoun;
//  THEM, THEIR : Look for a compound, an actor followed immediately by a plural, 
//  THEY          otherwise assign the first actor.

//  Deferenced pronouns are marked as follows:
//	1. wtype is changed to actor
//	2. a tag is added consisting of the index of the actor masked with setRefer

//	Note1: The Roots.lit* comparisons will only work if these are the first literals
//  ###   in the list.  Since they are initialized early, they currently are first,
//  ###   but it would be safer to replace these with some variant on the "hasLitr()" 
//				function that scans the entire list. [00.02.25]
//
//	Note 2. The sleazy shortcut of using litTHE to get the plural pronouns THEM and 
//          THEY exploits the fact that the first literal in the list is THE
{
syntStruct * psyn = syntArray;
syntStruct * psa;
int idx = -1;
toktype cmpdindex;

	while (psyn->wtype != Halt) {
		if (Pronoun == psyn->wtype) {
			if ((Roots.litHE <= literaList[psyn->ilit]) && 
					(literaList[psyn->ilit] <= Roots.litIT)) {
				psa = syntArray;			// find the first non-compound actor
				while (psa < psyn) {
					while ((psa < psyn) && (psa->wtype != Actor)) ++psa;
					if (psa >= psyn) break;
					if (hasHeadTag(psa->index,Compound,cmpdindex)) {
//						 Processor.fprob << "PC:dP Mk1 " << endl; // *** debug
						 do 	++psa;	// skip over the compound: note there is no bounds check here
						 while (!hasTailTag(psa->index,Compound,cmpdindex));
						 ++psa;
					}
					else if (!hasHeadTag(psa->index,Actor)) ++psa;  // this happens when head was inside a compound; don't match it
					else {
//						Processor.fprob << "PC:dP Mk2 " << endl; // *** debug
						idx = psa->index;
						break;
					}
				} // while psa < psyn
			if ((idx < 0) && (fcanForward)) {  // forward the reference if available
//				Processor.fprob << "PC:dP Mk3 " << idxForwActor << endl; // *** debug
				if (idxForwActor >= 0) idx = idxForwActor;
			}
		} 
			else if (Roots.litTHE == literaList[psyn->ilit]) { // see Note 2 above
				psa = syntArray;			// find the first actor
				while (psa < psyn) {
					while ((psa < psyn) && (psa->wtype != Actor)) ++psa;
					if (psa >= psyn) break;
					if (hasHeadTag(psa->index,Compound)) {
						 idx = psa->index;
						 break;  // got a compound, so stop
					}
					else if (idx < 0) idx = psa->index;  // only save the first actor
					++psa;							 // and keep looking
				} // while psa < psyn
				if ((idx < 0) && (fcanForward)) {  // forward the reference if available
					if (idxForwCompd >= 0) idx = idxForwCompd;
				}		
			}

			if (idx >= 0) {		// successful search, so dereference
				toktype index;  
				psyn->wtype = Actor;
//				Processor.fprob << "Changed type "<< psyn->index << " idx = " << idx << endl;
				addHeadTag(psyn->index,Actor, index);
				addTailTag(psyn->index,Actor, index);
				addHeadTag(psyn->index,(idx | setRefer));  
				addTailTag(psyn->index,(idx | setRefer));  // not actually used, but tags must be symmetric
			}
				
		}
		++ psyn;
		idx = -1;
	} // while psyn

} // dereferencePronouns

//___________________________________________________________________________________
//																																		complexity
void ParserClass:: checkCplxValue (int check, int value, const char *s)
// checks whether value >= check; if true, adds s to the sComplx string with
// the value of the overflow
{
char *ps = sComplex;

	if ((check <= 0) || (value < check)) return;
	fIsComplex = true;
	if (fCplxStrg) { 
		if (sComplex[0]) {
			strcat(sComplex, ", ");
			strcat(sComplex,s);
		}
		else {
			strcpy(sComplex,s);
			sComplex[0] = toupper(sComplex[0]);
		}
		
		while (*ps) ++ps;		// go to end of string
		*ps++ = '(';					// insert numerical values
		if (value > 9) *ps++  = (char)((value/10) + 48);
		*ps++  = (char)((value%10) + 48);
		*ps++ = ')';  // ### could this be replaced with a strcat? Or all of it with sprintf?
		*ps++  = ' ';
		*ps++  = '>';
		*ps++  = '=';
		*ps++  = ' ';
		if (check > 9) *ps++  = (char)((check/10) + 48);
		*ps++  = (char)((check%10) + 48);
		*ps = '\0';
	}
} // checkCplxValue

void ParserClass:: checkCplxBool (bool check, bool value, const char * s)
// checks whether value and check are true; if yes, adds s to the sComplx string
{
	if (!check || !value) return;
	fIsComplex = true;
	if (fCplxStrg) {
		if (sComplex[0]) {
			strcat(sComplex, ", ");
			strcat(sComplex,s);
		}
		else {
			strcpy(sComplex,s);
			sComplex[0] = toupper(sComplex[0]);
		}
	}
} // checkCplxBool

int ParserClass:: countType (wordtype thetype)
// returns the count of words of type wtype in syntArray, skipping over 
// subordinate clauses 
{
int	n = 0;
syntStruct * psyn = syntArray;

	while (psyn->wtype != Halt) {
		if (skipSegment(psyn,Subord)) continue;		// skip across subordinate clauses
		if (thetype == psyn->wtype) {
			++n;
//		  cout << "PC:cT " << n << "  " << psyn->index << endl;  // debug
		}
		++psyn;
	} // while psyn
	return n;
} // countType

int ParserClass:: countTags (wordtype thetype)
// returns the count of head tags of type wtype in syntArray, skipping over 
// subordinate clauses 
{
int	n = 0;
syntStruct * psyn = syntArray;

	while (psyn->wtype != Halt) {
		if (skipSegment(psyn,Subord)) continue;		// skip across subordinate clauses
		if (hasHeadTag(psyn->index,thetype)) ++n;
		++psyn;
	} // while psyn
	return n;
} // countTags

void ParserClass:: evaluateComplexity(void)
// evaluate cplxValue indicators for the complexity detector 
{
int	 	ka, iverb = 0;
bool	fokay;

	cplxValue.nVerb = countTags(Verb);		// always compute this; it is used below
	if (cplxLimit.nActor) cplxValue.nActor = countTags(Actor);
	if (cplxLimit.nPronoun) cplxValue.nPronoun = countType(Pronoun);

	if (cplxLimit.nConj) {    // count conjunctions outside of the syntactic compounds
		ka = -1;
		while (++ka <= iLex) { 
			if (hasHeadTag(ka,Compound)) {			// skip over compound
				while (!hasTailTag(ka,Compound)) ++ka;
				++ka;
			}
			if (Conj == syntArray[ka].wtype) ++iverb;
		}
		cplxValue.nConj = iverb;
	}

	if (!cplxValue.nVerb) { 	// signal absence of verb
		cplxValue.fPreAct   = false;
		cplxValue.fPostAct  = false;
		cplxValue.nLateVerb = 255;	// indicates no verb
	}
								// find position of first non-null verb 
	iverb = 0;
	while (iverb <= iLex) {
		if (skipSegment(iverb,Subord)) continue;		// skip across subordinate clauses
		if ((Verb == syntArray[iverb].wtype) &&
				hasHeadTag(iverb,Verb) &&
				!Roots.isNullVerb(syntArray[iverb].iroot)) break;
		++iverb;
	}

	cplxValue.nLateVerb = iverb;
	if (cplxLimit.fPreAct) { // check for actor before verb 
		fokay = true;
		ka = 0;
		while (ka < iverb) { 
			if (skipSegment(ka,Subord)) continue;		// skip across subordinate clauses
			if (Actor == syntArray[ka].wtype) {
				fokay = false;
				break;
			}
			++ka;
		}
		cplxValue.fPreAct = fokay;
	}

	if (cplxLimit.fPostAct) { // check for actor after verb
		fokay = true;
		ka = iverb + 1;
		while (ka <= iLex) { 
			if (skipSegment(ka,Subord)) continue;		// skip across subordinate clauses
			if (Actor == syntArray[ka].wtype) {
				fokay = false;
				break;
			}
			++ka;
		}
		cplxValue.fPostAct = fokay;
	}

	fIsComplex = false; 
	sComplex[0] = '\0';

	checkCplxValue(cplxLimit.nVerb, cplxValue.nVerb, "verbs");
	checkCplxValue(cplxLimit.nActor, cplxValue.nActor, "actors");
	checkCplxValue(cplxLimit.nPronoun, cplxValue.nPronoun, "pronouns");
	checkCplxValue(cplxLimit.nConj, cplxValue.nConj, "conjunctions");
	checkCplxValue(cplxLimit.nNonRes, cplxValue.nNonRes, "nonres clauses");
	if (cplxLimit.nLateVerb > 0) {
		if (cplxValue.nVerb < 255)	 checkCplxValue(cplxLimit.nLateVerb, cplxValue.nLateVerb, "verb position");
		else if (!cplxLimit.fNoVerb) checkCplxValue(cplxLimit.nLateVerb, 0, "verb missing");
	}
	checkCplxBool(cplxLimit.fPreAct, cplxValue.fPreAct, "no actor before verb");
	checkCplxBool(cplxLimit.fPostAct, cplxValue.fPostAct, "no actor after verb");
	if (cplxLimit.fNoVerb && (0 == cplxValue.nVerb)) checkCplxBool(true, true, "no verb");
	checkCplxBool(cplxLimit.fNoSrc, cplxValue.fNoSrc, "no source");
	checkCplxBool(cplxLimit.fNoTar, cplxValue.fNoTar, "no target");
	checkCplxBool(cplxLimit.fNoEvt, cplxValue.fNoEvt, "no event");

} // evaluateComplexity

void ParserClass:: checkDiscards(void)
// check for primary discard codes -- codes directly associated with a phrase
// CoderClass:: evaluateEvents() later checks for codes in a pattern  
// ### it would be nice to return information on what was a discard...
{
syntStruct * psyn = syntArray;

	fHasDiscard = false;
	while (psyn->wtype != Halt) {
		if (Roots.isDiscardCode(psyn->iroot)) {
			fHasDiscard = true;
			return;
		}
		++psyn;
	} // while psyn
} // checkDiscards

//___________________________________________________________________________________
//																																		display routines

bool ParserClass:: checkDisplay(bool check)
// scroll parse display
{
static int 		startDisplay;	// first line of display
static int		curline;

	if (!check) {  // just initialize
		ResetCurs();
		startDisplay = TabariFlags.cursr;
		curline = startDisplay;
		return false;
	} 
	else { // see if we're at the end of screen
		WRITEEOL();	
		++curline;
		if (curline > WINDOW_HEIGHT - 2) {
			if (GetAnswer((char *)"Continue display",(char *)"")) { // erase and continue
				move(startDisplay,0);
				clrtobot(); 
				curline = startDisplay;
				return(false);
			}
			else return true; // quit
		}
		else return false;
	}
} // checkDisplay


void ParserClass:: showParsing(void)
// tabular display of syntactic markup on screen
{
syntStruct * psyn = syntArray;
litstring s;
char *	pc;
int 		idxlit;
toktype * ptok;
int 	kc;
char 		stab[] = "                   ";  // tab for the segment tags
bool 		finSubord = false;			// inside a subordinate clause?
toktype itok;
char sout[4];

	WRITEEOL();
	checkDisplay(false);

	if (TabariFlags.cursr > WINDOW_HEIGHT - 8) {
		WriteAlert("Insufficient room to show parsing;\nuse a browser to display the TABARI.Parse.html file");
		return;
	}

	if (iparseError) {
		WriteLine("Parsing was not completed; no display is available");
		return;
	}

	
	while (psyn->wtype != Halt) {
		if (hasHeadTag(psyn->index,Subord)) {
			 	WRITESTRING(stab);
			 	WRITESTRING("<subord>");
				if (checkDisplay(true)) return;
			 	finSubord = true;
		}
		else {
			if (hasHeadTag(psyn->index,Clause)) 	{
			 	WRITESTRING(stab);
				WRITESTRING("<clause>");
				if (checkDisplay(true)) return;
			}
			if (hasHeadTag(psyn->index,Compound)) {
			 	WRITESTRING(stab);
				WRITESTRING("<compnd>");
				if (checkDisplay(true)) return;
			}
		}
		sprintf(sout,"%2d  ",psyn->index);
		WRITESTRING(sout);
		pc = lexArray[psyn->index].ptext;		// write original text
		kc = 0;
		do {
			WRITECHAR(*pc);
			++kc;
		} while (' ' != *pc++);
//		Pause();
		while (kc++ < 16) WRITECHAR(' ');		// blank fill -- '16' is a tabbed field position; note that ncurses could do this directly

		if (!finSubord) {
			idxlit = psyn->ilit;
			if (idxlit) {
			 	if (psyn->iroot) {
					if (hasHeadTag(psyn->index, Pronoun) && (Actor == psyn->wtype)) {  // handle a dereferenced pronoun
						itok = getReference(psyn->index);
						WRITESTRING("<prnoun> ");
						WRITESTRING(Phrases.get(s, Roots.rootArray[psyn->iroot].phrase));
						WRITESTRING(" <refernc = "); 
						if (itok > iSynt) WRITESTRING("forwarded>\n");
						else {
							sprintf(sout,"%2d >\n",itok);
							WRITESTRING(sout);
						}
					}
					else {
						WRITECHAR('<');
						WRITESTRING(tagText[psyn->wtype]);
						WRITESTRING("> ");
						for (kc = 0; kc<tagTab[psyn->wtype]; ++kc) WRITECHAR(' ');
						WRITESTRING(Phrases.get(s, Roots.rootArray[psyn->iroot].phrase));
						if (checkDisplay(true)) return;
					}	
				}
				else if (idxlit < 0) {
					idxlit = -idxlit;				// reverse sign since this is a symbol
					WRITECHAR('<');
					WRITESTRING(tagText[psyn->wtype]);
					WRITESTRING("> ");
					if (COMMA_LIT == idxlit) WRITESTRING(" , ");
					else if (NUMBER_LIT == idxlit) {
						pc = lexArray[psyn->index].ptext;
						do WRITECHAR(*pc); 
						while (' ' != *pc++);
					}
				if (checkDisplay(true)) return;
				}
				else {				// show the literals list
					WRITESTRING("<litrl>  ");
					ptok = &literaList[idxlit];
					while (*ptok) {
						WRITESTRING(Literals.litArray[*ptok].pchar);
						++ptok;
						if (*ptok) WRITESTRING("_");
						++ptok;
						if (*ptok) WRITESTRING(" | ");
						else if (checkDisplay(true)) return;
					}
				}
			}
			else 	if (checkDisplay(true)) return;
	#if FALSE
			else {
				pc = lexArray[psyn->index].ptext;
				do WRITECHAR(*pc); while (' ' != *pc++);
				if (checkDisplay(true)) return;
			}
	#endif
		}  // if !finSubord
		else if (checkDisplay(true)) return;
		
		if (hasTailTag(psyn->index,Subord)) {
			WRITESTRING(stab);
			WRITESTRING("</subord>");
			if (checkDisplay(true)) return;
			finSubord = false;
		}
			if (hasTailTag(psyn->index,Compound)) {
			WRITESTRING(stab);
			WRITESTRING("</compnd>");
			if (checkDisplay(true)) return;
		}
		if (hasTailTag(psyn->index,Clause)) {
			WRITESTRING(stab);
			WRITESTRING("</clause>");
			if (checkDisplay(true)) return;
		}
		++psyn;
	} // while 
	ResetCurs();
//WRITEEOL();
//	Pause();  // *** debug
} // showParsing

void ParserClass:: htmlParsing(void)
// writes parse information in html format
{
ofstream fhtml;
syntStruct * psyn = syntArray;
litstring s;
char *	pc = Processor.sentText;;
int 		idxlit;
toktype * ptok;
int 		kc;
char 		stab[] = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";  // tab for the segment tags
bool 		finSubord = false;			// inside a subordinate clause?
toktype itok;
char		sColor[8];


	fhtml.open("TABARI.Parse.html",ios::out); 
	if (fhtml.fail()) ShowFatalError("Unable to open the TABARI.Parse.html file ",shError03);
	fhtml << "<head>" << endl;
	fhtml << "\t<title>TABARI Parsing</title>" << endl;
	fhtml << "\t<meta name=\"generator\" content=\"TABARI " << sRelease << "\" />" << endl;
	fhtml << "</head>" << endl;
	fhtml << "<body>" << endl<< endl;

	if (iparseError) {
		fhtml << "Parsing was not completed; no display is available\n";
		fhtml << "</body>\n</html>" << endl;
		fhtml.close();		
		return;
	}
	
// Show original text

	fhtml << endl << "<h3>Original text</h3>" << endl;
	fhtml << "Date: " << Processor.sRecDate;
	fhtml << stab << "Record : " << Processor.sRecID << "<br>" << endl;
	
	while (*pc) {
		if (('\\' == *pc) && ('n' == *(pc+1))) {  // deal with \n
			fhtml << " <br>" << endl;
			pc += 2;
		}
		else fhtml << *pc++; 
	}
	fhtml << "<p><hr>" << endl;

// Textual parse

	fhtml << endl << "<h3>Parsed text</h3>" << endl;
	while (psyn->wtype != Halt) {
		if (hasHeadTag(psyn->index,Subord)) {
			 	fhtml << "<strike>"<< endl;
			 	finSubord = true;
		}
		else {
			if (hasHeadTag(psyn->index,Clause)) 	fhtml << "<font color = OLIVE size = +2>{</font>"<< endl;
			if (hasHeadTag(psyn->index,Compound)) fhtml << "<font color = OLIVE size = +2>&lt;</font>"<< endl;
		}
		strcpy(sColor,"GRAY");
		if (!finSubord) {
			if (psyn->ilit) {
			 	if (psyn->iroot) {
					if (hasHeadTag(psyn->index, Pronoun) && (Actor == psyn->wtype)) {  // handle a dereferenced pronoun
				// ### still need to include pronoun reference? -- KEDS did this...
					  strcpy(sColor,tagColor[Pronoun]);
			  	}
					else strcpy(sColor,tagColor[psyn->wtype]);
				}
				else strcpy(sColor,"BLACK");
			}
		}	
		fhtml << "<font color = " << sColor << ">";

		pc = lexArray[psyn->index].ptext;		// write original text
		kc = 0;
		do {
			fhtml << *pc;
			++kc;
		} while (' ' != *pc++);
		fhtml << "</font>";
		
		if (hasTailTag(psyn->index,Subord)) {
			fhtml << "</strike>"<< endl;
			finSubord = false;
		}
		if (hasTailTag(psyn->index,Clause)) fhtml << "<font color = OLIVE size = +2>}</font>"<< endl;
		if (hasTailTag(psyn->index,Compound)) fhtml <<"<font color = OLIVE size = +2>&gt;</font>" << endl;
		++psyn;
	} // while 
	fhtml << "<p><hr>" << endl;

// Tabular parse
// ### This could be done more neatly and appropriately as a table, but this looks okay for the time being

	fhtml << endl << "<h3>Parse table</h3><tt>" << endl;
	psyn = syntArray;  // re-start at the beginning
	while (psyn->wtype != Halt) {
		if (hasHeadTag(psyn->index,Subord)) {
			 	fhtml << stab << "&lt;subord&gt;" << "<br>" << endl;
			 	finSubord = true;
		}
		else {
			if (hasHeadTag(psyn->index,Clause)) 	fhtml << stab << "&lt;clause&gt;" << "<br>" << endl;
			if (hasHeadTag(psyn->index,Compound)) fhtml << stab << "&lt;compnd&gt;" << "<br>" << endl;
		}
		
		strcpy(sColor,"GRAY");  // default for unclassified and subord
		if (!finSubord) {
			if (psyn->ilit) {
				if (psyn->iroot) {
					if (hasHeadTag(psyn->index, Pronoun) && (Actor == psyn->wtype)) {  // handle a dereferenced pronoun
						strcpy(sColor,tagColor[Pronoun]);
					}
					else strcpy(sColor,tagColor[psyn->wtype]);
				}
				else strcpy(sColor,"BLACK");
			}
		}
		fhtml << "<font color = " << sColor << ">";
		
		if (psyn->index > 9) fhtml << psyn->index << "&nbsp;"; // "> 9 " is just checking for a single digit 
		else fhtml << "&nbsp;" << psyn->index << "&nbsp;";
		pc = lexArray[psyn->index].ptext;		// write original text
		kc = 0;
		do {
			fhtml << *pc;
			++kc;
		} while (' ' != *pc++);
		while (kc++ < 16) fhtml << "&nbsp;";		// blank fill -- '16' is a tabbed field position

		if (!finSubord) {
			idxlit = psyn->ilit;
			if (idxlit) {
			 	if (psyn->iroot) {
					if (hasHeadTag(psyn->index, Pronoun) && (Actor == psyn->wtype)) {  // handle a dereferenced pronoun
						itok = getReference(psyn->index);
						fhtml << "&lt;prnoun&gt; ";
						fhtml << Phrases.get(s, Roots.rootArray[psyn->iroot].phrase);
						fhtml << " &lt;refernc = "; 
						if (itok > iSynt) fhtml << "forwarded&gt;";
						else fhtml << itok << " &gt;";
					}
					else {
						fhtml << "&lt;" << tagText[psyn->wtype] << "&gt;&nbsp;";
						for (kc = 0; kc<tagTab[psyn->wtype]; ++kc) fhtml << "&nbsp;";
						fhtml << Phrases.get(s, Roots.rootArray[psyn->iroot].phrase) ;
					}	
				}
				else if (idxlit < 0) {
					idxlit = -idxlit;				// reverse sign since this is a symbol
					fhtml << "&lt;" << tagText[psyn->wtype] << "&gt;&nbsp;";
					if (COMMA_LIT == idxlit) fhtml << " , ";
					else if (NUMBER_LIT == idxlit) {
						pc = lexArray[psyn->index].ptext;
						do fhtml << *pc; 
						while (' ' != *pc++);
					}
					fhtml << "<br>" << endl;		
				}
				else {				// show the literals list
					fhtml << "&lt;litrl&gt;&nbsp;&nbsp;";
					ptok = &literaList[idxlit];
					while (*ptok) {
						fhtml << Literals.litArray[*ptok].pchar;
						++ptok;
						if (*ptok) fhtml << "_";
						++ptok;
					if (*ptok) fhtml << " | ";
					}
				}
			}
	#if FALSE
			else {
				pc = lexArray[psyn->index].ptext;
				do fhtml << *pc; while (' ' != *pc++);
			}
	#endif
		}  // if !finSubord
		fhtml << "</font><br>" << endl;
		
		if (hasTailTag(psyn->index,Subord)) {
			fhtml << stab << "&lt;/subord&gt;" << "<br>" << endl;
			finSubord = false;
		}
		if (hasTailTag(psyn->index,Compound)) fhtml << stab << "&lt;/compnd&gt;" << "<br>" << endl;
		if (hasTailTag(psyn->index,Clause)) 	fhtml << stab << "&lt;/clause&gt;" << "<br>" << endl;
		++psyn;
	} // while 
	fhtml << "</tt><p><hr>" << endl;

	
#if TRUE
// as of 07.06.01, this is broken...doubtlessly related to the fact that tags in
// in are broken...
// tag listing
	fhtml << endl << "<h3>Tag listing</h3><tt>" << endl;

	psyn = syntArray;  // re-start at the beginning
	while (psyn->wtype != Halt) {
		if (psyn->index > 9) fhtml << psyn->index << "&nbsp;"; // "> 9 " is just checking for a single digit 
		else fhtml << psyn->index << "&nbsp;&nbsp;";
		pc = lexArray[psyn->index].ptext;		// write original text
		kc = 0;
		do {
			fhtml << *pc;
			++kc;
		} while (' ' != *pc++);
		while (kc++ < 16) fhtml << "&nbsp;";		// blank fill -- '16' is a tabbed field position

		if (psyn->ilit) {
			fhtml << "&lt;" << tagText[psyn->wtype] << "&gt;&nbsp;";
			for (kc = 0; kc<tagTab[psyn->wtype]; ++kc) fhtml << "&nbsp;";
		}
		else 	for (kc = 0; kc<9; ++kc) fhtml << "&nbsp;";

		if (psyn->pheadtag) {  // list head tags
			fhtml << "head: ";
	    idxlit = psyn->pheadtag;
			while (idxlit) { 
//				fhtml << idx << "-[" << tagText[tagArray[idx]];
				if (tagArray[idxlit]>MAX_TAG_TYPES) fhtml << idxlit << "-[deref " << getReference(psyn->index); // handle dereferenced cases
				else {
					fhtml << '[' << tagText[tagArray[idxlit]];
					if (tagArray[idxlit+1]) fhtml << "&nbsp;" << tagArray[idxlit+1];
				}
				fhtml << "] ";
				idxlit = tagArray[idxlit+2];
			}
		}
		if (psyn->ptailtag) {  // list tail tags
			if (psyn->pheadtag) { // complete head tag line if written
			  fhtml << "<br>" << endl;
				kc = 0;
				while (kc++ < 28) fhtml << "&nbsp;";		// tab: blank fill to 28
			}
			fhtml << "tail: ";
	    idxlit = psyn->ptailtag;
			while (idxlit) { 
//				fhtml << idx << "-[" << tagText[tagArray[idx]];
				if (tagArray[idxlit]>MAX_TAG_TYPES) fhtml << idxlit << "-[deref " << getReference(psyn->index); // handle dereferenced cases
				else {
					fhtml << '[' << tagText[tagArray[idxlit]];
					if (tagArray[idxlit+1]) fhtml << ' ' << tagArray[idxlit+1];
				}
				fhtml << "] ";
				idxlit = tagArray[idxlit+2];
			}
		}
		fhtml << "<br>" << endl;				
		++psyn;
	} // while 
	fhtml << endl;

	fhtml << "</tt>" << endl;
#endif
	fhtml << "</body>" << endl;
	fhtml << "</body>" << endl;
	fhtml << "</html>" << endl;
	fhtml.close();
} // htmlParsing

void ParserClass:: writeParsing(const char *sinfo)
// writes parsing information and tag to Processor.fprob; *sinfo is output on first line
{
syntStruct * psyn = syntArray;
litstring s;
char *pc;
int 		idxlit;
toktype * ptok;
int kc;
char stab[] = "                   ";  // tab for the segment tags
bool finSubord = false;			// inside a subordinate clause?

	Processor.fprob  << "\n>>> Parsing information: " << sinfo << " <<<" << endl;
	if (iparseError) {
		Processor.fprob  << "\nParsing was not completed; no display is available\n";
		return;
	}
	
	Processor.fprob << "\n--- Parsed Text ---\n";	

	while (psyn->wtype != Halt) {
		if (hasHeadTag(psyn->index,Subord)) {
			 	Processor.fprob << stab << "<subord>" << endl;
			 	finSubord = true;
		}
		else {
			if (hasHeadTag(psyn->index,Clause)) 	Processor.fprob << stab << "<clause>" << endl;
			if (hasHeadTag(psyn->index,Compound)) Processor.fprob << stab << "<compnd>" << endl;
		}
		if (psyn->index > 9) Processor.fprob << psyn->index << " "; // "> 9 " is just checking for a single digit 
		else Processor.fprob << psyn->index << "  ";
		pc = lexArray[psyn->index].ptext;		// write original text
		kc = 0;
		do {
			Processor.fprob << *pc;
			++kc;
		} 
		while (' ' != *pc++);
		while (kc++ < 16) Processor.fprob << ' ';		// blank fill -- '16' is a tabbed field position

		if (!finSubord) {
			idxlit = psyn->ilit;
			if (idxlit) {
			 	if (psyn->iroot) {
					if (hasHeadTag(psyn->index, Pronoun) && (Actor == psyn->wtype)) {  // handle a dereferenced pronoun
						Processor.fprob << "<refernc = " << getReference(psyn->index) << ">" << endl;
						Processor.fprob << "<prnoun> ";
						Processor.fprob << Phrases.get(s, Roots.rootArray[psyn->iroot].phrase);
						Processor.fprob << endl;		
					}
					else {
						Processor.fprob << "<" << tagText[psyn->wtype] << "> ";
						for (kc = 0; kc<tagTab[psyn->wtype]; ++kc) Processor.fprob << ' ';
						Processor.fprob << Phrases.get(s, Roots.rootArray[psyn->iroot].phrase) << endl;
					}	
				}
				else if (idxlit < 0) {
					idxlit = -idxlit;				// reverse sign since this is a symbol
					Processor.fprob << "<" << tagText[psyn->wtype] << "> ";
					if (COMMA_LIT == idxlit) Processor.fprob << " , ";
					else if (NUMBER_LIT == idxlit) {
						pc = lexArray[psyn->index].ptext;
						do Processor.fprob << *pc; 
						while (' ' != *pc++);
					}
					Processor.fprob << endl;		
				}
				else {				// show the literals list
					Processor.fprob << "<litrl>  ";
					ptok = &literaList[idxlit];
					while (*ptok) {
						Processor.fprob << Literals.litArray[*ptok].pchar;
						++ptok;
						if (*ptok) Processor.fprob << "_";
						++ptok;
						if (*ptok) Processor.fprob << " | ";
						else Processor.fprob << endl;
					}
				}
			}
			else Processor.fprob << endl;
	#if FALSE
			else {
				pc = lexArray[psyn->index].ptext;
				do Processor.fprob << *pc; while (' ' != *pc++);
				Processor.fprob << endl;
			}
	#endif
		}  // if !finSubord
		else Processor.fprob << endl;
		
		if (hasTailTag(psyn->index,Subord)) {
			Processor.fprob << stab << "</subord>" << endl;
			finSubord = false;
		}
		if (hasTailTag(psyn->index,Compound)) Processor.fprob << stab << "</compnd>" << endl;
		if (hasTailTag(psyn->index,Clause)) 	Processor.fprob << stab << "</clause>" << endl;
		++psyn;
	} // while 

	Processor.fprob << "\n--- Tag listing ---\n";
	
	psyn = syntArray;
	while (psyn->wtype != Halt) {
	if (psyn->index > 9) Processor.fprob << psyn->index << ' '; // "> 9 " is just checking for a single digit 
	else Processor.fprob << psyn->index << "  ";
	pc = lexArray[psyn->index].ptext;		// write original text
	kc = 0;
	do {
		Processor.fprob << *pc;
		++kc;
	} while (' ' != *pc++);
	while (kc++ < 16) Processor.fprob << ' ';		// blank fill -- '16' is a tabbed field position

	if (psyn->ilit) {
		Processor.fprob << "<" << tagText[psyn->wtype] << "> ";
		for (kc = 0; kc<tagTab[psyn->wtype]; ++kc) Processor.fprob << ' ';
	}
	else for (kc = 0; kc<9; ++kc) Processor.fprob << ' ';

	if (psyn->pheadtag) {  // list head tags
		Processor.fprob << "head: ";
    idxlit = psyn->pheadtag;
		while (idxlit) { 
			if (tagArray[idxlit]>MAX_TAG_TYPES) Processor.fprob << idxlit << "-[deref " << getReference(psyn->index); // handle dereferenced cases
			else {
				Processor.fprob << idxlit << "-[" << tagText[tagArray[idxlit]];
				if (tagArray[idxlit+1]) Processor.fprob << ' ' << tagArray[idxlit+1];
			}
			Processor.fprob << "] ";
			idxlit = tagArray[idxlit+2];
		}
	}
	if (psyn->ptailtag) {  // list tail tags
		if (psyn->pheadtag) { // complete head tag line if written
		  Processor.fprob << endl;
			kc = 0;
			while (kc++ < 28) Processor.fprob << ' ';		// tab: blank fill to 28
		}
		Processor.fprob << "tail: ";
    idxlit = psyn->ptailtag;
		while (idxlit) { 
			if (tagArray[idxlit]>MAX_TAG_TYPES) Processor.fprob << idxlit << "-[deref " << getReference(psyn->index); // handle dereferenced cases
			else {
				Processor.fprob << idxlit << "-[" << tagText[tagArray[idxlit]];
				if (tagArray[idxlit+1]) Processor.fprob << ' ' << tagArray[idxlit+1];
			} 
			Processor.fprob << "] ";
			idxlit = tagArray[idxlit+2];
		}
	}
	Processor.fprob << endl;				
	++psyn;
	} // while 
	Processor.fprob << endl;

} // writeParsing

void ParserClass:: showTags(void)
// displays tag lists for syntArray: this option is invisible to the
// user but can be invoked from the O)ther menu
// ### <08.06.24> this doesn't handle multi-screen displays 
{
syntStruct * psyn = syntArray;
int 		idx;
int 		kc;
char *  pc;
char 		sout[4];

	WRITESTRING("\n--- Tag listing ---\n");

	if (iparseError) {
		WRITESTRING("Parsing was not completed; no display is available\n");
		return;
	}
	
	while (psyn->wtype != Halt) {
		sprintf(sout,"%2d ",psyn->index);
		WRITESTRING(sout);
		pc = lexArray[psyn->index].ptext;		// write original text
		kc = 0;
		do {
			WRITECHAR(*pc);
			++kc;
		} while (' ' != *pc++);
		while (kc++ < 16) WRITECHAR(' ');		// blank fill -- '16' is a tabbed field position

		if (psyn->ilit) {
			WRITECHAR('<');
			WRITESTRING(tagText[psyn->wtype]);
			WRITESTRING("> ");
			for (kc = 0; kc<tagTab[psyn->wtype]; ++kc) WRITECHAR(' ');
		}
		else 	for (kc = 0; kc<9; ++kc) WRITECHAR(' ');

		if (psyn->pheadtag) {  // list head tags
			WRITESTRING("head: ");
	    idx = psyn->pheadtag;
			while (idx) { 
//				cout << idx << "-[" << tagText[tagArray[idx]];
				if (tagArray[idx] > MAX_TAG_TYPES) { // handle dereferenced cases
					WRITESTRING("[deref ");
					sprintf(sout," %2d",getReference(psyn->index));
					WRITESTRING(sout);
				}
				else {
					WRITECHAR('[');
					WRITESTRING(tagText[tagArray[idx]]);
					if (tagArray[idx+1]) {
						sprintf(sout," %2d",tagArray[idx+1]);
						WRITESTRING(sout);
					}
				}
				WRITESTRING("] ");
				idx = tagArray[idx+2];
			}
		}
		if (psyn->ptailtag) {  // list tail tags
			if (psyn->pheadtag) { // complete head tag line if written
			  WRITEEOL();
				kc = 0;
				while (kc++ < 28) WRITECHAR(' ');		// tab: blank fill to 28
			}
			WRITESTRING("tail: ");
	    idx = psyn->ptailtag;
			while (idx) { 
//				cout << idx << "-[" << tagText[tagArray[idx]];
				if (tagArray[idx] > MAX_TAG_TYPES) { // handle dereferenced cases
						WRITESTRING("[deref ");
						sprintf(sout," %2d",getReference(psyn->index));
						WRITESTRING(sout);
				}
				else {
					WRITECHAR('[');
					WRITESTRING(tagText[tagArray[idx]]);
					if (tagArray[idx+1]) {
						sprintf(sout," %2d",tagArray[idx+1]);
						WRITESTRING(sout);
					}
				}
				WRITESTRING("] ");
				idx = tagArray[idx+2];
			}
		}
		WRITEEOL();
		++psyn;
	} // while 
	WRITEEOL();
	ResetCurs();
} // showTags

//___________________________________________________________________________________
//																																		core routines

void ParserClass:: doParsing(void)
// calls the various parsing routines in the correct order
// the various "writeParsing()" calls give a nice trace for debugging purposes
{
//	Literals.alphaList(); // *** debug
//	Pause();  						// *** debug

	iparseError = PARSE_OK;

	if (!(*Processor.sentText)) {  // this forces a reasonable bailout when there is no text
		iparseError = NO_TEXT_ERROR;
		return;
	}
	
	try{
		filterText();
		makeLex();
		makeSynt();
//        Processor.fprob << Parser.filtext << endl; // *** debug
        //  	Parser.writeParsing("Called from doParsing 1 ");  // *** debug
//		if (TabariFlags.fFBISMode) makeFBISCompound();  // currently doesn't work
		markNouns();
		if (TabariFlags.fConvAgent) convertAgents();
		markCompound();
		markSubordinate();
		if (!TabariFlags.fhasAttribList) nullSubordinate(); 
		markClauses();
		dereferencePronouns();	
//		showParsing(); Pause();                           // *** debug
		checkDiscards();
		if (!fHasDiscard) evaluateComplexity();
	}
	catch (int ierror) {
		iparseError = ierror;
	}

} // doParse

//___________________________________________________________________________________
// 																																		debugging code

#if TRUE
void ParserClass:: testCplx(void)
// *** debugging : tests to check/boolCplx functions
{	
	sComplex[0] = '\0';
	fCplxStrg = true;
	checkCplxBool(true,true,"testing 1");
	checkCplxBool(true,true,"testing 2");
	sComplex[0] = '\0';
	checkCplxValue(3,4,"testing 1");
	checkCplxValue(3,4,"testing 2");
	sComplex[0] = '\0';
	checkCplxValue(23,34,"testing 3");
	checkCplxValue(43,54,"testing 4");
		
} // testCplx
#endif

#if FALSE
void ParserClass:: showLitr(int idx)
// *** debugging : shows the contents of literaList list starting at idx
{	
	while (literaList[idx]) {
//		cout << idx << "  '" << Literals.litArray[literaList[idx]].pchar << "'\n";
		++idx;
	}
} // showLitr

int ParserClass:: getLitr(void)
// *** debugging : returns current value of iLitr
{ return iLitr; }

void ParserClass:: initLitr(void)
// *** debugging : sets iLitr = 0
{ iLitr = 0; }

#endif
