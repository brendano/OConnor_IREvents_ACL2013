// 															TABARI Project
//__________________________________________________________________________________
// 																memory.cp

//  This file contains the various routines for handling storage and retrieval

//	CHARACTER STORAGE
//	This array of chars holds all of the permanent text -- literals, codes, 
//	labels -- used by the program (except for comments, which are stored
//	in the temporary text file "Comments.temp").  These are stored as null-
//  terminated strings and accessed exclusively with pointers.  

//	This file contains the PhraseClass routines for managing the root and pattern
// 	phrases.  When a term or phrase is read from a dictionary, it is broken down 
//	into literals (added as necessary), and then the list of literals and connectors
//	is stored as a consecutive list of toktypes (16-bit words).  All processing
//	can then be done with literals rather than characters.

// 	LITERAL STORAGE
//  Literals are the fundamental unit of character strings recognized by TABARI.  In
//	practical terms, they are words and endings.
//
//  The litArray contains four references: 
//
//		charptr pchar;      // pointer to text in charBuf				
//		toktype istart;     // index of litBuf list of phrases that begin with this literal 
//		toktype ifreq;      // index of litBuf list of freq categories containing literal	
//		toktype pnext;      // pointer to next literal in alphabetical list	
//
//  Access to this is via litIndex[], which contains pointers based on the first two
//  characters of the string.  The LiteralsClass::find() and add() functions are a 
//	bit tricky (as distinct from clever): see comments in the functions.  
//
//	istart and ifreq point to linked lists stored in litBuf[].  These have 
//  the same form: a toktype index to rootArray followed by an index to the next 
//	element in the list; if the pointer is zero, this is the end of the list.

//	PHRASE STORAGE
//	A "phrase" pointer points to a list of consecutive word pairs stored in litBuf[].  
//  This list contains a litArray[] index followed by a connection marker that is
//	determined by the literal ending (e.g. space, _, +, * etc).  A litArray index of
//	zero (valEndList) indicates the end of the list.  The markers are:
//
//	checkPattern connectors
//		connFullNext  = 0x0000;		// exact match and consecutive
//		connPartNext  = 0x4000;		// partial match and consecutive
//		connFullSkip  = 0x0400;     // exact match and skip
//		connPartSkip  = 0x4400;		// partial match and skip 
//
//	When a dictionary is saved, it is written via the phrase and code storage
//	systems, which is why the output from the program often looks nicer than the
//	input (as well as being alphabetized).

//	[Historical note: KEDS, in contrast, saved everything in its character 
//	representation.  There was a certain rationale for this -- the combination of
//   Pascal input and 1-Mhz computers meant that in the early years of KEDS, the input
//   system was *very* slow, a process that led indirectly to KEDS's "tapping shoe"
//	logo.  Because many dictionary entries would never be used, no extra processing
//	occurred until the phrase was actually needed.  Processing was done either 
//	during dictionary development -- where the user was the slowest component of the
//	system -- or in batch mode.  I.e. phrase processing was delayed until the user
//	didn't care about time.
//
//	[The *disadvantage* of this approach was that text was never chunked into basic
//	units, but always dealt with character-by-character.  Another design problem 
//	that eventually became very problematic is that phrases such as "PRESIDENT_REAGAN"
//   were permanently connected so that "PRESIDENT_RONALD_REAGAN" was a completely
//	distinct phrase.  KEDS finally acquired something vaguely resembling tokens when
//	"classes" (= TABARI "synonyms") were added near the end of the program
//	development, but this was never used extensively.]

//	ROOT STORAGE
//   Actor and verb roots are stored indirectly in a list of indices to literals.
//   The rootArray contains four references: 
//
//		tokindex 	iphrase;	// index to start of phrase list in phraseBuf 
//		wordtype 	wtype;  	// type 
//		patStruct	*ppat;		// pointer to pattern list; first pattern carries code of root 
//		tokindex 	iclass; 	// index to start of class list; zero if none 
//		int				used;			// number of times used
//

//	PATTERN STORAGE (part of RootsClass)
//	patArray primarily stores pointers to the phrases associated with verbs, but it 
//  is also used to store the codes and comments for actors.
//
//		tokptr 	phrase;   // index of phrase list in patBuf				
//		toktype 	icode;    // location of codes: see Codes.pas for details 
//		commloc 	icomment; // filepos for comments 
//		int		length;		// length of phrase; used to determine order of evaluation
//		int		used;			// number of times used
//		patStruct	*pnext;   // next pattern record; NULL if none	
//
//	Note that patBuf is not subject to the toktype restriction: it is accessed with
//  pointers.  patBuf will usually be the largest array in the system.

//  ISSUES
//	Issue patterns are stored in the manner similar to actors, with the code stored
//	in root->ppat.code.  The root.class slot holds the category of the issue; if the
//  issue is numeric, root->ppat.length holds the value.

//  FREQS
//	Frequency lists are stored directly with the literals themselves using a list in
//	litBuf pointed to by .ifreq.

//  TIME-SHIFT
//	Time-shift patterns are stored in the same structures used by verbs.  The code 
//  storage is specific to this type and is accessed through store/getTimeCode().

