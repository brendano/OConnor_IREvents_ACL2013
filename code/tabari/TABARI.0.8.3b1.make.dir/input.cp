// 																																		TABARI Project
//___________________________________________________________________________________
// 																																	 			input.cp

//  This file contains the routines for handling file input
 
//__________________________________________________________________________________
//
//	 Copyright (c) 2000 - 2009  Philip A. Schrodt.  All rights reserved.
//
// 	Redistribution and use in source and binary forms, with or without modification,
// 	are permitted under the terms of the GNU General Public License:
// 	http://www.opensource.org/licenses/gpl-license.html
//
//	Report bugs to: schrodt@ku.edu

//	The most recent version of this code is available from the KEDS Web site:
//		http://www.ku.edu/~keds

//	For plausible indenting of this source code, set the tab size in your editor to "2"

//___________________________________________________________________________________
// 																																						Headers

#include "TABARI.h"

//___________________________________________________________________________________
// 																																   external globals

extern TabariFlagsClass TabariFlags;
extern CodeStoreClass CodeStore;
extern CharStoreClass CharStore;
extern PhrasesClass   Phrases;
extern RootsClass 		Roots;
extern LiteralsClass 	Literals;  // *** debug only
extern ParserClass		Parser;		
extern CoderClass			Coder;		
extern ProcessorClass  Processor;

//___________________________________________________________________________________
// 																																   internal globals

ReadFilesClass 		ReadFiles;
CommentFileClass	CommentFile;		

//___________________________________________________________________________________
// 																														primary input functions

void ReadFilesClass:: doPhraseError(const char *sreason)
// announces error; decides whether to quit; sets the phrase variables to
// defaults if continuing
{
	fPhraseOK = false;
	if (PhraseError(sLine, (char *)sreason)) {
//  WriteLine("RFC:dPE: Resetting sPhrase to ***\n",sLine); // *** debug		
		strcpy(sPhrase,sBlankCode);
		strcpy(sCode,sCodeNull);
		sComment[0] = '\0';
	}
	else exit(0);
} // doPhraseError

void ReadFilesClass:: doDupError(const char *stype)
// announces duplicate error with line information decides whether to quit;
// defaults if continuing
{
    if (TabariFlags.fWarnDups) {
        char sout[16];
        sprintf(sout,"%d",TabariFlags.nlinesread);
        strcat(sPhrase,"\" ");
        strcat(sPhrase,stype);
        strcat(sPhrase," has already been entered in the dictionary in line ");
        strcat(sPhrase,sout);
        ShowWarningError("\"",sPhrase,"New entry has been skipped",shError00);
    }
}
// doDupError

void ReadFilesClass:: readActors(void)
// input routine for .actors file
{
	openFile(Processor.nextDict->fileName);
	fcodeOptional = true;  // this prevents parseline from indicating error due to no code
	while (readLine()) {
		if (fPhraseOK) {
			if (strstr(sPhrase,"<NOUN")) {
				Processor.nextDict->hasNounList = true;
				readWord("</NOUN>",Noun);
				continue;
			}
			if (strstr(sPhrase,"<ADJ")) {
				Processor.nextDict->hasAdjctList = true;
				readWord("</ADJ",Adjctv);
				continue;
			}
            try {Roots.storeActor();}
            catch (int i) {
                switch (i) {
                    case DATES_LONG:  {
                        instring s;
                        instring sphr;
                        strcpy(s, "Total length of date restrictions is too long in ");
                        Phrases.get(sphr, Roots.rootArray[Roots.iRoot].phrase);
                        strcat(s,sphr);
                        doPhraseError(s);
                        break;
                    }
                    case EMPTY_CODE:  {
                        if (Roots.needcodes) doPhraseError("Missing code in date restrictions for the entry BEFORE the phrase above");
                        else doPhraseError(sErrEMPTY_CODE);
                        break;
                    }
                    case DATE_INVLD: 
                        doPhraseError("Date contains illegal characters ");
                        break;
                    case YEAR_OOR: 
                        doPhraseError("Year is out of range in date ");
                        break;
                    case MONTH_OOR: 
                        doPhraseError("Month is out of range in date ");
                        break;
                    case DAY_OOR: 
                        doPhraseError("Day is out of range in date ");
                        break;
                    case DUP_ROOT: 
                        doDupError("<ACTOR>");
                        break;
                    default: doPhraseError(sErrEMPTY_CODE);
                } // switch
                Roots.iRoot--;     // cancel the entry
                Roots.sdaterest[0] = '\0';
            } // catch
		} // if fPhraseOK
	} // while

	if (Roots.needcodes) {
		strcpy(sPhrase,sDiscardCode); // signals to only process sdaterest
		Roots.storeActor();
	}
	storeTail();  // store comments from end of file
	fcodeOptional = false; 
} // readActors

void ReadFilesClass:: readAgents(char *sfilename)
// input routine for .agents file
{
	openFile(sfilename);
	while (readLine()) {
		if (fPhraseOK) {
			if (!strchr(sCode,'~')) {
				strcat(sLine," in the .agent file requires a \'~\' in the code.");
				ShowWarningError("\"",sLine,"New entry has been skipped",shError00);
			}
			else if (Literals.duplicateRoot(sPhrase)) doDupError("<AGENT>");
			else {
				try {Roots.storeAgent();}
				catch (int i) { doPhraseError(sErrEMPTY_CODE);}
			}
		}
	}
	storeTail();
} // readAgents

