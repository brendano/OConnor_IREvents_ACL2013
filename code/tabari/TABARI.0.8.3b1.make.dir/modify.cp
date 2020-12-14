// 																																		TABARI Project
//__________________________________________________________________________________
// 																																	 			modify.cp

//  This file contains the various routines for modifying dictionaries.  A number of
//  the routines call input/output functions directly rather than going through 
//  interface.cp.
//
//	This is a minimalist (but blazingly fast...) interface that was originally 
//  implemented as an expedient, but has proven to be preferable to the GUI of
//  KEDS, a result consistent with general work on human interface, so we are
//  keeping with it.
//
//  This was originally implemented in the Metrowerks SIOUX interface, which didn't
//	even implement an equivalent of the old BASIC/UCSD-Pascal GOTOXY() command for
//	writing to the screen, so this version doesn't even contain cursor controls.
//  It has yet to be completely re-written for ncurses, and probably should be.

//	Notes:
//	1. Menus implement a number of "do what I mean" checks to respond intuitively
//     to user entries (e.g. <return>) that are not specified in the prompt

//__________________________________________________________________________________

//__________________________________________________________________________________
//
//	 Copyright (c) 2002 - 2007  Philip A. Schrodt.  All rights reserved.
//
// 	Redistribution and use in source and binary forms, with or without modification,
// 	are permitted under the terms of the GNU General Public License:
// 	http://www.opensource.org/licenses/gpl-license.html
//
//	Report bugs to: schrodt@ku.edu

//	The most recent version of this code is available from the KEDS Web site:
//		http://web.ku.edu/keds

//	For plausible indenting of this source code, set the tab size in your editor to "2"

//___________________________________________________________________________________
// 																																   external globals

#include "TABARI.h"

//___________________________________________________________________________________
// 																																   Global Variables

extern TabariFlagsClass TabariFlags;
extern LiteralsClass 		Literals;
extern RootsClass 			Roots;
extern PhrasesClass 		Phrases;
extern CodeStoreClass 	CodeStore; 
extern ReadFilesClass 	ReadFiles;
extern ProcessorClass 	Processor;

ModifyClass 		 Modify;

//___________________________________________________________________________________
//																																						driver

void ModifyClass:: doModify(void)
// driver for dictionary modification
{
int ichoice = modifyMenu();

	if (0 == ichoice) {						// handle actors
		ichoice = startDisplayMenu(false);
		if (0 == ichoice) displayRoots(Actor); 
		else if (1 == ichoice)  addRoot(Actor); 
	}
	else if (1 == ichoice) {
		ichoice = startDisplayMenu(false);
		if (0 == ichoice) displayRoots(Verb); 
		else if (1 == ichoice)  addRoot(Verb); 
	}
	if (2 == ichoice) {						// handle actors
		ichoice = startDisplayMenu(false);
		if (0 == ichoice) displayRoots(Agent); 
		else if (1 == ichoice)  addRoot(Agent); 
	}
	else if (3 == ichoice) {
		ichoice = startDisplayMenu(true);
		if (0 == ichoice) displayRoots(Noun); 
		else if (1 == ichoice)  addRoot(Noun); 
	}
	else if (4 == ichoice) {
		ichoice = startDisplayMenu(true);
		if (0 == ichoice) displayRoots(Adjctv); 
		else if (1 == ichoice)  addRoot(Adjctv); 
	}
} // doModify


//___________________________________________________________________________________
//																																				utilities

bool ModifyClass:: enterCode(bool fnew)
// enter code string into ReadFiles.sCode, adding [] if needed,set locComment to -2
{
instring s;

	WRITEEOL();
	if (fnew) GetInput(s,"Enter new code");
	else  GetInput(s,"Enter code");
	if (!s[0]) return false;
	if (strchr(s,'[')) {
		strcpy(ReadFiles.sCode, s);
		if (!strchr(s,']')) strcat(ReadFiles.sCode,"]"); 
	}
	else {												// add brackets
		strcpy(ReadFiles.sCode, "[");
		strcat(ReadFiles.sCode, s);
		if (!strchr(s,']')) strcat(ReadFiles.sCode,"]");  // <07.08.02> somehow we were occasionally gettting a ']]'
	}
	ReadFiles.locComment = -2;				// signal to add code id and date
	return true;
} // enterCode