//	POINTERS VS. INDICES
//	The program does indirection through a combination of true pointers and indices 
//  into arrays.  (The indices are almost always of type toktype).  There is (nominally)
//  some logic behind the choice: an index is used whenever it is desirable to try 
//  to store the location in 16 or fewer bits; otherwise a true pointer is used.
//  Indices are typically used in combination with flag-bits that indicate additional
//  characteristics of the object that was stored.  
//
//  The design assumption here is that pointers are faster for directly accessing the 
//  the parts of a list, but  bit-level comparisons are a faster way to access and
//  query flags than doing the multiple memory accesses that would be required if 
//	flags were stored in separate words (as well as using less memory).  This 
//  assumption may not hold for all future machine architectures.
//
//  There are an assortment of arrays that all hold toktype values -- for example 
//  TokBuf, LitBuf and phraseBuf.  In theory these could be combined into a single
//  contiguous block of memory, but keeping them separate allows a greater number 
//  of items to be stored while being able to use the 16-bit toktype indices.  
//  If you are concerned about the total amount of memory the program requires 
//  (i.e. you don't work for Microsoft), the various array bounds -- the MAX_* 
//  constants -- can be adjusted to fit the frequency of various objects found in
//  the dictionaries for a particular problem.
// 
//__________________________________________________________________________________
//                                                              Debugging Guides
//
//  TABARI being something of a rat's nest of indirection via pointers and indices,
//  and congnizant of the rule-of-thumb that code one has not looked at for six months
//  might as well be code written by someone else, the following are an assortment
//  of code snippets for producing readable output
//
//  WARNING: printing a null pointer as if it was a string appears to crash the
//           <fstream> system without, however, generating a seg fault, and 
//           output after such an operation will not appear. So be sure to trap
//           any possible instances of this. E.g use code along the lines of
//                if (ps) Processor.fprob << "cEv2: " << ps << endl;
//                else Processor.fprob << "cEv2: *NULL* " << endl;
//
//  Print the text of a phrase in a pattern
//      patStruct *pPat = Roots.rootArray[*ptPat].ppat;
//      instring s;			// *** debug
//      pPat = pPat->pnext; // go to start of pattern list
//      Processor.fprob << "compSynt: " << Phrases.get(s, pPat->phrase) << endl; // *** debug: this is the phrase we are trying to match
//
//  Print the list of literals in a a phrase with connector values [which in fact
//  provides pretty much the same information as Phrases.get()]
//      toktype * ptok = pPat->phrase;
//      Processor.fprob  << "Phrase list :" << endl;
//      while (*ptok) {
//          Processor.fprob  << "  " << *ptok << ": " << Literals.litArray[*ptok].pchar;  // get the text
//          Processor.fprob  << " -- " << *(++ptok) << endl;  // shows the actual value of the connector
//          ++ptok;
//      }
//
//  Print the [original/filtered/ text 
//      Processor.fprob << endl << Processor.sentText << endl; // *** debug
//      Processor.fprob << Parser.filtext << endl; // *** debug  commented-out in ParserClass:: doParsing()
//
//__________________________________________________________________________________
//
//	 Copyright (c) 2002 - 2012  Philip A. Schrodt.  All rights reserved.
//
// 	Redistribution and use in source and binary forms, with or without modification,
// 	are permitted under the terms of the GNU General Public License:
// 	http://www.opensource.org/licenses/gpl-license.html
//
//	Report bugs to: schrodt@psu.edu

//	The most recent version of this code is available from the KEDS Web site:
//		http://eventdata.psu.edu

//___________________________________________________________________________________
// 															  external globals

#include "TABARI.h"

extern ReadFilesClass ReadFiles;
extern ParserClass 	Parser;
extern ProcessorClass Processor;

//___________________________________________________________________________________
//                                                                  Allocate variables

// Note: the order of initialization here is *very* important, otherwise the 
//       system will lose track of literals and roots that are initialized in the
//       constructors
//       (### which suggests that perhaps this is not the right place to 
//       initialize them...)
TabariFlagsClass 	TabariFlags;
CharStoreClass 		CharStore;
TokenStoreClass 	TokenStore;
PhrasesClass 			Phrases;
CodeStoreClass 		CodeStore; 
LiteralsClass 		Literals;
RootsClass 				Roots;

void memTest(void);


//___________________________________________________________________________________
// 																																		CharStoreClass

charptr CharStoreClass::putCharStr (char *s)
// stores a string in charBuf and returns a pointer to it 
{	
	charptr ptr = pCharLast;
	do *pCharLast++ = *s; while (*s++);
    if (pCharLast - charBuf >= MAX_CHAR - WARN_LEVEL) {
		if (pCharLast - charBuf < MAX_CHAR)
			ShowWarningError("Running out of memory in character storage", "Program will continue but may crash soon",shError10); 
		else ShowFatalError("Allocated memory exceeded in character storage." ,shError11);
	}
	return ptr; 
} // putCharStr

//___________________________________________________________________________________
// 																																		TokenStoreClass

tokptr TokenStoreClass:: getTokPtr(tokindex index) 
// gets the pointer to tokenBuf[index]
// ### inline this?
{
	return &tokenBuf[index];
}

toktype TokenStoreClass:: getToken (tokptr loc)
// returns a word from TokenBuf
{	
	return *loc;
} // getToken

tokindex TokenStoreClass:: putToken (toktype tokitem)
// stores a word in TokenBuf and returns an index to it }
{
	tokenBuf[iToken]	= tokitem;
	IncrementIndex(iToken, MAX_TOKENS, "token storage");
	return (iToken-1);
} // putToken

toktype TokenStoreClass:: getTokenValue (tokindex index)
// returns the value of tokenBuf[index]
{	
	if (index >= MAX_TOKENS) CheckIndex(index, MAX_TOKENS, "reference to token storage");
	return tokenBuf[index];
} // getTokenValue


//___________________________________________________________________________________
// 																																	Literals class

#if FALSE 
void LiteralsClass::alphaList(void)
// *** debugging :this should produce an alphabetical listing from Literals in the
// correct length order if things are working correctly.
{
int ka;
litStruct *plit;
tokindex itok;
RootsClass::rootStruct root;
instring sphr;
	
	cout << "Alphabetical listing of litArray" << endl;
	for (ka=0; ka<MAX_LIT_HASH; ka++) {
		if (litIndex[ka]) {
			plit = litIndex[ka];
			while (plit) {
				cout << plit->pchar << endl;	
				itok = plit->istart;		// go through istart list
				while (itok) {
					root =  Roots.rootArray[Literals.litBuf[itok]];
				  Phrases.get(sphr,root.phrase); 
				  cout << "  root:" << Literals.litBuf[itok] << "--" << sphr << endl;
					itok = Literals.litBuf[itok+1];		// continue through the istart list
				}	// while itok
				plit = plit->pnext;
			}
		}
	}
		
} // alphaList

void LiteralsClass::cellList(char *s)
// *** debugging :this should produce an istart listing from a single litIndex entry
{
int ka;
litStruct *plit;
tokindex itok;
RootsClass::rootStruct root;
instring sphr;
	
	ka = iHash(s);
	if (litIndex[ka]) {
		plit = litIndex[ka];
		while (plit) {
			cout << plit->pchar << endl;	
			itok = plit->istart;		// go through istart list
			while (itok) {
				root =  Roots.rootArray[Literals.litBuf[itok]];
			  Phrases.get(sphr,root.phrase); 
			  cout << "  root:" << Literals.litBuf[itok] << "--" << sphr << endl;
				itok = Literals.litBuf[itok+1];		// continue through the istart list
			}	// while itok
			plit = plit->pnext;
		}
	}
} // alphaList