void ReadFilesClass:: readVerbs(char *sfilename)
// input routine for .verbs file
{
toktype idxroot;  // rootArray index of the current verb
instring sverb; // current verb being processed
bool gotVerb = false; // checks against possibility of verbless pattern (it's happened...)
bool fskip   = false; // skips verbforms and patterns if main verb is a duplicate
	
	openFile(sfilename);
	while (readLine()) {
//		Processor.fprob << sPhrase << "  " << sCode << endl; // *** debug
		try {
			if (fPhraseOK) {
	//			cout << sPhrase << endl;
				if (strstr(sPhrase,"<TIME>")) {
					readTime();
					continue;
				}
				if (strstr(sPhrase,"<ATTRIB")) {
					readAttrib();
					continue;
				}
                if (strstr(sPhrase,"<SYN")) {
                    readSynsets();
                    continue;
                }
				if (sPhrase[0] != '-') { // not pattern
					if (Literals.duplicateRoot(sPhrase)) {  // if false, moves down to the single root processing
						throw DUP_VERB;
					}
					else {	
						if (('{' == sPhrase[1]) && !fskip) {  // parseLine adds a leading blank to '{', so a first-col '{' is now char [1] 
							// process multiple verb forms						
							instring scopy;
							char * pword;
							strcpy(scopy,sPhrase); // working copy of sPhrase to be parsed
							pword = strtok(scopy+2," }");
							if (NULL == pword) throw NO_VFORM;
							while (pword != NULL) {   // process all of the forms
								strcpy(sPhrase, pword);
								if (Literals.duplicateRoot(sPhrase)) throw DUP_VFORM;
								Roots.storeVerbForm(idxroot);
								pword = strtok (NULL, " }");
							}
						}							
						else {   // store main verb 
							idxroot = Roots.storeVerb();
	//					Processor.fprob << "RFC:rV Mk2 " << sPhrase << " :: " << sCode << endl; // *** debug
							if (fPhraseOK) {   // ### can storeVerb detect an error?
		//					  if (sPhrase[0] == '*') { WriteLine("RFC:rV: resetting sverb to ***\n",sLine); WriteNewLine(sPhrase);} // ### debug
								strcpy(sverb,sPhrase);
								gotVerb = true;
								fskip = false;
							}  // fPhraseOK
						}  // main verb process
					}    // not dup
				}      // not pattern
				
				else { // process pattern
					if (!strchr(sPhrase,'*')) throw MISS_VERBM;
					else if (!gotVerb) throw PAT_ORPHAN; 
					else if (!fskip) Roots.storePattern(idxroot,Verb);
				}
			} // fPhraseOK
		} // try
		catch (int i) {
			if (DUP_VFORM == i) doDupError("<VERB> form");
            else if (DUP_VERB == i) { 
                doDupError("<VERB> ");// DUP_VERB
                fskip = true;  // skip associated patterns
            }
            else {
                ResetCurs();
                WriteLine("Pattern error in .verbs file for verb ", sverb);
                switch (i) {
                    case MISS_LBRAC: doPhraseError(sErrMISS_LBRAC); break;
                    case MISS_RBRAC: doPhraseError(sErrMISS_RBRAC); break;
                    case MISS_VERBM: doPhraseError("No verb marker \'*\' has been specified"); break;				
                    case EMPTY_CODE: doPhraseError(sErrEMPTY_CODE); break;				
                    case PAT_ORPHAN: ShowWarningError("Patterns found at beginning of .verb file","A verb must come before any patterns",shError00); break;
                    case NO_VFORM:   doPhraseError("No verb forms were found inside {...}"); break;
                    case MISS_SYN:   doPhraseError("Undeclared &synset designator"); break;
                } // switch
            } // else
		}   //catch
	}     // readLine
	storeTail();
} // readVerbs

void ReadFilesClass:: readTime(void)
// input routine for <TIME>...</TIME> segment of verbs file
{
toktype idxroot;
	
	TabariFlags.fhasTimeList = true;
	
	while (readLine()) {
		if (fPhraseOK) {
//			cout << sPhrase << endl;
			if (strstr(sPhrase,"</TIME>")) return;  // normal exit point
			if (sPhrase[0] != '-') idxroot = Roots.storeTime();
			else Roots.storePattern(idxroot,Time);
		}
	}
	ShowFatalError("No </TIME> tag in file ",curFilename,shError00);
} // readTime

void ReadFilesClass:: readAttrib(void)
// input routine for <ATTRIB>...</ATTRIB> segment of verbs file
{
toktype idxroot;
	
	TabariFlags.fhasAttribList = true;
	fcodeOptional = true;  // this prevents parseline from indicating error due to no code
	
	while (readLine()) {
		if (fPhraseOK) {
//			cout << sPhrase << endl;
			if (strstr(sPhrase,"</ATTRIB")) {
				fcodeOptional = false;
				return;  // normal exit point
			}
			if (sPhrase[0] != '-') idxroot = Roots.storeAttrib();
			else Roots.storePattern(idxroot,Attrib);
		}
	}
	ShowFatalError("No </ATTRIB... tag in file ",curFilename,shError00);
} // readTime

void ReadFilesClass:: readWord(const char *sendtag, wordtype wtype)
// input routine for <NOUN> and <ADJECTIVE> segments of actors file
{	
//	if (Noun == wtype) TabariFlags.fhasNounList	= true;
//	else if (Adjctv == wtype) TabariFlags.fhasAdjctvList	= true;

	while (readLine()) {
		if (fPhraseOK) {
            if (strstr(sPhrase,sendtag)) {
                return;  // normal exit point for while(readLine()) loop
            }
            if (Literals.duplicateRoot(sPhrase)) doDupError(sendtag);
            else Roots.storeWord(sPhrase,wtype);
		}
	}
	ShowFatalError("No </NOUN> or </ADJ... tag in file ",curFilename,shError00); // make general
} // readWord

void ReadFilesClass:: readSynsets(void)
// input routine for <Synsets> file.
// Generic note <12.01.19>: &synset tokens are weird, since they are initially stored as a root,
// but then are stored as a special type of token inside patterns. Mixing these two can lead to 
// an assortment of subtle programming errors.
{
toktype idxroot;
bool fskip  = false; // skips patterns if designator is a duplicate
litstring sdesig;

	fcodeOptional = true;  // this prevents parseline from indicating error due to no code    
    isSynRoot = true;
    while (readLine()) {
        if (fPhraseOK) {
            if (strstr(sPhrase,"</SYN")) {
                isSynRoot = false;
                return;  // normal exit point for while(readLine()) loop
            }
            if (sPhrase[0] != '+') { // not pattern
 //               Processor.fprob << "rSyn:" << sPhrase << endl;;
                if (Literals.duplicateSyn(sPhrase)) {
                    doDupError("<SYNSET> designator ");
                    fskip = true;
                }
               else {
                   fskip = false;
                   Roots.storeWord(sPhrase,Synym);  // store designator
                   idxroot = Roots.iRoot;
                   strcpy(sdesig, sPhrase);      // save the root so we can error check for it
//                  Processor.fprob << "readSyn:" << sPhrase << endl; // *** debug
//                   Processor.fprob << "  index = " << idxroot << endl; // *** debug
               }	
            }
            else { // process synonym
                isSynRoot = false;  // allow synsets in patterns
                if (strstr(sPhrase,sdesig)) doPhraseError("Synset designator used in one of its own patterns");
                else if (!fskip) Roots.storePattern(idxroot,Synym);
                isSynRoot = true;
            }
        } // fPhraseOK
    }     // readLine
    ShowFatalError("No </SYNSET> tag in file ",curFilename,shError00); // make general
} // readSynsets