//___________________________________________________________________________________
//																																						menus

char ModifyClass:: getResponse(void)
//	gets a character response from a menu, echos it, and does an assortment of
//	"do what I mean" substitutions.  These are relevant only on the first 
//	character entered; many should be retained on the second character.
//	If you don't like idiot-proofing, by-pass this -- for example one can imagine
//	situations where the "implicit shift" would create problems.
{
char c = getUpper();
	if ('.' == c) c = '>';   // implicitly do a shift to '>'
	else if (',' == c) c = '<';   // same for '<'
	else if ('=' == c) c = '+';   // same for '+'
	return c;
} // getResponse

int ModifyClass:: startDisplayMenu(bool deletePrompt)
// Initial menu in the root modification
{
	if (deletePrompt) WRITESTRING("\nEnter first two letters for delete list, + to add, < to exit menu ->");
	else WRITESTRING("\nEnter first two letters for list, + to add, < to exit menu ->");
	start[0] = getResponse();
	if ('+' == start[0]) return 1;
	else if ('<' == start[0]) return 2;
	start[1] = getUpper();
	if (isspace(start[1])) start[1] = ' ';  // another "do what I mean..."  
	WRITEEOL();
	return 0;
} // startDisplayMenu


int ModifyClass:: modifyMenu(void)
// Menu for the primary modify selection
{
	ClearScreen();
	while (true) {
		showMenu("Modify: A)ctors  V)erbs  aG)ents  N)ouns  aD)jectives eX)it ->");
		switch (getUpper()) {
		case '\n':;						// handles <return> in Linux
		case '\r':;						// handles <return>
		case 'A': return 0;
		case 'V': return 1;
		case 'G': return 2;
		case 'N': return 3;
		case 'D': return 4;
		case '<':;						
		case 'X': return 9;
		}
		correctMenu("Please enter A, V, G, N, D, or X");
	}	
} // ModifyMenu

int ModifyClass:: changeMenu(void)
// Menu for code/delete option
{
	while (true) {
		showMenu("Change: C)ode  D)elete  eX)it ->");
		switch (getUpper()) {
		case '\n':;						// handles <return> in Linux
		case '\r':;						// handles <return>
		case 'C': return 0;
		case 'D': return 1;
		case '<':;
		case 'X': return 2;
		}
		correctMenu("Please enter C, D, or X");
	}	
} // changeMenu

int ModifyClass:: changeRootMenu(void)
// Menu for code/delete option
{
	while (true) {
		showMenu("Change: A)dd  D)elete  eX)it ->");
		switch (getUpper()) {
		case '\n':;						// handles <return> in Linux
		case '\r':;						// handles <return>
		case '+':;            // assorted "do what I mean..."
		case '=':;
		case 'A': return 0;  
		case '-':;
		case 'D': return 1;
		case '<':;
		case 'X': return 2;
		}
		correctMenu("Please enter A, D, or X");
	}	
} // changeRootMenu

int ModifyClass:: changeVerbMenu(void)
// Menu for pattern/code/delete option
{
	while (true) {
		showMenu("Change: P)attern  C)ode  D)elete  eX)it ->");
		switch (getUpper()) {
		case '\n':;						// handles <return> in Linux
		case '\r':;						// handles <return>
		case 'P': return 0;
		case 'C': return 1;
		case 'D': return 2;
		case '<':;
		case 'X': return 3;
		}
		correctMenu("Please enter P, C, D, or X");
	}	
} // changeVerbMenu