void LiteralsClass:: cHash(char &ca, char &cb, int i)
//  *** debugging: returns characters that would generate hash i
{ 
	int na = i / 37;
	int nb = i % 37;

	if (!na) ca = '*';
	else if (na<27) ca = (char)(na + 64);
	else ca = (char)(na + 21);

	if (!nb) cb = '*';
	else if (nb<27) cb = (char)(nb + 64);
	else cb = (char)(nb + 21);
} // iHash

#endif

int LiteralsClass::	iHash(char *s)
//  hash into litIndex based on first two chars in s: values are
//  A - Z : 1 - 26  ||   0 - 9 : 27 - 36   || all others : 0
//  then create an index of the form *s * 37 + *(s+1) 
{ 
	int na, nb;

	if (isalpha(*s)) na = toupper(*s) - 64;
	else if (isdigit(*s)) na = *s - 21;
	else na = 0;

	s++;
	if (isalpha(*s)) nb = toupper(*s) - 64;
	else if (isdigit(*s)) nb = *s - 21;
	else nb = 0;
	
	return (na*37 + nb);
} // iHash

toktype LiteralsClass::	getIndex(litStruct *plit)
// gets the index for a pointer
{
	return (toktype)(plit - litArray);
} // getIndex

toktype LiteralsClass:: newLiteral(void)
// Increments iLit, initializes the litArray[iLit] entry, then returns iLit 
{
	IncrementIndex(iLit, MAX_LITERALS, "Literal storage");
	litArray[iLit].pchar   = NULL; 
	litArray[iLit].istart = 0; 
	litArray[iLit].ifreq   = 0; 
	litArray[iLit].pnext   = NULL; 
	return iLit;
} // newLiteral

bool LiteralsClass:: find(litStruct **plit, char *s)
// looks for s in literals; sets *plit to location if found, otherwise to the last
// location prior.  Return true if found, false otherwise.  if *plit == null, 
// litIndex is either NULL or *litIndex[]->phrase is greater than s.  The double-
// indirection **plit is used so that we can return a pointer in *plit.
// 										----- NOTE ON find() AND add() -------
// These two functions are tightly linked and a bit subtle.  find() is one of those
// deadly C functions that is doing several thing depending on the context (c.f.
// malloc(), a.k.a. the one-function memory manager).  
// When find() returns true, then it has one behavior: *plit points to the literal
// corresponding to s.
// When find() returns false, then the options get complicated:
//   if (*plit) then *plit is the litArray entry just prior to where s should be
//                   	added
//   if !(*plit) then 	the litIndex[] entry is either null or else points to a
//     							 	literal greater than s.  In either case, the litIndex[] entry
//										needs to be replaced if s is added.
// There are other ways of doing this, of course, but they seem to have similar
// levels of advantages and disadvantages.  Arguments to the contrary will be 
// entertained, however.
// That all said, you don't want to mess with the calling sequence for this function
// unless you are ready to change a lot of additional code throughout the program.
{
int index = iHash(s);	
int icomp;        		// comparison results
litStruct * prev;    	// pointer to previous item checked

//	cout << "find: check " << s << endl;

	if (!litIndex[index]) {
		*plit = NULL;
		return false;
	}
													// check the litIndex entry
	*plit = litIndex[index];
	icomp = strcmp(s,(*plit)->pchar); 
	if (!icomp) {
//		cout << "find: fondx " << (*plit)->pchar << endl;
		return true;
	}
	else if (icomp < 0) {
//		cout << "find: gtidx " << (*plit)->pchar << endl;
		*plit = NULL;
		return false;
	} 
													// check the strings in the list
	do {
		prev = *plit;						// save pointer to previous item
		*plit = (*plit)->pnext;	// check next item in list
		if (!(*plit)) break;
		icomp = strcmp(s,(*plit)->pchar);	// compare using literal-compare function
//		cout << "find: compar " << s << " < " << (*plit)->pchar << " = " << icomp << endl;
	} while ((*plit) && (icomp > 0)); 

	if ((!(*plit)) || (icomp)) {
		*plit = prev;						// return pointer to previous cell
//		cout << "find: prior " << (*plit)->pchar << endl;
		return false;
	}	
	else {
//		cout << "find: found " << (*plit)->pchar << endl;
		return true;
	}
} // find

toktype LiteralsClass:: add(litStruct *plit, char *s)
// Creates a new litArray entry, then stores s in charBuf.  If (plit == NULL), stores 
// pointer to new entry; if index was not null, litArray.pnext points to whatever was
// there. Otherwise stores at the location following plit, adjusting the litArray.pnext 
// entry accordingly.  See notes in LiteralsClass::find
{
int iha = iHash(s);
charptr pcl = CharStore.pCharLast;
toktype itok = newLiteral();

	if (plit) {															// modify the pointers in an existing list
		litArray[itok].pnext = plit->pnext;
		plit->pnext = &litArray[itok];
//		cout << "add : next  " << (plit->pnext)->pchar << endl;
	}
	else {   																// modify the litIndex
		if (litIndex[iha]) litArray[itok].pnext = litIndex[iha];	// current entry is greater than s
		litIndex[iha] = &litArray[itok];
//		cout << "add : newdx " << s << endl;
	}
	
	litArray[itok].pchar = CharStore.putCharStr(s);
	litArray[itok].length = (int)(CharStore.pCharLast - pcl - 1); // conversion here is safe because single literal will never to longer than MAX_INT
	return itok;
		
} // add


toktype LiteralsClass:: litSub(char *s1, char *s2)
// Literal comparison: returns 
//   0  if s2 is an anchored substring of s1, i.e. all characters in s1 match
//      characters in s2, starting from the beginning.
//   1  if s1 eq s2
//	 2  if s1 gt s2		
//	 3  if s1 lt s2		
{
	while ((*s1) && (*s2) && (*s1 == *s2)) {
		++s1;
		++s2;
	}
	if (!(*s2)) {
		if (*s1) return 0;	// s2 is substring of s1
		else     return 1;  // s1 eq s2
	}
		 
	if (!(*s1))	return 2; // s1 ended first
	else if (*s1 < *s2 )	return 3; 
	else return 2;
} // litSub