//___________________________________________________________________________________
// 																																	          issues

int ReadFilesClass:: getISSUE(void)
// process the ISSUES tag; returns current value of kategory
// This thing should eventually be back-fitted with a real XML parser; at the moment we
// are just faking it...  ###
// The CATEGORY text is saved but at the moment we don't do anything
// isNumeric is set to true if issue is numeric
{
static issueHeadStruct * pHead = &Coder.issueHead; // current issue header
static int curkat = 0;  			// current category
int 	 kat = curkat++;
char * pstext = sComment;		// use the class input globals for temporary storage
char * pst;

																// set up issue header
	if (pHead->kategory >= 0) {		// create a new link at end (note that pHead is static)
		pHead->pnext = new issueHeadStruct;
		pHead = pHead->pnext;
		if (!pHead) ShowFatalError("Unable to allocate memory for new issue in ReadFilesClass:: readIssues",sPhrase,shError11);
		pHead->found = false;
		pHead->pnext = NULL;
		pHead->plist = NULL;
	}
	pHead->kategory = kat;
	fisNumeric = false;

	getField(sPhrase,"CATEGORY",pstext); 	// run this first to make sure the syntax is okay
	if (*pstext) {												// but actually do the transfer with an erase
		pst = strstr(sPhrase,"CATEGORY"); 
		pst += 8;
		while (*pst != '\"') ++pst; // move to the "
		++pst;			// move past the "
		while (*pst != '\"') {  // copy text inside quotes into sComment
			*pstext++ = *pst; 
			*pst++ = '-';     // erase the text so we don't confuse it with a keyword
		}
	  *pstext = '\0';
	  pHead->phrase = CharStore.putCharStr(sComment);
  }
  else pHead->phrase = CharStore.charNullString;

	pstext = sComment;  // continue to use this for intermediate storage
	getField(sPhrase,"TYPE",pstext); // check for "TYPE=" field
	if (*pstext) {
		pHead->doAll    = (bool) strstr(pstext,"ALL");
		pHead->doNumber = (bool) strstr(pstext,"NUM");
		fisNumeric = pHead->doNumber;
	}
	else {
		pHead->doAll		= false;
		pHead->doNumber = false;
	}

	getField(sPhrase,"DEFAULT",pstext); // check for "DEFAULT=" field
	if (*pstext) pHead->icode = 	CodeStore.addCode(pstext);
	else pHead->icode = 0;
	
	return kat;					
} // getISSUE

void ReadFilesClass:: readIssues(char *sfilename)
// input routine for .issues file
// Note that lines between <ISSUE>...</ISSUE> will be skipped; this is controlled by kat < 0
{
int 		kat = -1;

	fisNumeric = false;   // also set in the constructor...
	openFile(sfilename);
	while (readLine()) {
		if (fPhraseOK) {
			if (strstr(sPhrase,"<ISSUE")) kat = getISSUE();
			else if (strstr(sPhrase,"</ISSUE")) {
				 kat = -1;
				 fisNumeric = false;
			}
			else if (kat >= 0) {
				try {Roots.storeIssue(kat,fisNumeric);}
				catch (int i) {
					if (MISS_LBRAC == i) doPhraseError(sErrMISS_LBRAC);	
					else doPhraseError(sErrMISS_RBRAC);
				}	
			}
		}
	} // while
} // readIssues

//___________________________________________________________________________________
// 																																	            FREQs

int ReadFilesClass:: getFREQ(void)
// process the FREQ tag; returns current value of kategory
{
static freqStruct * pHead = &Coder.freqHead; // current issue header
int 	 kat = Coder.iFreq++; // current category -- starts at 1
char * pstext = sComment;		// use the class input globals for temporary storage
char * pst;

																// set up issue header
	if (pHead->kategory >= 0) {		// create a new link at the end of the list (note that pHead is static)
		pHead->pnext = new freqStruct;
		pHead = pHead->pnext;
		if (!pHead) ShowFatalError("Unable to allocate memory for new issue in ReadFilesClass:: readFreq",sPhrase,shError11);
		pHead->pnext = NULL;
	}
	pHead->kategory = kat;

	getField(sPhrase,"CATEGORY",pstext); 	// run this first to make sure the syntax is okay
	if (*pstext) {												// but actually do the transfer with an erase
		pst = strstr(sPhrase,"CATEGORY"); 
		pst += 8;  // skip past known text
		while (*pst != '\"') ++pst; // move to the "
		++pst;			// move past the "
		while (*pst != '\"') {  // copy text inside quotes into sComment
			*pstext++ = *pst; 
			*pst++ = '-';     // erase the text so we don't confuse it with a keyword
		}
	  *pstext = '\0';
	  pHead->phrase = CharStore.putCharStr(sComment);
  }
  else pHead->phrase = CharStore.charNullString;

	pstext = sComment;
	getField(sPhrase,"ABBREV",pstext); 	// run this first to make sure the syntax is okay
	if (*pstext) {												// but actually do the transfer with an erase
		pst = strstr(sPhrase,"ABBREV"); 
		pst += 6;
		while (*pst != '\"') ++pst; // move to the "
		++pst;			// move past the "
		while (*pst != '\"') {  // copy text inside quotes into sComment
			*pstext++ = *pst; 
			*pst++ = '-';     // erase the text so we don't confuse it with a keyword
		}
	  *pstext = '\0';
//	  cout << "ABBREV: " << sComment << endl; // ***
	  pHead->abbrev = CharStore.putCharStr(sComment);
  }
  else pHead->abbrev = CharStore.charNullString;
	
	return kat;					
} // getFREQ

void ReadFilesClass:: readFreqs(char *sfilename)
// input routine for .freq file
// Note that lines between </FREQ>...<FREQ> will be skipped; this is controlled by kat < 0
{
int 		kat = -1;

	fdoingFreq	= true;
	openFile(sfilename);
	while (readLine()) {
		if (fPhraseOK) {
			if (strstr(sPhrase,"<FREQ")) kat = getFREQ();
			else if (strstr(sPhrase,"</FREQ")) {
				 kat = -1;
			}
			else if (kat >= 0) {
//				cout << "rf: " << sPhrase << "k = " << kat << endl;
				Literals.storeFreq(sPhrase,kat);
			}
		}
	} // while
	fdoingFreq	= false;
} // readFreqs

//___________________________________________________________________________________
// 																																	working functions

void ReadFilesClass:: doLINE_LONG(void)  
// handles LINE_LONG exception
{
instring serr;
	strcpy(serr,"Input line too long in file ");
	strcat(serr, curFilename);
	strcat(serr,"\nProblem line:\n");
	ShowWarningError(serr,sLine,"Line will be ignored",shError32);
	sLine[0] = '/';  // force line to be skipped
}	