int ModifyClass:: pauseDisplayMenu(wordtype wty)
// Intermediate menu in the root modification
{
char ca, cb;

	while (true) {
		if ((Noun == wty) || (Adjctv == wty)) WRITESTRING("Enter two digits to delete word, > to continue list, < to exit list ->");
		else WRITESTRING("Enter two digits to change, > to continue list, < to exit list ->");
		ca = getResponse();
		if (('\r' == ca) || ('\n' == ca)){  // select first item on <return>
			iSelect = 1;
			return 0;
		}
		if (isspace(ca) || ('>' == ca)) return 1;   // more "do what I mean"
		if (('=' == ca) || ('+' == ca)) return 2;   // allow "add" even though it isn't listed
		else if ('<' == ca) return 3;
		cb = getUpper();
		if (isspace(cb) && isdigit(ca)) {		// so let them do single digits...
			cb = ca;
			ca = '0';
		}
		iSelect = (int) (10*ca + cb - 528);  //  = 10*(ca-48) + (cb-48)   convert to integers
		if ((iSelect < minItem) || (iSelect > maxItem)) {
			char sout[64];
			sprintf(sout,"Please enter a number between %d and %d\n", minItem, maxItem);
			WRITESTRING(sout);
		}
		else return 0;
	}
} // pauseDisplayMenu

int ModifyClass:: pausePatternMenu(bool fend)
// 	Menu for pattern modification.  If fend == true, then the "continue" option
//	is not displayed
// ### does this need an explicit refresh?? <05.04.13>
{
char ca, cb;

	WRITEEOL();
	while (true) {
		if (fend) {
			WRITESTRING("Enter two digits to change, + to add, < to exit list ->");
			ca = getResponse();
			if (('\r' == ca) || ('\n' == ca)){  // select first item on <return>
				iSelect = 1;
				return 0;
			}
			if (isspace(ca) || ('+' == ca)) return 1;   // more "do what I mean"
			else if ('>' == ca) return 3; 			// Note this is different than below
			else if ('<' == ca) return 3;
		}
		else {
			WRITESTRING("Enter two digits to change, + to add, > to continue list, < to exit list ->");
			ca = getResponse();
			if (('\r' == ca) || ('\n' == ca)){  // select first item on <return>
				iSelect = 1;
				return 0;
			}
			if (isspace(ca) || ('+' == ca)) return 1;   // more "do what I mean"
			else if ('>' == ca) return 2; 
			else if ('<' == ca) return 3;
		}
		cb = getUpper();
		iSelect = (int) (10*ca + cb - 528);  //  = 10*(ca-48) + (cb-48)   convert to integers
		if ((iSelect < minItem) || (iSelect > maxItem)) {
			char sout[64];
			sprintf(sout,"Please enter a number between %d and %d\n", minItem, maxItem);
			WRITESTRING(sout);
		}
		else return 0;
	}
} // pausePatternMenu


//___________________________________________________________________________________
//																																						displays