bool LiteralsClass:: check(char *s)
// looks for s or part of s in literals; puts the successful matches in Parser.litrArray
// starting at the current value of Parser.iLitr, and sets a zero cell when finished.
// ilitr returns the start of the literal list if there is a match, zero otherwise
// Return true if there was at least one match, false otherwise.  
// This routine is used for matching text words to the existing literals, rather than
// for storing dictionaries.
{
int index = iHash(s);	
toktype icomp;			//  holds litSub results
litStruct * plit; 
bool found = false;

//	cout << "find: checking " << s << endl;		// debug

	if (!litIndex[index]) return false;
	
													// check the strings in the list
	plit = litIndex[index];
	do {
		icomp = litSub(s,plit->pchar);	// compare using literal-substring function
//		cout << "find: compare " << s << " < " << plit->pchar << " = " << icomp << endl;		// debug
		if (icomp < 2) {
//			cout << "find: match " << plit->pchar << endl;		// debug
			Parser.addLitr(getIndex(plit),icomp);
			found = true;
		}
		plit = plit->pnext;	// check next item in list
	} while ((plit) && (icomp < 3)); 
	
	if (found) {
		Parser.addLitr(0,0); // terminate the list
		return true;
	}
	else return false;

} // check

void LiteralsClass:: create(char *s)
// do find, then add s to literals
{
litStruct * plit = NULL;

	if (find(&plit,s)) ShowWarningError("Initializing duplicate literal in LiteralsClass::create :",s,"Program will continue",shError00);
	else add(plit,s);
} // create

void LiteralsClass::addStart(tokptr ptok, toktype iRoot)
// adds a root to the start list of the first literal in phrase.  For the record, the starting
// phrase being processed is found with:
//    litArray[*Roots.rootArray[litBuf[litArray[*ptok].istart]].phrase].pchar
// so there are something like four levels of indirection here, which suggests this might
// be getting too complicated.  However, it works.
{
	toktype itok;
	
//	cout << "addst:enter iRoot =" << iRoot << ", *ptok=" << *ptok <<"->" << litArray[*ptok].pchar << endl;
		
	IncrementIndex(iLitBuf,MAX_LITBUF,"start list and synonym list storage"); 
	
	itok = litArray[*ptok].istart;
	if (itok) {
//		cout << "addst:start " << itok << "=" << litArray[*Roots.rootArray[litBuf[itok]].phrase].pchar << endl;
		while (litBuf[itok+1]) {
//			cout << "addst:loop " << itok << "=" << litArray[*Roots.rootArray[litBuf[itok]].phrase].pchar << endl;
			itok = litBuf[itok+1];	// go to next item in the list
		}
		litBuf[itok+1] = iLitBuf;  // change to point to the new entry
	}
	else {
		litArray[*ptok].istart = iLitBuf; // start a new list
//		cout << "addst:add " << *ptok << "=" << litArray[*ptok].pchar << endl;
	}

	litBuf[iLitBuf] = iRoot;   	// store root
	litBuf[++iLitBuf] = 0;			// terminate list

} // addStart

void LiteralsClass:: storeFreq(char *s, int kat)
// adds a freq index to literal in s, creating the literal if needed
{
	char *pst;
	litstring sa;
	toktype index;
	toktype itok;
	litStruct * plit = NULL;
		
	while ((*s) && (' ' == *s)) s++;	// skip leading blanks
	if (!(*s)) return;									// nothing left							
	pst = strpbrk(s," _~");
	if (pst) Copy2Chr(sa,s,*pst);   // ### need to error-check length
	else strcpy(sa,s);

	if (find(&plit,sa)) index = getIndex(plit);
	else index = add(plit,sa);
	
	IncrementIndex(iLitBuf,MAX_LITBUF,"freq list storage"); 

	itok = litArray[index].ifreq; // figure out where to store the index
	if (itok) {
		while (litBuf[itok+1]) { itok = litBuf[itok+1];}	// go to next item in the list
		litBuf[itok+1] = iLitBuf;  // change to point to the new entry
	}
	else litArray[index].ifreq = iLitBuf; // start a new list

	litBuf[iLitBuf] = kat;   	// store freq index
	litBuf[++iLitBuf] = 0;		// terminate list
	
} // storeFreq

void LiteralsClass:: countFreqs(toktype * ptok)
// counts freq codes for literal list pointed to by toklit ( = &Parser.litrArray[Parser.lexArray[index].ilit])
// Generic note: the memory and functions used to count frequencies are evenly spread across
// Coder, Parser, and Literals, all of which suggests that it need to be reorganized, or in its
// own class, or something <05.05.15>
{
toktype  indbuf;

	while (*ptok) { // go through the literal list
		indbuf = litArray[*ptok].ifreq;
		if (*(ptok+1)) { // only count full matches
			while (indbuf) {				// go through the freq codes attached to the literal
//				cout << "cF: " << litBuf[indbuf] << " " << litArray[*ptok].pchar << endl;
				assert (litBuf[indbuf] < MAX_FREQ);  // in fact should be < Coder.iFreq
				++kountFreq[litBuf[indbuf]];  
				indbuf = litBuf[indbuf+1];
//				indbuf = litBuf[litBuf[indbuf+1]];
			}
		}
		++ptok;
//				if (*ptok) WRITESTRING("_");  // this gives us the full/partial info
		++ptok;
	}
//	Pause();
} // countFreqs

bool LiteralsClass:: duplicateRoot(char *s)
// checks whether s already exists as a root
{
toktype listAr[64];
litStruct *plit;
toktype itok;
instring sphr;
instring star;
char *ps = star;
RootsClass::rootStruct root;

	strcpy(star,s);
	while (*ps) ++ps;				// add blank unless string ends in '_'
	if ('_' != *(ps-1)) {
		*ps++ = ' ';
		*ps = '\0';
	}
	Phrases.make(listAr,star);		// ### need a bounds check on listAr; maybe in make()? Also this is probably where that apparent double call arises
	if ((plit = litIndex[iHash(s)])) {  // deliberate use of assignment
 		while (plit) {						// go through litIndex list
 			if (listAr[0] == getIndex(plit)) {
                 itok = plit->istart;		// go through istart list
                while (itok) {
                    root =  Roots.rootArray[Literals.litBuf[itok]];
                    Phrases.get(sphr,root.phrase);  // get the phrase
                    if (!strcmp(sphr,star)) return true;   // found it
                    itok = litBuf[itok+1];		// continue through the istart list
                }	// while itok
            } 	// if listAr
		plit = plit->pnext;	// continue through litIndex list
		}		// while plit
	} 		// if litIndex
	return false;					// not found

} // duplicateRoot