void ReadFilesClass:: insertStr(char *s1, char *s2)  
// inserts s2 at the beginning of s1, overwriting the first char of s1
// This is typically called with s1 set in the middle of the string
// Note: this is used to replace single delimiters in the input with 
//       space-separated delimiters
{
	char *s = s2;
	char *sa = s1;
	int len;
//	char *sc = s1; // *** debugging: used to track original string
	
	while (*s) s++;  		// find end of s1, s2
	while (*sa) sa++; 
	len = (int)(s - s2) - 1;		// number of cells we need to open up
	s = sa;				// move string back len cells
	sa += len;
	while (s > s1)  *sa-- =  *s--;  // copy all but first char
	while (*s2) *s1++ = *s2++; 	// copy s2 into the open cells
} // insertStr

void ReadFilesClass:: getsLine(void)
// safe version of getline: throws LINE_LONG exception if line is too long.
// Finishes reading line before throwing exception 
{
char * ps = sLine;
int ka = 0;

	++TabariFlags.nlinesread;
	while (ka++ < MAX_TAB_INPUT) {
		fin.get(*ps);
		if (('\n' == *ps) || (fin.eof())) {
			*ps = '\0';
			return;
		}
		++ps;
	}
	--ps;
	while (('\n' != *ps) && (!fin.eof())) {  // finish up reading line
		fin.get(*ps);
	}
	*ps = '\0';  // terminate string so it can be displayed
	throw LINE_LONG;

} // getsLine

void ReadFilesClass:: openFile(char *sfilename)
{	
	fin.open(sfilename,ios::in); 
	if (fin.fail()) ShowFatalError("Unable to open the input file ",sfilename,shError03);
	WriteLine("Initializing from ",sfilename);
	strcpy(curFilename,sfilename);
	TabariFlags.nlinesread = 0;
} // open

bool ReadFilesClass::readLine(void)
// Generic phrase reading procedure that handles all of the file input;
// stores comment info if appropriate 
{	
	do {
		try {
			getsLine();
//		cout << sLine << endl; // DEBUG ***
		}
		catch (int i) { doLINE_LONG(); }	
	}
	while ((!fin.eof()) && ((!sLine[0]) || ( '/' == sLine[0]) || ( '#' == sLine[0])));	

	if ((fin.eof()) || ('~' == sLine[0])) {
		fPhraseOK = false; // ### <09.01.10> throwing an informative error here would be more useful...actually, this is just an EOF check, right?
		return false;
	}

	TrimBlanks(sLine);
	parseLine();	// pull out the various parts of the input

	if (sComment[0]) locComment = CommentFile.putComment(sComment); 	// handle comments 
	else locComment = -1;  																		// no comments on this line
	
	return true;
} // readLine

void ReadFilesClass:: storeTail(void)
// store the tail lines for Processor.nextDict in CommentFile, then close fin
{
	while (!fin.eof()) {
		try { 
			getsLine(); 
		}
		catch (int i) {  // handles LINE_LONG exception
			instring serr;
			strcpy(serr,"Final comments line too long in file ");
			strcat(serr, curFilename);
			strcat(serr,"\nProblem line:\n");
			ShowWarningError(serr,sLine,"Line will be truncated",shError32);
		}
		if (fin.eof()) break;
		if (fin.fail()) ShowFatalError("Input problem in ReadFilesClass::storeTail",shError05);
		CommentFile.putTail(sLine);
	}
	fin.clear(); 	// clear eof flag 
	fin.close(); 
	if (fin.fail()) ShowFatalError("Unable to close the input file",shError04);
} // storeTail

void ReadFilesClass::addBlanks (char *s, char c, int &len)
// adds blanks around c; increments len; does nothing if len>=MAX_TAB_INPUT
// This is currently only used for {, |, and }, and therefore has idiosyncratic
// treatment of each
{
	char *pst = s;
	char si[4] = "   ";
	bool done;
	
	if (len >= MAX_TAB_INPUT) return;
	while ((pst) && (pst = strchr(pst,c))) {		// the assignment is deliberate here
		if ((*(pst-1) != ' ') || (*(pst+1) != ' ')) {
			done = false;
			if (('{' == c) && ('_' == *(pst-1))) {  // allow _{
				strcpy(si,"{ ");
				insertStr(pst,si);
				done = true;
			}
			else if (('}' == c) && ('_' == *(pst+1))) { // allow }_
				strcpy(si," }");
				insertStr(pst,si);
				done = true;
			}
			if (done) strcpy(si,"   ");
			else {
				si[1] = c;
				insertStr(pst,si);
			}
			pst += 2;
			len += 2;
		}
		else if (pst) ++pst;
	}
} // addBlanks

void ReadFilesClass::parseLine (void)
// Generic phrase parsing procedure that handles all phrase [code] strings
// Input is through sLine 
// Sets the values of sCode, sPhrase,and sComment
// ### <09.01.10> there is a fair amount of error checking here, but switching to a throw/catch format 
//     would be better form than just using the bool fPhraseOK...