void ModifyClass:: displayRoots(wordtype wty)
// writes the part of the actors or verbs dictionary to the screen beginning at start
{
	int ka;
	litStruct *plit;
	toktype itok;
	instring sphr,scode;
	RootsClass::rootStruct root;
	int nline = 0;
	char cd1 = '0';				// row labels
	char cd2 = '0'; 
	
	minItem = 1;
	maxItem = WINDOW_HEIGHT - 4; 

	ka = Literals.iHash(start);
	while (true) {
	while (ka < MAX_LIT_HASH) {
		plit = Literals.litIndex[ka];
		while (plit) {						// go through litIndex list
			itok = plit->istart;		// go through istart list
			while (itok) {
				root =  Roots.rootArray[Literals.litBuf[itok]];
				if (wty == root.wtype) {
					irootSelect[nline] = Literals.litBuf[itok];
					Phrases.get(sphr,root.phrase);  // get the phrase
					if ((Actor == wty) || (Agent == wty)) CodeStore.getActorString(scode,root.ppat->icode);
					else if (Verb == wty) CodeStore.getEventString(scode,root.ppat->icode);
					else *scode = '\0';
														// [no codes on nouns and adjectives]
					++cd2;						// increment the line numbers
					if (cd2 > '9') {
						cd2 = '0';
						++cd1;
					}
					WRITECHAR(cd1);
					WRITECHAR(cd2);
					WRITESTRING("  ");
					WRITESTRING(sphr);
					WRITESTRING("  ");
					WRITESTRING(scode);
					WRITEEOL();
					++nline;
					if (nline >= WINDOW_HEIGHT - 4) {
						ResetCurs();
						move(0,0); clrtoeol();  
						move(2,0); clrtoeol();
						move(1,0); clrtoeol();
						switch (pauseDisplayMenu(wty)) {
							case 0:		
								if (Verb == wty) changeVerb(irootSelect[iSelect-1]);
								else if ((Actor == wty) || (Agent == wty)) changeActor(irootSelect[iSelect-1]);
								else deleteRoot(irootSelect[iSelect-1]);
								return;
							case 1:
								nline = 0;	// reset and continue listing
								cd1 = '0';
								cd2 = '0';
								ClearScreen();
								WRITEEOL(); WRITEEOL();WRITEEOL(); // maintain alignment on screen
								break;
							case 2: addRoot(wty);
							case 3: return;			// finished; do nothing
						}
					} // if nline
				}
				itok = Literals.litBuf[itok+1];		// continue through the istart list
			}	// while itok
		plit = plit->pnext;	// continue through litIndex list
		}	// while plit
		++ka;								// go to the next litIndex entry
	} // while ka

	WriteLine(" ---------- END OF DICTIONARY ---------- ");
	
	ResetCurs();
	maxItem = nline; 
	switch (pauseDisplayMenu(wty)) {
		case 0:		
			if (Verb == wty) changeVerb(irootSelect[iSelect-1]);
			else if ((Actor == wty) || (Agent == wty)) changeActor(irootSelect[iSelect-1]);
			else deleteRoot(irootSelect[iSelect-1]);
			return;
		case 1:
			ka = 0;  // wrap to the beginning of the dictionary
			nline = 0;	// reset and continue listing
			cd1 = '0';
			cd2 = '0';
			ClearScreen();
			WRITEEOL(); WRITEEOL();WRITEEOL(); // maintain alignment on screen
			break;
		case 2: addRoot(wty);
		case 3: return;			// finished; do nothing
	}
	}
} // displayRoots

void ModifyClass:: displayPatterns(int iroot)
// display the patterns for rootArray[iRoot]
{
	instring sphr, scode, sroot;
	patStruct *pPat;
	int nline = 0;
	char cd1 = '0';				// row labels
	char cd2 = '0'; 

	minItem = 1;
	theRoot =  Roots.rootArray[iroot];
	Phrases.get(sroot,theRoot.phrase);  // get the phrase
	pPat = theRoot.ppat->pnext;	// go through patterns list
	if (!pPat) {
		WRITEEOL();
		if (GetAnswer(sroot,(char *)" has no patterns.  Add a new pattern")) addPattern(iroot);
		return;
	}
	ClearScreen();
	WriteLine("Patterns for ",sroot);
	while (pPat) {
		ppatSelect[nline] = pPat;
		Phrases.get(sphr,pPat->phrase); // get the phrase
		CodeStore.getEventString(scode,pPat->icode);
		++cd2;
		if (cd2 > '9') {
			cd2 = '0';
			++cd1;
		}
		WRITECHAR(cd1);
		WRITECHAR(cd2);
		WRITESTRING(" - ");
		WRITESTRING(sphr);
		WRITESTRING("  ");
		WRITESTRING(scode);
		WRITEEOL();
		++nline;
		if (nline >= WINDOW_HEIGHT - 4) {
			ResetCurs();
			maxItem = nline+1;
			switch (pausePatternMenu(false)) {
				case 0:		
					Processor.changeDict(Roots.rootArray[iroot].iDict, Verb);
					changePattern(ppatSelect[iSelect-1]);
					return;
				case 1:						// 
					addPattern(iroot);
					return;
				case 2: 
					nline = 0;	// reset and continue listing
					cd1 = '0';
					cd2 = '0';
					ClearScreen();
					WriteLine("Patterns for ",sroot);
					break;
				case 3:
					return;			// finished; do nothing
			}
		} // if nline
		pPat = pPat->pnext;
	} // while pPat

	WRITEEOL();
	maxItem = nline+1;
	nline = pausePatternMenu(true);
	if (0 == nline) {  // ### <09.01.06> is there some way to move this code into the loop above?
		Processor.changeDict(Roots.rootArray[iroot].iDict, Verb);
		changePattern(ppatSelect[iSelect-1]);
	}
	else if (1 == nline)	addPattern(iroot);

} // displayPatterns