bool LiteralsClass:: duplicateSyn(char *s)
// checks whether s already exists as a synset designator
// As usual, can't use dupRoots because of the special storage of designators
{
    litStruct *plit;
    toktype itok;
    instring sphr;
    instring star;
    RootsClass::rootStruct root;
    
	strcpy(star,s);
    strcat(star," ");
//    Processor.fprob << "\ndupR: " << "\"" << s << "\"  \"" << star << "\"\n"; // *** debug
	if ((plit = litIndex[iHash(s)])) {  // deliberate use of assignment
		while (plit) {						// go through litIndex list
            itok = plit->istart;		// go through istart list
             while (itok) {
                root =  Roots.rootArray[Literals.litBuf[itok]];
                Phrases.get(sphr,root.phrase);  // get the phrase
//                Processor.fprob << "dupR: comp " << sphr << " ?= " << star << endl; // *** debug
                if (!strcmp(sphr,star)) return true;   // found it
                itok = litBuf[itok+1];		// continue through the istart list
            }	// while itok
            plit = plit->pnext;	// continue through litIndex list
		}		// while plit
	} 		// if litIndex
	return false;					// not found    
} // duplicateSyn

//________________________________________________________________________________________
//																																				PhrasesClass

void PhrasesClass::add2Phrase(toktype index,toktype marker)   
// adds the next token to the rootBuf phrase list; |'s the marker with maskContin; #### <08.12.31> Huh?? -- no it doesn't...
// returns index of marker
{
	++iPhrase;
	if (iPhrase > WARN_PHRASES) ShowWarningError("Running out of phrase list storage", "Program will continue but may crash soon",shError00); 
	phraseBuf[iPhrase] = index;
	phraseBuf[++iPhrase] = marker;
} // add2Phrase


tokptr PhrasesClass:: store(char *s)
// store s in phraseBuf
// Notes:
// 1. <12.01.13>: On routine input, this is called twice when storing roots, once by initRoot and then from make() 
//    when it is called by checkDuplicates. I think. Anyway, it looks like there is a redundant call
//    here but that is what it is supposed to be doing. Seem to run into this every time I get back into the program.
// 2. <12.01.16>: The flag Roots.isSynRoot is required because the synset roots are initially stored as conventional
//    literals so that they can be found later in the patterns. However, when stored as part of a pattern, they
//    are a pointer to a RootBuf entry which contains a list of the elements of the synset.
{
	char *pst;
	litstring sa;
	toktype index;
	litStruct * plit = NULL;
	tokptr 	pstart = &phraseBuf[iPhrase+1];    // this is where phrase will start; we will at least store an empty list
    bool fIncr = false;
    
//    int flag = 0; // debug
	
	while (s) {
		while ((*s) && (' ' == *s)) s++;	// skip leading blanks
		if (!(*s)) break;									// nothing left
		pst = strpbrk(s," _~=");
		if (pst) Copy2Chr(sa,s,*pst);   // ### need to error-check length
		else {
            strcpy(sa,s);
//			if ('&' == sa[0]) { Processor.fprob << "End string = " << sa << endl;} // *** debug
        }
/*		if ('&' == sa[0]) {
            Processor.fprob << "Phr:store -- Synset detected:" << sa << endl;
            flag = 1;
        }; // *** debug 
//*/
		if (Literals.find(&plit,sa)) index = Literals.getIndex(plit);
		else index = Literals.add(plit,sa);
        
                                                                    // add token to phrase list with connector
/*		if (flag) {
            Processor.fprob << "Phr:store mk2 -- " << sa << endl;
            if (pst) Processor.fprob << "Phr:store pst -- " << *pst << endl;
            else Processor.fprob << "Phr:store pst -- *NULL*" << endl;
            flag = 0;
        }; // debug 
//*/
		if (pst) {
			if ('&' == sa[0]) {  // store a synset designator
				tokindex itok  = Literals.litArray[index].istart;
//				Processor.fprob << " index = " << index << " istart = " << itok << " litBuf = " << Literals.litBuf[itok] << endl; // *** debug
                if (itok) {
                	if(' ' == *pst) add2Phrase(Literals.litBuf[itok],flagSynmFS);
                	else			 add2Phrase(Literals.litBuf[itok],flagSynmPN);
                }
                else throw MISS_SYN;
//                index = 0;  // this is just a placeholder for the next segment of the phrase, which will hold the connector
				
			}
            else {
                fIncr = false;
                if      (' ' == *pst) add2Phrase(index,connPartSkip);   // wildcard and unlimited intermediate
                else if ('=' == *pst) add2Phrase(index,connEqual);      // cancels addition of regular endings
                else if ('~' == *pst) {  // this is actually the TABARI ending '~_' 
                    add2Phrase(index,connPartNext); // #### probably should error-check here for _
                    fIncr = true;	// signal to skip over '_'
                }
                else if ('_' == *pst) {  
                    if (' ' == *(pst+1)) {    // '_ ' literal word ending, unlimited intermediate
                        add2Phrase(index,connFullSkip);
                        fIncr = true;	// signal to skip over '_'
                    }
                    else add2Phrase(index,connFullNext);
                }
                else add2Phrase(index,0);   // ### this is a default -- shouldn't hit this
                if (fIncr) ++pst;
            }
		s = pst + 1;		// move past connector and blanks
        }
        else {						// last item
            s = '\0';
			if (('&' == sa[0]) && (!ReadFiles.isSynRoot)) {  // store a synset designator
				tokindex itok  = Literals.litArray[index].istart;
//				Processor.fprob << "last: index = " << index << " istart = " << itok << " litBuf = " << Literals.litBuf[itok] << endl; // *** debug
                if (itok) add2Phrase(Literals.litBuf[itok],flagSynmFS); // implict open ending
                else throw MISS_SYN;
                index = 0;
			}
            else add2Phrase(index,connPartSkip);  
        }
	}	
	add2Phrase(valEndList,0);   // terminate the phrase list
	return pstart;
} // store