{
instring sa;
char 		*pst;
int 		len = 0;  // used to check total length of sPhrase

	fPhraseOK = false;

	if (!sLine[0]) {
		return;
	}
	if (strchr(sLine,';')) Copy2Chr(sa,sLine,';');			// get non-comment part of s 
	else strcpy(sa,sLine);
	
	if (fdoingFreq || ('<' == sa[0]) || ('&' == sa[0])) {   // XML tag, freqs and synonym designator ; just return in sPhrase
		strcpy(sPhrase,sa);
		sComment[0] = '\0'; //  no comments on this line 
		sCode[0] = '\0';		// no code either
		fPhraseOK = true;		
		return;
	}		

	if (('+' == sa[0])|| ('&' == sa[0]) || ('{' == sa[0]))  {   // actor synonym or verb form line; add a code to bypass the error checking but still continue processing
		strcat(sa," [---]");                    // ### it would be better to check that these are in the correct context, i.e. have a flag for what type of file we are reading <09.11.12>
	}		

	pst = strchr(sa,'[');
	if (pst) {
		bool gotContent = false;
		Copy2Chr(sPhrase,sa,'[');
		strcpy(sCode,pst);
		pst = sCode + 1 ;
		while (!gotContent && (*pst != ']') && *pst) {  // check for non-blank content
			if (*pst > 32) gotContent = true;  // "32" is first non-blank printing ASCII code
			++pst;
		}
		if (!gotContent) {
			doPhraseError(sErrEMPTY_CODE);
			return;
		}
	}
	else {
		strcpy(sPhrase,sa);
		strcpy(sCode,sNullCode);
		if ((!fcodeOptional) && (!fisNumeric)) {
			doPhraseError(sErrMISS_LBRAC);  // allow empty code on numeric issues
			return;
		}
	}

	pst = sPhrase;			// shift to upper case [09.06.29] and convert hyphens to underscores
	while (*pst) {
		*pst = toupper(*pst);
		if ((pst>sPhrase) && ('-' == *pst)) *pst = '_';  // don't change initial - on phrases
		++pst;
		++len;
	}
// delimit '{', '}' and '|'
	addBlanks(sPhrase,'{',len);  
	addBlanks(sPhrase,'|',len);
	addBlanks(sPhrase,'}',len);
	if (len >= MAX_TAB_INPUT)  doPhraseError("Phrase too long after adding blanks to {, |, }");
	
	TrimEndBlanks(sPhrase);
  if (!Roots.needcodes && ((!sPhrase[0]) || (!sPhrase[1]))) doPhraseError("No phrase has been specified");  // ### i.e. phrase must be at least two chars in length -- check that this is the correct contingency, e.g. A_ is as short as we go

	pst = strchr(sLine,';');
	if (pst) strcpy(sComment,++pst);	// get the comment from end of line
	else sComment[0] = '\0';  // else no comments on this line 

	fPhraseOK = true;

} // parseLine 


//___________________________________________________________________________________
// 																																		.options file

void ReadFilesClass:: optionsError (const char *s)
{
	WriteLine("This .options command has a problem: ",sLine);
	ShowWarningError(s,"Line was skipped",shError42);
} // optionsError

void ReadFilesClass:: trimCommand (char *s)
// trim command from s and delete leading blanks
{	
char *ps = strchr(s,':') + 1;   // ':' exists, as this was checked earlier

	while (isspace(*ps)) ++ps;  // skip over blanks as well
	while (*ps) *s++ = *ps++;		// delete first part of string
	*s = '\0';									// terminate string	

} // trimCommand

void ReadFilesClass:: getField (char *s, const char *substr, char *sout)
// Gets the ="..." in s following substr, assigns blank-trimmed value to sout. 
// If substr is not present or there is no number, sets iout = 0 and 
// sends ShowWarning about the syntax error
{
char * ps = strstr(s,substr);		// find start of substring ;
char * psa;

	if (!ps) {			// substring wasn't found
		*sout = '\0';
		return;
	}
	
	try {
		while ((*ps) && ('=' != *ps)) ++ps; // go to the next =
		if (!*ps) throw 1;

		while ((*ps) && ('\"' != *ps)) ++ps; // go to the next "
		if (!*ps) throw 2;

		++ps;
		psa = ps;   		// make sure we've got a closing " before copying
		while ((*psa) && ('\"' != *psa)) ++psa; // go to the next "
		if (!*psa) throw 3;

		psa = sout;  			// save the start of sout
		while ((*ps) && ('\"' != *ps)) *sout++ = *ps++; // it's okay, so copy into sout
		*sout = '\0';			// terminate the string
		TrimBlanks(psa);	// blank-trim sout
		return;
	} // try
	
	catch (int i) {  // syntax error
		litstring stemp;
		strcpy(stemp, substr);
		WriteLine("XML field in this line has a problem: ",sLine);		
		switch (i) {
		 	case 1: ShowWarningError("No = following ",stemp,"Field will be ignored.",shError42);
		 			break;
		 	case 2: ShowWarningError("No opening \" following ",stemp,"Field will be ignored.",shError42);
		 			break;
		 	case 3: ShowWarningError("No closing \" following ",stemp,"Field will be ignored.",shError42);
		 			break;
		}
		*sout = '\0';
		return;
	} // catch
		
} // getField

void ReadFilesClass:: getInteger (char *s, const char *substr, int &iout)
// Gets the first number in s following substr, assigns value to iout. 
// If substr is not present or there is no number, sets iout = 0
{
char * ps = strstr(s,substr);		// find start of substring ;
char * psa = (char *)substr;

	iout = 0;									// set iout to default
	if (!ps) return;					// substring isn't there, so quit					
	
	while (*(++psa)) ++ps; // move past the end of the substring
	++ps;
		
	while ((*ps) && !isdigit(*ps)) ++ps;  // go to first digit
	if (!*ps) return;											// no number, so default
	iout = atoi(ps);											// get the integer

} // getInteger

void ReadFilesClass:: resetInteger (char *s, const char *substr, int &iout)
// Identical to getInteger except leaves value alone if the string is absent. 
{
    char * ps = strstr(s,substr);		// find start of substring ;
    char * psa = (char *)substr;
    
	if (!ps) return;					// substring isn't there, so quit					
	
	while (*(++psa)) ++ps; // move past the end of the substring
	++ps;
    
	while ((*ps) && !isdigit(*ps)) ++ps;  // go to first digit
	if (!*ps) return;											// no number, so default
	iout = atoi(ps);											// get the integer
    
} // resetInteger

bool ReadFilesClass:: getBoolean(void)
// looks for a T/F after an =; defaults to true.  Assumes the input
//	has been shifted to upper case 
{
char * ps = strchr(sLine,'=');

	if (ps) {
		while (*ps) {
			if ('T' == *ps) return true;
			else if ('F' == *ps) return false;
			else ++ps;
		}
	}
	else optionsError("Missing '=' in command");
	return true;
} // getBoolean

int ReadFilesClass:: get3Way(void)
// looks for a OFF/T/F after an =; defaults to OFF.  Assumes the input
//	has been shifted to upper case 
{
char * ps = strchr(sLine,'=');
	if (ps) {   // ### why am I rewriting strchr()? <07.08.03>
		if (strstr(ps,"OFF")) return OFF_3WAY;
		while (*ps) {
			if ('T' == *ps) return TRUE_3WAY;
			else if ('F' == *ps) return FALSE_3WAY;
			else ++ps;
		}
	}
	else optionsError("Missing '=' in command");
	return OFF_3WAY;
} // get3Way