//___________________________________________________________________________________
//																															modification routines

void ModifyClass:: setupChange(void)
// set up a modify screen
{
	ClearScreen();
	Processor.showRecord();
	hline(0,WINDOW_WIDTH);	
    ResetCurs(); move(++TabariFlags.cursr,0);
} // setupChange

bool ModifyClass:: checkAgentCode(void)
// loop until we get a valid agent code or null entry
{
	if (strchr(ReadFiles.sCode,'~')) return true;
	else {
		while (true) {
			WriteLine("Agent codes require a \'~\'; please try entering code again");
			if (!enterCode(false)) return false;
			if (strchr(ReadFiles.sCode,'~')) return true;
		}
	}
} //  checkAgentCode(void)

void ModifyClass:: addRoot(wordtype wty)
// adds a new actor
{
char * ps;

	setupChange();
	if (Actor == wty) GetInput(ReadFiles.sPhrase,"Enter new actor");
	else if (Agent == wty) GetInput(ReadFiles.sPhrase,"Enter new agent"); 
	else if (Verb == wty) GetInput(ReadFiles.sPhrase,"Enter new verb"); 
	else if (Noun == wty) GetInput(ReadFiles.sPhrase,"Enter new noun");
	else GetInput(ReadFiles.sPhrase,"Enter new adjective");
	if (!ReadFiles.sPhrase[0]) return;
	ps = ReadFiles.sPhrase;
	while (*ps) {
		*ps = toupper(*ps);
		++ps;
	}

	if ((Actor == wty)  || (Agent == wty) || (Verb == wty)) {
		if (!enterCode(false)) return;
		if ((Agent == wty) && (!checkAgentCode())) return;
	}

	Processor.changeDict(0, wty);

	if (Actor == wty) Roots.storeActor();
	else if (Agent == wty) Roots.storeAgent();
	else if (Verb == wty) Roots.storeVerb();
	else Roots.storeWord(ReadFiles.sPhrase,wty);

} // addRoot

void ModifyClass:: addPattern(int iroot)
// adds a new pattern
{
instring s;
char * ps;

	setupChange();
	GetInput(s,"Enter pattern");
	if (!s[0]) return;
	if (s[0] != '-') {
		strcpy(ReadFiles.sPhrase,"-");
		strcat(ReadFiles.sPhrase,s);
	}
	else strcpy(ReadFiles.sPhrase,s);
	ps = ReadFiles.sPhrase;
	while (*ps) {
		*ps = toupper(*ps);
		++ps;
	}	
	if (!enterCode(false)) return;
	Processor.changeDict(Roots.rootArray[iroot].iDict, Verb);
	Roots.storePattern(iroot,Verb);	
} // addPattern

void ModifyClass:: deleteRoot(int iroot)
// Delete noun or adjective
{
instring sphr;
RootsClass::rootStruct root = Roots.rootArray[iroot];;

//	setupChange();
	Phrases.get(sphr,root.phrase);  // get the phrase
//	if (Noun == wty) WriteLine("Noun: ",sphr);
//	else WriteLine("Adjective: ",sphr);
//	switch (changeRootMenu()) {
//		case 0:	
//			addRoot(wty);					// change code
//			return;
//		case 1: 
			if (GetAnswer((char *)"\nDo you really want to delete ",sphr)) {
				Roots.rootArray[iroot].wtype = Null; 
				WriteLine("Deleted");
				Processor.changeDict(Roots.rootArray[iroot].iDict, Actor);  // these are stored in an actors dictionary, so mark the change in it
			}
			else WriteLine("Delete cancelled");
//		case 2: return;			// do nothing and exit
//	}
} // deleteRoot