void PhrasesClass:: make(toktype tokar[], char *s)
// store s in tokar but not in phraseBuf
{
	phrindex oldPhrase = iPhrase;
	tokptr tstart = tokar;
	tokptr pstart = store(s);		// temporarily put the list in phraseBuf

	while ((*tstart++ = *pstart++)); // transfer to tokar; deliberate use of = and empty while
	iPhrase = oldPhrase;  					// restore old value of iPhrase
} // make

char * PhrasesClass:: get(char *sphrase, tokptr phrase)
// gets the text of a phrase and connector from list beginning at phrase
{
toktype mask;

  *sphrase = '\0';
  while (*phrase) {
	  if (flagSynm & *(phrase+1)) {
          strcat(sphrase,"*Synset_Root* "); // ### <12.01.24? it would be nice to actually show this but so far I haven't figured out a straightforward way to do this.
          ++phrase;
      }
      else {
          strcat(sphrase,Literals.litArray[*phrase].pchar);
          mask = *(++phrase);
          if (mask == connPartSkip) strcat(sphrase," "); 
          else if (mask == connFullNext) strcat(sphrase,"_"); 
          else if (mask == connFullSkip) strcat(sphrase,"_ "); 
          else if (mask == connPartNext) strcat(sphrase,"~_"); 
          else if (mask == connEqual) strcat(sphrase,"=");
      }
	  ++phrase;
  }
  return sphrase;
} // get

//___________________________________________________________________________________
// 																																		Roots class

patStruct * RootsClass::newPattern(void)   
//	increments iPattern; initializes patArray[iPattern] and returns a pointer to 
//	patArray[iPattern]
{ 
	IncrementIndex(iPattern, MAX_PATTERN, "Pattern storage");
	patArray[iPattern].phrase 	= NULL; // location of pattern list in Pat_Buf				
	patArray[iPattern].icode	 	= 0;    // location of codes string
	patArray[iPattern].icomment = 0; 		// filepos for comments 
	patArray[iPattern].length	  = 0; 		// this will get reset shortly
	patArray[iPattern].used			= 0; 		// number of times used
	patArray[iPattern].pnext 		= NULL; // next pattern record; 0 if none	
	
	return &patArray[iPattern];
	
} // newPattern

void RootsClass::initRoot(void)
// Handles the standard rootArray[iRoot] initializations
{	
//    if ('&' == ReadFiles.sPhrase[0]) { Processor.fprob << "initRoot store = \"" << ReadFiles.sPhrase << "\", isSynRoot = " << isSynRoot << endl; } // *** debug
	rootArray[iRoot].phrase = Phrases.store(ReadFiles.sPhrase);
	Literals.addStart(rootArray[iRoot].phrase,iRoot);

	rootArray[iRoot].ppat 					= newPattern();
	rootArray[iRoot].ppat->icomment = ReadFiles.locComment;

	rootArray[iRoot].used		= 0;		// number of times used
	rootArray[iRoot].iclass = 0;    // used for verb forms, issues, actors
	rootArray[iRoot].iDict  = Processor.curDict;    // index of source dictionary
//	Processor.fprob << "RC:iR index " << Processor.curDict << "  iRoot " << iRoot << endl;  // *** debug

} // RootsClass::initRoot 

void RootsClass::storeActor(void)
// store the current contents of InPhrase as an Actor in rootArray
{	
	if ('+' == ReadFiles.sPhrase[0]) { // synonym
		char * ps = ReadFiles.sPhrase;
		do {  //  take off the initial '+'
			++ps;
			*(ps-1) = *ps;
		} while (*ps);
		if (Literals.duplicateRoot(ReadFiles.sPhrase)) throw DUP_ROOT;
		IncrementIndex(iRoot, MAX_ROOTS, "Root storage (actors)");
//		Processor.fprob << "\nstore: " << iRoot << " " << ReadFiles.sPhrase << endl;
		initRoot();
		rootArray[iRoot].wtype = Actor;
		rootArray[iRoot].iclass = actParentIdx;
		--iPattern;                                         // delete the new pattern
		rootArray[iRoot].ppat = rootArray[iRoot-1].ppat ;   // set to same pattern as previous entry
	}
	else if (!ReadFiles.sPhrase[0]) { // add date restricted block to sdaterest
		if (strlen(sdaterest) + strlen(ReadFiles.sCode) + 1 < MAX_TAB_INPUT) {
			strcat(sdaterest,ReadFiles.sCode);
			strcat(sdaterest," ");
		}
		else throw DATES_LONG;
	}
	else { // single line entry
		if (needcodes) {
			rootArray[iRoot].ppat->icode = CodeStore.storeActorString(sdaterest);  // fill in codes for previous entry
			if (!strcmp(ReadFiles.sPhrase, sDiscardCode)) return;  // signals EOF in actors file
		}
		if (Literals.duplicateRoot(ReadFiles.sPhrase)) throw DUP_ROOT;
		IncrementIndex(iRoot, MAX_ROOTS, "Root storage (actors)");
//		Processor.fprob << "\nstore: " << iRoot << " " << ReadFiles.sPhrase << endl;
		initRoot();
		rootArray[iRoot].wtype = Actor;
		actParentIdx = iRoot;
		sdaterest[0] = '\0';
		if(strchr(ReadFiles.sCode,'[')) {  // on-line codes
			needcodes = false;
			rootArray[iRoot].ppat->icode = CodeStore.storeActorString(ReadFiles.sCode);
		}
		else needcodes = true; // codes to follow
 	}

} // RootsClass::storeActor 

void RootsClass::storeAgent(void)
// store the current contents of InPhrase as an Actor in rootArray
{	
// instring s;
	IncrementIndex(iRoot, MAX_ROOTS, "Root storage (agents)");
//	cout << "\nstore: " << iRoot << " " << ReadFiles.sPhrase << endl;
	initRoot();
	rootArray[iRoot].wtype = Agent;
	rootArray[iRoot].ppat->icode 		= CodeStore.storeAgentCode(ReadFiles.sCode);
/*	if (strstr(ReadFiles.sCode,"~GOV")) {
		CodeStore.getAgentCode(s,rootArray[iRoot].ppat->icode);
		cout << "RC:sA1 "<< ReadFiles.sCode << "  new:" << s << endl;
	}
//*/
} // RootsClass::storeActor 