void ReadFilesClass:: getCOMPLEX(void)
// process the COMPLEX command 
{
char *ps;

	trimCommand(sLine);
	ps = sLine-1;
	while (*(++ps)) *ps = toupper(*ps);	// convert entire line to upper case

	if ('V' == sLine[0]) {  // deals with an initial 'VERBS'
		sLine[0] = 'X';
		getInteger(sLine, "XERBS", Parser.cplxLimit.nVerb);
	}
	else  getInteger(sLine, " VERBS", Parser.cplxLimit.nVerb); // leading space is used to avoid hitting misspelled NOVERBS, LATEVERBS

	getInteger(sLine, "ACTORS", Parser.cplxLimit.nActor);
	getInteger(sLine, "PRONOUNS", Parser.cplxLimit.nPronoun);
	getInteger(sLine, "CONJ", Parser.cplxLimit.nConj);
	getInteger(sLine, "COMMA", Parser.cplxLimit.nNonRes);
	getInteger(sLine, "LATEVERB", Parser.cplxLimit.nLateVerb);
	Parser.cplxLimit.fPreAct = strstr(sLine,"ACTPRIOR");
	Parser.cplxLimit.fPostAct = strstr(sLine,"ACTAFTER");
	Parser.cplxLimit.fNoSrc = strstr(sLine,"NOSOURCE");
	Parser.cplxLimit.fNoVerb = strstr(sLine,"NOVERB");
	Parser.cplxLimit.fNoTar = strstr(sLine,"NOTARGET");
	Parser.cplxLimit.fNoEvt = strstr(sLine,"NOEVENT");
	Parser.fCplxStrg = strstr(sLine,"EXPLAIN");

int ka = Parser.cplxLimit.nVerb + Parser.cplxLimit.nActor + Parser.cplxLimit.nPronoun + 
				 Parser.cplxLimit.nConj + Parser.cplxLimit.nNonRes + Parser.cplxLimit.nLateVerb;

bool fok = Parser.cplxLimit.fPreAct || Parser.cplxLimit.fPostAct || Parser.cplxLimit.fNoSrc || 
					 Parser.cplxLimit.fNoVerb || Parser.cplxLimit.fNoTar || Parser.cplxLimit.fNoEvt;

	Parser.fCheckCplx = (ka > 0) || fok;
} // getCOMPLEX

void ReadFilesClass:: getDEFAULT(void)
// process the DEFAULT command 
{
char *ps;
char *scode;
    
	trimCommand(sLine);
	ps = sLine-1;
	while (*(++ps)) *ps = toupper(*ps);	// convert entire line to upper case
	
    if (strstr(sLine,"OFF")) {  // deactivates all options
		Coder.fhasDefltSrc = false;	// 	default source code is available
		Coder.fhasDefltTar = false;	//  same for target
		Coder.fDefPRIOR	 = false;   //  PRIOR option
		Coder.fDefAFTER	 = false;   //  AFTER option
		Coder.idxcodeDefSrc = 0;		//  indices for default codes
		Coder.idxcodeDefTar = 0;       
    }
    
	try {
        if ((scode = strstr(sLine,"SOURCE"))) {
            ps = strchr(scode,'[');
            if ((ps-scode) > 15) throw MISS_LBRAC;  // well, this is probably what happened
            if (ps) {
                bool gotContent = false;
                ++ps;
                scode = ps;
                while ((*ps != ']') && *ps) {  // check for non-blank content
                    if (*ps > 32) gotContent = true;  // "32" is first non-blank printing ASCII code
                    if ((ps-scode) > 15) throw MISS_RBRAC;  // well, this is probably what happened
                    ++ps;
                }
                if (!(*ps)) throw MISS_RBRAC;
                if (gotContent) {
                    Coder.fhasDefltSrc  = true;	
                    if (strstr(sLine,"PRIOR")) Coder.fDefPRIOR  = true;	
                    *ps = '\0';   // clever C coding to save a zillionth of a second on a procedure that is called once...
                    Coder.idxcodeDefSrc = CodeStore.addCode(scode,(char *)"Source default");  // *** debug
                    *ps = ']';
                }
                else throw EMPTY_CODE;
            }
            else throw MISS_LBRAC;
       }
        if ((scode = strstr(sLine,"TARGET"))) {
            ps = strchr(scode,'[');
            if ((ps-scode) > 15) throw MISS_LBRAC;  // well, this is probably what happened
            if (ps) {
                bool gotContent = false;
                ++ps;
                scode = ps;
                while ((*ps != ']') && *ps) {  // check for non-blank content
                    if (*ps > 32) gotContent = true;  // "32" is first non-blank printing ASCII code
                    if ((ps-scode) > 15) throw MISS_RBRAC;  // well, this is probably what happened
                    ++ps;
                }
                if (!(*ps)) throw MISS_RBRAC;
                if (gotContent) {
                    Coder.fhasDefltTar  = true;	
                    if (strstr(sLine,"AFTER")) Coder.fDefAFTER  = true;	
                    *ps = '\0';  
                    Coder.idxcodeDefTar = CodeStore.addCode(scode,(char *)"Target default");  // *** debug
                    *ps = ']';
                }
                else throw EMPTY_CODE;
            }
            else throw MISS_LBRAC;
        }
    }
    catch (int i) { // these don't include the DEFAULT: but they seem sufficiently informative
        if (EMPTY_CODE == i) doPhraseError(sErrEMPTY_CODE);
        else if (MISS_LBRAC == i) doPhraseError(sErrMISS_LBRAC);
        else doPhraseError(sErrMISS_RBRAC);      // MISS_RBRAC
   }
} // getDEFAULT

void ReadFilesClass:: getFORWARD(void)
// process the format string in the FORWARD command
{	

char * ps;
char * pt;

	trimCommand(sLine);
	ps = sLine;
	if (!(*sLine)) return;
	
	if (strstr(sLine," DOC")) fhasDoc = true;
	else fhasDoc = false;

	if (strchr(sLine,'S') && !strstr(sLine,"OFF")) Parser.fuseForward = true;
	else Parser.fuseForward = false;
	
	while ((*ps) && ('^' == *ps)) ++ps;  // skip initial blanks
	if (*ps) {  // set IDInfo location 
	//		if ka > 1 then
	//			Delete(strin, 1, ka - 1);
		iIDInfoLoc = (int)(ps - sLine);
		pt = ps;
		while ((*pt) && (' ' != *pt)) ++pt;  // terminate at blank (end of record ID, before DOC or OFF)
		*pt = '\0';
		iIDInfoLen = (int)strlen(ps);
		if (iIDInfoLen > MAX_RECID_LEN) { 		// trim to maximum length 
				iIDInfoLen = MAX_RECID_LEN;
				ShowWarningError("FORWARD string  was >63 chars: ",ps,"String was truncated",shError41);
				*(ps+MAX_RECID_LEN - 1) = '\0';
		}
	}			
	else {
		ShowWarningError("FORWARD string had no non-blank characters","FORWARD option was not processed",shError42);
		return ;
	}

	ps = strchr(sLine,'N');
	if (ps) { 												// set story serial # location 
		iStoryLoc = (int)(ps - sLine);
		pt = ps + 1;
		while ((*pt) && ('N' == *pt)) ++pt;
		iStoryLen = (int) (pt - ps);
	}

	ps = strchr(sLine,'S');
	if (ps) { 											// set sequence serial # location 
		iSentLoc = (int)(ps - sLine);
		pt = ps + 1;
		while ((*pt) && ('S' == *pt)) ++pt;
		iSentLen = (int) (pt - ps);
	}

} // getFORWARD