void ModifyClass:: changeActor(int iroot)
// change code or delete actor or agent
{
instring sphr,scode;
RootsClass::rootStruct root = Roots.rootArray[iroot];

	setupChange();
	Phrases.get(sphr,root.phrase);  // get the phrase
	CodeStore.getActorString(scode,root.ppat->icode);
	if (Actor == root.wtype) WriteLine("Actor: ",sphr);
	else                     WriteLine("Agent: ",sphr);
	WriteLine("Code:  ",scode );
	switch (changeMenu()) {
		case 0:						// change code
			if (!enterCode(true)) return;
			if ((Agent == root.wtype) && (!checkAgentCode())) return;

			try {root.ppat->icode = CodeStore.storeActorString(ReadFiles.sCode);}
			catch (int i) {
				 ShowWarningError("no code in [...] in ModifyClass:: changeActor","Something is going very wrong here; contact program support; no user fixes are possible",shError98);
				 return;
			}
			root.ppat->icomment = -2;
			Processor.changeDict(root.iDict, Actor);
			return;
		case 1: 
			if (GetAnswer((char *)"\nDo you really want to delete ",sphr)) {
				Roots.rootArray[iroot].wtype = Null; 
				WriteLine("Deleted");
				Processor.changeDict(root.iDict, Null);  // Null is just a placeholder here
			}
			else WriteLine("Delete cancelled");
		case 2: return;			// do nothing and exit
	}
} // changeActor

void ModifyClass:: changeVerb(int iroot)
// display patterns, change code or delete verb
{
instring sphr,scode;
RootsClass::rootStruct root =  Roots.rootArray[iroot];

	setupChange();
	Phrases.get(sphr,root.phrase);  // get the phrase
	CodeStore.getEventString(scode,root.ppat->icode);
	WriteLine("Verb:  ",sphr);
	WriteLine("Code:  ",scode);
	switch (changeVerbMenu()) {
		case 0:						// change patterns
			displayPatterns(iroot);	
			return;	
		case 1:						// change code
			if (!enterCode(true)) return;
			try {root.ppat->icode = CodeStore.storeEventString(ReadFiles.sCode);}
			catch (int i) {
				 ShowWarningError("Missing \']\' in code in ModifyClass:: changeVerb","Something is going very wrong here; contact program support; no user fixes are possible",shError98);
				 return;
			}
			root.ppat->icomment = -2;
			Processor.changeDict(root.iDict, Verb);
			return;
		case 2: 
			if (GetAnswer((char *)"\nDo you really want to delete ",sphr)) {
				Roots.rootArray[iroot].wtype = Null;
				WriteLine("Deleted");
				Processor.changeDict(root.iDict, Verb);
			}
			else WriteLine("Delete cancelled");
	case 3: return;			// do nothing and exit
	}
} // changeVerb

void ModifyClass:: changePattern(patStruct *ppat)
// change code or delete pattern
{
	instring sphr,scode;

	setupChange();
	Phrases.get(sphr,ppat->phrase);  // get the phrase
	CodeStore.getEventString(scode,ppat->icode);
	WriteLine("Pattern:  ",sphr);
	WriteLine("Code:     ",scode);
	switch (changeMenu()) {
		case 0:						// change code
			if (!enterCode(true)) return;
			try  {ppat->icode = CodeStore.storeEventString(ReadFiles.sCode);}
			catch (int i) {
				 ShowWarningError("Missing \']\' in code in ModifyClass:: changePattern","Something is going very wrong here; contact program support; no user fixes are possible",shError98);
				 return;
			}
			ppat->icomment = -2;
			return;
		case 1: 
			if (GetAnswer((char *)"\nDo you really want to delete ",sphr)) { 
				deletePattern(ppat); 
				WriteLine("Deleted");
			}
			else WriteLine("Delete cancelled");
		case 2: return;			// do nothing and exit
	}
} // changePattern

void ModifyClass:: deletePattern(patStruct *ppat)
// delete pattern from list
{
	patStruct *pPat = theRoot.ppat->pnext;

	if (pPat == ppat) {  // delete first pattern
		theRoot.ppat->pnext = ppat->pnext;
	}
	else {
		while (pPat && (pPat->pnext != ppat)) pPat = pPat->pnext; // find pointer to ppat
		if (pPat->pnext != ppat) {
			WriteLine("Error in deletePattern: not found");return;		
		}
		pPat->pnext = ppat->pnext;
	}
} // deletePattern