toktype RootsClass::storeVerb(void)
// store the current contents of InPhrase as a Verb in rootArray
// rootArray[iRoot].iclass == 0 from initRoot() signals this is initially set as a primary verb  
{	
	IncrementIndex(iRoot, MAX_ROOTS, "Root storage (verbs)");
//	cout << "\nstore: " << ReadFiles.sPhrase << endl;
	initRoot();
	rootArray[iRoot].wtype = Verb;
	rootArray[iRoot].ppat->icode 		= CodeStore.storeEventString(ReadFiles.sCode);
//  Processor.fprob << "RC:sV Mk2 " << ReadFiles.sPhrase << " :: " << ReadFiles.sCode << endl; // *** debug
	return iRoot;
} // RootsClass:: storeVerb 

void RootsClass::storeVerbForm(toktype idxroot)
// store alternative form of verb, copying code and patterns from rootArray[idxroot]
{	
	IncrementIndex(iRoot, MAX_ROOTS, "Root storage (verbs)");
//	cout << "\nstore: " << ReadFiles.sPhrase << endl;
	rootArray[iRoot].phrase = Phrases.store(ReadFiles.sPhrase);
	Literals.addStart(rootArray[iRoot].phrase,iRoot);

	rootArray[iRoot].ppat 					= rootArray[idxroot].ppat;	
	rootArray[iRoot].wtype = Verb;
	rootArray[iRoot].ppat->icode 		= rootArray[idxroot].ppat->icode;

	rootArray[iRoot].ppat->icomment = ReadFiles.locComment;  // ### this stores the comment multiple times...
	rootArray[iRoot].used		= 0;		// number of times used
	rootArray[iRoot].iclass = -1;		// signal this is a verb form entry
	rootArray[idxroot].iclass = iRoot; // record this form; iclass will contain the highest index of form roots	

} // RootsClass:: storeVerbForm 

toktype RootsClass::storeTime(void)
// store the current contents of InPhrase as a Time in rootArray
{	
	IncrementIndex(iRoot, MAX_ROOTS, "Root storage (time)");
	initRoot();
	rootArray[iRoot].wtype = Time;
	rootArray[iRoot].ppat->icode 		= CodeStore.storeTimeCode(ReadFiles.sCode);
	return iRoot;
} // RootsClass:: storeTime 

toktype RootsClass::storeAttrib(void)
// store the current contents of InPhrase as a Time in rootArray
// if no code is present, code defaults to the null code -- see ReadFilesClass::parseLine ()
{	
	IncrementIndex(iRoot, MAX_ROOTS, "Root storage (attrib)");
	initRoot();
	rootArray[iRoot].wtype = Attrib;
	rootArray[iRoot].ppat->icode = CodeStore.storeEventString(ReadFiles.sCode); // if we have a code, store same as verb
	return iRoot;
} // RootsClass:: storeAttrib 

void RootsClass::storeIssue(int kat, bool isNumeric)
// store the current contents of InPhrase as an Issue in rootArray
//
// Throws MISS_RBRAC
//
{	
instring sa;
char * pst;

	IncrementIndex(iRoot, MAX_ROOTS, "Root storage (issues)");
//	cout << "\nstore: " << iRoot << " " << ReadFiles.sPhrase << endl;
	initRoot();
	rootArray[iRoot].wtype = Issue;
	rootArray[iRoot].iclass = kat;		// issue category

	pst = strchr(ReadFiles.sCode,'[');
	if (pst) ++pst;
	else throw MISS_LBRAC; // error: no [ exists
	if (strchr(pst,']')) Copy2Chr(sa, pst,']');  // copy the string inside [...]  
	else throw MISS_RBRAC; // error: no ] exists
	TrimBlanks(sa);
	if (isNumeric) rootArray[iRoot].ppat->length = atoi(sa);
	else rootArray[iRoot].ppat->icode = CodeStore.addCode(sa);
} // RootsClass::storeIssue 

void RootsClass::storeType(const char *sphrase, wordtype wtype)
// store sphrase as an wtype in rootArray
// Note that "storeType" is only used to set required words such as pronouns and 
// conjunctions; it is not used for file input and therefore never carries a 
// comment.  For input, use storeWord()
{	
litstring s;  // overkill, but let's be safe

    strcpy(s,sphrase);
	IncrementIndex(iRoot, MAX_ROOTS, "Root storage (type)");
//	rootArray[iRoot].phrase = Phrases.store(sphrase);
	rootArray[iRoot].phrase = Phrases.store(s);
	Literals.addStart(rootArray[iRoot].phrase,iRoot);

	rootArray[iRoot].wtype	= wtype;
	rootArray[iRoot].ppat 	= NULL;

	rootArray[iRoot].used	= 0;		// number of times used
	rootArray[iRoot].iclass = 0;		
} // RootsClass:: storeType 

void RootsClass::storeWord(char *sphrase, wordtype wtype)
// store sphrase as an wtype in rootArray; used for Noun and Adjct
{	
	IncrementIndex(iRoot, MAX_ROOTS, "Root storage (synset, noun or adjctv)");
	initRoot();
	rootArray[iRoot].wtype	= wtype;
	rootArray[iRoot].ppat->icode = 0; // these don't have codes
} // RootsClass:: storeWord 

void RootsClass::storePattern(toktype idxroot, wordtype wtype)
// stores current pattern and code in the pattern list of rootArray[idxroot]
{
	patStruct *pPat = rootArray[idxroot].ppat;
	patStruct *ptemp;
	int len = 0;
	int alt = 0;
//	instring s; // ### debug

//	cout << 	ReadFiles.sPhrase << endl;
	lengthPattern(len,alt);    // get the pattern lengths
	
	while ((pPat->pnext) && 
           (pPat->pnext->length >= len)) pPat = pPat->pnext;  // go pointer to first item shorter, or to end of list
	
	ptemp = pPat->pnext;					// insert at this point in the list
	pPat->pnext = newPattern();		// create a new pattern, add to end of list
	pPat = pPat->pnext;						// now pPat points to it
	pPat->pnext = ptemp;
	
//	cout << "sPhrase: "<< &ReadFiles.sPhrase[1] << endl;
	pPat->phrase 		= Phrases.store(&ReadFiles.sPhrase[1]);  // store, skipping leading - or +
//	cout << Phrases.get(s,pPat->phrase) << endl; Pause();
	if (Synym == wtype) pPat->icode = 0;  // no codes for synonyms
	if (Time == wtype)  pPat->icode = CodeStore.storeTimeCode(ReadFiles.sCode);
	else				pPat->icode = CodeStore.storeEventString(ReadFiles.sCode);
	pPat->icomment 	= ReadFiles.locComment;
	pPat->length 	= len + alt;
	pPat->maxalt 	= alt;
	pPat->used		= 0;

} // RootsClass:: storePattern 