void ReadFilesClass:: getOUTPUT(void)
// process the OUTPUT command 
{
	fOutputID = (bool)(strstr(sLine, "ID"));
	fOutputStory = (bool)(strstr(sLine, "STO"));
	fOutputSeq   = (bool)(strstr(sLine, "SEN"));
	fOutputDoc   = (bool)(strstr(sLine, "DOC"));
	fOutputText  = (bool)(strstr(sLine, "TEX"));
	fOutputPats  = (bool)(strstr(sLine, "PAT"));
	fOutputShift  = (bool)(strstr(sLine, "SHI"));
	if (!fOutputMatch)fOutputMatch = (bool)(strstr(sLine, "MAT")); // currently can also get this set using SET:
	fOutputActRoots	= (bool)(strstr(sLine, "ACT"));
	fOutputAgtRoots = (bool)(strstr(sLine, "AGE"));
	fOutputParent   = (bool)(strstr(sLine, "PAR"));
    fOutputLocs     = (bool)(strstr(sLine, "LOC"));

} // getOUTPUT

void ReadFilesClass:: getCOMMA(void)
// process the COMMA command 
{
	if (strstr(sLine, "OFF")) {
		Parser.minSubord	=  MAX_LEX;
		Parser.maxSubord	=  0;
		Parser.minBegSubord =  MAX_LEX; 
		Parser.maxBegSubord =  0;
		Parser.minEndSubord =  MAX_LEX;
		Parser.maxEndSubord =  0;        
    }
    else {
        resetInteger(sLine, " MIN", Parser.minSubord);
        resetInteger(sLine, " MAX", Parser.maxSubord);
        resetInteger(sLine, "BMIN", Parser.minBegSubord);
        resetInteger(sLine, "BMAX", Parser.maxBegSubord);
        resetInteger(sLine, "EMIN", Parser.minEndSubord);
        resetInteger(sLine, "EMAX", Parser.maxEndSubord);
    }
 /*   Processor. fprob <<  "Mod: " << Parser.minSubord<< "  " << Parser.maxSubord << endl;
    Processor. fprob <<  Parser.minBegSubord<< "  " << Parser.maxBegSubord << endl;
    Processor. fprob <<  Parser.minEndSubord<< "  " << Parser.maxEndSubord << endl; */
} // getCOMMA

void ReadFilesClass:: getNONEVENTS(void) 
// process the NONEVENTS command 
{
	if (strstr(sLine, "TRUE"))  TabariFlags.fNonEvents  = 1;
	else if (strstr(sLine, "ONLY"))  TabariFlags.fNonEvents  = 2;
	else TabariFlags.fNonEvents  = 0;
//	WriteLine("Set Nonevents");  // *** debug
} // getNONEVENTS

void ReadFilesClass:: getLABEL(void)
// process the LABEL command 
{
char * ps = strchr(sLine,':') + 1; // readOptions has checked that the ':' is present
char * pscode = sCode;				// use the class input globals here
char * pstext = sPhrase;

	while ((*ps) && (isspace(*ps))) ++ps; // skip over any leading white space
	if (!*ps) {
		optionsError("No code specified in LABEL command");
		return;
	}

	while ((*ps) && (*ps != '=') && (!isspace(*ps))) *pscode++ = *ps++; // copy the code
	if (*ps) *pscode = '\0';
	else {
		optionsError("No '=' in LABEL command");
		return;
	}

	while ((*ps) && (('=' == *ps) || (isspace(*ps)))) ps++; // skip the '=' and any white space
	if (!*ps) {
		optionsError("No label following '=' in LABEL command");
		return;
	}

	while (*ps) *pstext++ = *ps++; // copy the rest of the line into the label
	*pstext = '\0';
	TrimEndBlanks(sPhrase);

	CodeStore.addCode(sCode,sPhrase);  // add the code and label
} // getLABEL

void ReadFilesClass:: getSET(void)
// process the SET command 
{
char *ps;

	trimCommand(sLine);
	ps = sLine-1;
	while (*(++ps)) *ps = toupper(*ps);	// convert entire line to upper case
	
	if      (strstr(sLine, "CODE"))  Coder.setcodeMode(sLine);
	else if (strstr(sLine, "HAIKU")) TabariFlags.fShowHaiku = getBoolean();
	else if (strstr(sLine, "YYYY")) TabariFlags.f4DigitYear = getBoolean();
	else if (strstr(sLine, "TIME")) TabariFlags.fhasTimeList = getBoolean();
	else if (strstr(sLine, "LABEL")) ReadFiles.fOutputLabels = getBoolean();
	else if (strstr(sLine, "MATCH")) optionsError("SET: MATCH is no longer an option (Version 0.8+)");
	else if (strstr(sLine, "FBIS"))  TabariFlags.fFBISMode  = getBoolean();
	else if (strstr(sLine, "DUP "))  TabariFlags.fWarnDups  = getBoolean();
	else if (strstr(sLine, "SKIP "))  TabariFlags.fCheckSkip  = getBoolean();
	else if (strstr(sLine, "CONV"))  TabariFlags.fConvAgent  = getBoolean();
	else if (strstr(sLine, "MINIM")) {
		getInteger (sLine, "MINIM",Parser.minLex); // MINIMUM WORDS
//        Processor.fprob << "Reset minLex: " << Parser.minLex << endl; // *** debug
		--Parser.minLex;   // reduce by one so we can compare to iLex
	}
	else if (strstr(sLine, "USAGE")) {
		if (strstr(sLine, "ACT"))      TabariFlags.fActorUsage = get3Way();
		else if (strstr(sLine, "VER")) TabariFlags.fVerbUsage = get3Way();
		else if (strstr(sLine, "AGE")) TabariFlags.fAgentUsage = get3Way();
		else optionsError("Unrecognized SET: USAGE command");
	}
	else optionsError("Unrecognized SET: command");

} // getSET

void ReadFilesClass:: doOptionCommand(char *sCmd)
// processes on .options command. This will usually be in sLine but can be passed as a parameter sCmd
{
char *ps;
    
    if (sCmd) {
        TrimLeadBlanks(sCmd);
        strcpy(sLine, sCmd);
    }
    ps = strchr(sLine,':');
    if (!ps) {
        optionsError("No ':' in command");
        return;
    }

    ps = sLine - 1;
    while (*(++ps) != ':') 	*ps = toupper(*ps); // convert command to upper case

    switch (sLine[0]) {
            
        case 'C': switch (sLine[3]) {
            case 'M': getCOMMA();
                break;
            case 'P': getCOMPLEX();
                break;
            default : optionsError("Unrecognized command");
        }
            break;
            
        case 'D': getDEFAULT();
            break;
            
        case 'F': getFORWARD();
            break;
            
        case 'L': getLABEL();
            break;
            
        case 'N': getNONEVENTS();
            break;
            
        case 'O': getOUTPUT();
            break;
            
        case 'S': getSET();
            break;
            
        default: optionsError("Unrecognized command");
    }
} // doOptionCommand

void ReadFilesClass:: readOptions(char *sfilename)
// input routine for .options file
{
	openFile(sfilename);
	while (!fin.eof()) {
		do {
			try { getsLine();}
			catch (int i) { doLINE_LONG(); }
		}
		while ((!fin.eof()) && ((!sLine[0]) || ( '/' == sLine[0]) || ( '#' == sLine[0])));		// '//' and # is the comment indicator

		if ((fin.eof()) || ('~' == sLine[0])) {
			fin.close();
			break;
		}
		doOptionCommand(NULL);
	} // while
	if (TabariFlags.fNonEvents) ReadFiles.fOutputMatch = false; // disable this in case the user hasn't: see manual, chpt 10
	fin.close();
/*    Processor.fprob << ReadFiles.fOutputID << "  " << ReadFiles.fOutputStory  << "  " << ReadFiles.fOutputSeq  << endl;
    Processor.fprob << ReadFiles.iIDInfoLoc << "  " << ReadFiles.iIDInfoLen  << endl;
    Processor.fprob << ReadFiles.iStoryLoc << "  " << ReadFiles.iStoryLen  << endl;
    Processor.fprob << ReadFiles.iSentLoc << "  " << ReadFiles.iSentLen  << endl;
 */

} // readOptions

//___________________________________________________________________________________
// 																														class CommentFileClass

commloc CommentFileClass::putComment(char *sphrase)
 // stores comment; returns pointer
{
commloc loc = fcomm.tellp();

	while (*sphrase) {				// put sphrase into fcomm
		fcomm.put(*sphrase);
		if (fcomm.fail()) ShowFatalError("Output problem in CommentFileClass::put",shError06);
		++sphrase;
		}
		fcomm.put('\n');  // terminate with a newline so we can read this
	return loc;
	
} // put
 
void CommentFileClass::getComment(commloc loc, char *scomm)
// retrieves comment, puts in sphrase
{
	fcomm.seekg(loc);
	fcomm.getline(scomm, MAX_TAB_INPUT);
	if (fcomm.fail()) ShowFatalError("Input problem in CommentFileClass::get",shError05);
} // get

void CommentFileClass::putTail(char *sphrase)
// stores tail information -- this is the information following the ~~ FINISH
// terminator in the Processor.nextDict file.  Then closes fin
{
		
	if (Processor.nextDict->tailLoc < 0) Processor.nextDict->tailLoc = fcomm.tellp();  // record the start of the tail
	putComment(sphrase);
	++Processor.nextDict->tailLen;		
} // putTail

void CommentFileClass::reopenComment(ProcessorClass::dictListStruct *dictFile)
// close fcomm; re-opens as an ios:in file; sets cur* globals to info for dictFile
{
	
	curLen = dictFile->tailLen;
	curLoc = dictFile->tailLoc;
	
	fcomm.close(); 
	if (fcomm.fail()) ShowFatalError("Can't close comment file in CommentFileClass::reopen",shError04);
	fcomm.open(sfcommname,ios::in); 
	if (fcomm.fail()) ShowFatalError("Unable to reopen comments file",shError07);
} // reopen

bool CommentFileClass:: getTail(char *s)
// returns ~~ FINISH line and tail information.  "reopen" needs to be called first 
// note the somewhat tricky use of curLoc to determine if this is the first call
{		
	if (curLoc >= 0) {		// first time this has been called; return the ~~FINISH line
		litstring sdate,stime;
		MakeTimeDateStr (stime, sdate);			// get time and date strings
		strcpy(s,"~~FINISH ");
		strcat(s,stime);
		strcat(s,sdate);
		fcomm.seekg(curLoc);  										// go to the start of the tail
		curLoc = -1;															// signal that we've done this
		return true;
	}
	else if (--curLen >= 0) {		// read the tail, decrementing curLen
		fcomm.getline(s, MAX_TAB_INPUT);
		if (fcomm.fail()) ShowFatalError("Input problem in CommentFileClass::getTail",shError05);
		return true;
	}
	else return false;		// got everything

} // getTail	

//___________________________________________________________________________________
// 																																		debugging code

#if FALSE
void ReadFilesClass:: testOptions(void)
// *** debugging : tests assorted .options functions
{
instring stest;
	
	strcpy(stest, "COMPLEX: COMPLEX[2] DISCARD[2]");
	trimCommand(stest);
		
// getInteger ("COMPLEX: COMPLEX[2] DISCARD[2]","DISCARD", irec);
// getInteger ("COMPLEX: COMPLEX[2] DISCARD [213]","DISCARD", irec);

} // testOptions
#endif

#if FALSE
void ReadFilesClass:: test(void) 
// *** debugging function 
{
	strcpy(sLine," * {OUT|OFF|DOWN}[012]");
//	strcpy(sLine,"* OF + IN $  [212] ;JON 5/26/95");
	cout << "Input  :" << sLine << endl;
	parseLine();
	cout << "Phrase :" << sPhrase << endl;
	cout << "Code   :" << sCode << endl;
	cout << "Comment:" << sComment << endl;
	cout << "OK?    :" << fPhraseOK << endl;

} // test

void ReadFilesClass:: set(char *sphrase, char *scode, char *scomment, commloc loc)   
// sets the local variables
// *** currently just used for debugging
{
	strcpy(sPhrase, sphrase);
	strcpy(sCode, scode);
	strcpy(sComment, scomment);
	locComment = loc;
	fPhraseOK = true;
} // set

#endif