bool RootsClass:: isNullVerb(rootindex iroot)
// determines if rootArray[iroot] has a null primary code; 
{
	if (CodeStore.indexNull == TokenStore.tokenBuf[rootArray[iroot].ppat->icode]) return true;
	else return false;
} // RootsClass:: isNullCode

bool RootsClass:: isNullActor(rootindex iroot)
// determines if rootArray[iroot] has a null primary code after date restrictions are resolved
{
tokptr pt = NULL;
toktype istart;			// start of code storage
toktype idxactor;		// returns the token for the date-restricted actor code; checked for discard 
toktype idxagent;		// place-holder only

 	istart = Roots.rootArray[iroot].ppat->icode;
	CodeStore.nextActorCode (idxactor, idxagent, &pt, istart);
	if (pt) idxactor = istart;  // location of a complex code, just get the first one
	if (CodeStore.indexNull == idxactor) return true;
	else return false;
} // RootsClass:: isDiscardCode

bool RootsClass:: isDiscardCode(rootindex iroot)
// determines if rootArray[iroot] has a discard primary code
{
tokptr pt = NULL;
toktype istart;			// start of code storage
toktype idxactor;		// returns the token for the date-restricted actor code; checked for discard 
toktype idxagent;		// place-holder only

	if (NULL == Roots.rootArray[iroot].ppat) return false;  // root has no code
 	istart = Roots.rootArray[iroot].ppat->icode;
 	CodeStore.nextActorCode (idxactor, idxagent, &pt, istart);
	if (pt) idxactor = istart;  // location of a complex code, just get the first one
// 	if (CodeStore.indexDiscard == TokenStore.tokenBuf[rootArray[iroot].ppat->icode]) return true;
	if (CodeStore.indexDiscard == idxactor) return true;
	else return false;
} // RootsClass:: isDiscardCode

void RootsClass:: lengthPattern(int &len, int &alt)
// Returns the number of alphanumeric characters (plus ' and _) in the pattern in  
// sPhrase and sets two parameters in a pattern:
//   len = number of fixed chars + maxalt 
//   alt = total length of longest strings inside alternative patterns.  
{
char * ps = &ReadFiles.sPhrase[1];  // '-' is the first char, so it is skipped
int templen = 0;  // used in calculating alternatives
int maxlen  = 0;

	len = 0;
	alt = 0;
	
	while (*ps) {
		if (('_' == *ps) || ('\'' == *ps)) ++len;  // count underscores and apostrophes
		if (!isspace(*ps) && (!ispunct(*ps))) ++len;  // count alphanumerics
		else if (*ps == '{') {   // find maximum length of contingent phrase
			while (*ps != '}') {
				++ps;
				while ((*ps != '|') && (*ps != '}'))	{ // count substring length
					if (('_' == *ps) || ('\'' == *ps)) ++templen; 
					if (!isspace(*ps) && (!ispunct(*ps))) ++templen;
					++ps;
				}
				if (templen > maxlen) maxlen = templen;
				templen = 0;
			}	  
			alt += maxlen;
			maxlen = 0;
		}
		++ps;
	}
	
} // RootsClass:: lengthPattern

//___________________________________________________________________________________
// 																																		debugging code

#if FALSE
void MemTest(void)
// utility program for testing memory functions
{
litStruct * plit = NULL;
toktype itok;
instring sp,sc;
char *pst;
int ilit;

	Literals.create("ABC");
	Literals.create("MNO");
	Literals.create("XYZ");
	Literals.create("ABCDE");
	Literals.create("ABCXY");
	Literals.create("ABDG");
	Literals.create("ABFE");
	
	Literals.alphaList();
	Pause();

	Parser.initLitr();
	ilit = Parser.getLitr();
	if (Literals.check("ABC")) Parser.showLitr(ilit+1); 
	else cout << "void" << endl;
	Pause();

	ilit = Parser.getLitr();
	if (Literals.check("ABCX")) Parser.showLitr(ilit+1); 
	else cout << "void" << endl;
	Pause();

	ilit = Parser.getLitr();
	if (Literals.check("ABCXYZ")) Parser.showLitr(ilit+1);  
	else cout << "void" << endl;
	Pause();

	ilit = Parser.getLitr();
	if (Literals.check("ABF")) Parser.showLitr(ilit+1); 
	else cout << "void" << endl;

	Pause();

#if FALSE  // test of the old Literals.search function
	if (Literals.search(itok,&pst,"ABC")) cout << "match:'" << Literals.litArray[itok].pchar << "' last = " << *pst <<endl;
	else cout << "void" << endl;
	GetAnswer("Pause",'Y');
	if (Literals.search(itok,&pst,"ABCX")) cout << "match:'" << Literals.litArray[itok].pchar << "' last = " << *pst <<endl;
	else cout << "void" << endl;
	GetAnswer("Pause",'Y');
	if (Literals.search(itok,&pst,"ABCXYZ")) cout << "match:'" << Literals.litArray[itok].pchar << "' last = " << *pst <<endl;
	else cout << "void" << endl;
	GetAnswer("Pause",'Y');
	if (Literals.search(itok,&pst,"ABF")) cout << "match:'" << Literals.litArray[itok].pchar << "' last = " << *pst <<endl;
	else cout << "void" << endl;
	GetAnswer("Pause",'Y');
#endif

#if FALSE
//	InPhrase.set("ABC_DEF GHI JKL","[123:456]","A comment",0);
	InPhrase.set("+ SAID BELIEVED $ WOULD SAID MAINTAIN *","[123:456]","A comment",0);
	Roots.storeActor();
	Roots.getActor(sp,sc,1);
	cout << "Phrase:" << sp << endl;
	cout << "Code  :" << sc << endl;
#endif

#if FALSE
	InPhrase.set("--ABCD{EFGH}","*{*","A comment",0);
	cout << "Input :" << InPhrase.sPhrase << endl;
	pst = strchr(InPhrase.sPhrase,'{');
	InPhrase.insertStr(pst,InPhrase.sCode);
	cout << "Output:" << InPhrase.sPhrase << endl;
#endif
	
} // memTest
#endif

