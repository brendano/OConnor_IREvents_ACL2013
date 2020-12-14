// 																																		TABARI Project
//___________________________________________________________________________________
// 																																	 			output.cp

//  This file contains the routines for writing files
 
//__________________________________________________________________________________
//
//	 Copyright (c) 2000 - 2009 Philip A. Schrodt.  All rights reserved.
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

	WriteFilesClass WriteFiles;

//___________________________________________________________________________________
// 																																   Global Variables

extern TabariFlagsClass TabariFlags;
extern LiteralsClass 		Literals;
extern RootsClass 			Roots;
extern PhrasesClass 		Phrases;
extern CodeStoreClass 	CodeStore; 
extern CommentFileClass	CommentFile;		
extern ProcessorClass 	Processor;

//___________________________________________________________________________________

void WriteFilesClass:: openFile(char *sfilename)
// opens fout for writing
{
	fout.open(sfilename,ios::out); 
	if (fout.fail()) ShowFatalError("Unable to open the output file ",sfilename,shError03);
	ResetCurs();
	WriteLine("Writing ", sfilename);
} // openFile

void WriteFilesClass:: makeLine(char *s, char *scode, patStruct *pPat)
// adds scode and comments from *pPat pattern to s[]
{
	instring scomm;

	if (pPat->icomment >= 0) CommentFile.getComment(pPat->icomment, scomm);
	else if (-2 == pPat->icomment) strcpy(scomm,Processor.scommNew);
	else *scomm = '\0';
	strcat(s," ");
	strcat(s,scode);
	if (*scomm) {
		strcat(s," ;");
		strcat(s,scomm);
	}
} // openFile

void WriteFilesClass:: writeActors(ProcessorClass::dictListStruct *dictFile)
// writes the *dictFile .actors or .agents dictionary 
{
	int 			ka;
	int 			itype; 
	litStruct *plit;
	toktype 	itok;
	instring 	sphr,scode;
	wordtype 	wtype; 
	RootsClass::rootStruct root;
	int indexDict;
//	char ca,cb;  // *** debug
	
	openFile(dictFile->fileName);
	indexDict = dictFile->index;
	CommentFile.reopenComment(dictFile); 

	*scode = '\0';  // set to empty for nouns, adjectives
	
	for (itype = 0; itype < 3 ; ++itype) {  // output the various types of patterns
		if (0 == itype) { // <NOUN>
			if (dictFile->hasNounList) {
				wtype = Noun;
				fout << "<NOUN>" <<endl;
			} 
			else continue;				
		}
		else if (1 == itype) { // <ADJECTIVE>
			if (dictFile->hasAdjctList) {
				wtype = Adjctv;
				fout << "<ADJECTIVE>" << endl;
			} 
			else continue;				
		}
		else wtype = dictFile->dictType;  // main .actors and .agents dictionary 		

		for (ka = 0; ka<MAX_LIT_HASH; ka++) {   // go through litArray in alphabetical order
			if ((plit = Literals.litIndex[ka])) {  // deliberate use of assignment
	//			Literals.cHash(ca,cb,ka);  // *** debug
	//			cout << "--litIndex[" << ca << cb << "]: " << ka << endl;
				while (plit) {						// go through litIndex list
					itok = plit->istart;		// go through istart list
					while (itok) {
						root =  Roots.rootArray[Literals.litBuf[itok]];
						if ((root.iDict == indexDict) && (wtype == root.wtype)) {
							Phrases.get(sphr,root.phrase);  // get the phrase
							if (2 == itype) CodeStore.getActorString(scode,root.ppat->icode); // only actors have codes
							makeLine(sphr,scode,root.ppat);
							fout << sphr <<endl;
						}
						itok = Literals.litBuf[itok+1];		// continue through the istart list
					}	// while itok
				plit = plit->pnext;	// continue through litIndex list
				}	// while plit
			} // if litIndex
		} // for ka

		if (0 == itype) fout << "</NOUN>" <<endl;
		else if (1 == itype) fout << "</ADJECTIVE>" << endl;
	
	} // for itype

	while (CommentFile.getTail(sphr)) { 
		fout << sphr << endl;  // retrieve tail; reopen() should be called earlier
	}
	
	fout.close();
} // writeActors

void WriteFilesClass:: writeVerbs(ProcessorClass::dictListStruct *dictFile)
// writes the .verbs dictionary to sfilename
{
	int 				ka;
	int 				itype; 
	int         indexDict;
	litStruct *	plit;
	toktype 		itok;
	instring 		sphr,scode,spat;
	patStruct *	pPat;
	wordtype 		wtype; 
	RootsClass::rootStruct root;
//	char ca,cb;  // *** debug
	
	openFile(dictFile->fileName);
	indexDict = dictFile->index;
	CommentFile.reopenComment(dictFile);
	
	for (itype = 0; itype < 3 ; ++itype) {  // output the various types of patterns
		if (0 == itype) { // <TIME>
			if (TabariFlags.fhasTimeList) {
				wtype = Time;
				fout << "<TIME>" <<endl;
			} 
			else continue;				
		}
		else if (1 == itype) { // <ATTRIBUTE>
			if (TabariFlags.fhasAttribList) {
				wtype = Attrib;
				fout << "<ATTRIBUTION>" << endl;
			} 
			else continue;				
		}
		else wtype = Verb;   // verbs		

		for (ka = 0; ka<MAX_LIT_HASH; ka++) {   // go through litArray in alphabetical order
			if ((plit = Literals.litIndex[ka])) {  // deliberate use of assignment
	//			Literals.cHash(ca,cb,ka);  // *** debug
	//			cout << "--litIndex[" << ca << cb << "]: " << ka << endl;
				while (plit) {						// go through litIndex list
					itok = plit->istart;		// go through istart list
					while (itok) {
						root =  Roots.rootArray[Literals.litBuf[itok]];
						if (root.iclass < 0) {  // root is a verb form, so it will be output elsewhere
							itok = Literals.litBuf[itok+1];		
							continue;
						}
						if ((root.iDict == indexDict) && (wtype == root.wtype)) {
							Phrases.get(sphr,root.phrase);  // get the phrase
							if (0 == itype) CodeStore.getTimeString(scode,root.ppat->icode); 
							else if (1 == itype) {
								CodeStore.getEventString(scode,root.ppat->icode); 
								if (!strncmp(&scode[1],sNullCode,3)) *scode = '\0'; // eliminate if null code
							} 
							else CodeStore.getEventString(scode,root.ppat->icode);
							makeLine(sphr,scode,root.ppat);
							fout << sphr <<endl;
							
							if (root.iclass > 0) { // verb has verb forms, so write them in {...}
								toktype idx = Literals.litBuf[itok];
								fout << "{ ";
								while (++idx <= root.iclass) {
									Phrases.get(sphr,Roots.rootArray[idx].phrase);  // get the phrase
									fout << sphr << " ";
								}
								fout << "}" <<endl;
							}
							
							pPat = root.ppat->pnext;	// go through patterns list
							while (pPat) {
								Phrases.get(sphr,pPat->phrase); // get the phrase
								strcpy(spat,"- ");							// add the leading "- "
								strcat(spat,sphr);
								if (0 == itype) CodeStore.getTimeString(scode,pPat->icode); 
								else if (1 == itype) {
									CodeStore.getEventString(scode,root.ppat->icode); 
									if (!strncmp(&scode[1],sNullCode,3)) *scode = '\0'; // eliminate if null code
								} 
								else CodeStore.getEventString(scode,pPat->icode);
								makeLine(spat,scode,pPat);
								fout << spat <<endl;
								pPat = pPat->pnext;
							} // while pPat
						}
						itok = Literals.litBuf[itok+1];		// continue through the istart list
					}	// while itok
				plit = plit->pnext;	// continue through litIndex list
				}	// while plit
			} // if litIndex
		} // for ka

		if (0 == itype) fout << "</TIME>" <<endl;
		else if (1 == itype) fout << "</ATTRIBUTION>" << endl;
	
	} // for itype

	while (CommentFile.getTail(sphr)) {
		fout << sphr << endl;  // retrieve tail; reopen() should be called earlier
	}
	
	fout.close();
} // writeVerbs

void WriteFilesClass:: writeActorUsage(bool doActors)
// writes the actors or agents dictionaries with usage statistics to fileName.use
{
	int ka;
	litStruct *plit;
	toktype itok;
	instring sphr,scode;
	int curDict = 0;
	wordtype wtype = Agent;
	RootsClass::rootStruct root;
	ProcessorClass::dictListStruct *dictFile = &Processor.dictFileList;	
	
	if (doActors) wtype = Actor;
	
	while (dictFile) {
		if ((doActors && (Actor != dictFile->dictType)) || 
				(!doActors && (Agent  != dictFile->dictType))) {
			dictFile = dictFile->next;
			continue;
		}	
		strcpy(sphr,dictFile->fileName);
		strcat(sphr,".usage");
		openFile(sphr);
		curDict = dictFile->index;

		MakeTimeDateStr(sphr,scode);
		fout << "Usage statistics for " << dictFile->fileName << endl;
		fout << sphr << scode << endl;
		fout << endl;
		for (ka = 0; ka<MAX_LIT_HASH; ka++) {   // go through litArray in alphabetical order
			if ((plit = Literals.litIndex[ka])) {  // deliberate use of assignment
				while (plit) {						// go through litIndex list
					itok = plit->istart;		// go through istart list
					while (itok) {
						root =  Roots.rootArray[Literals.litBuf[itok]];
						if ((wtype == root.wtype) && (curDict == root.iDict)) {
							Phrases.get(sphr,root.phrase);  // get the phrase
							fout << root.used << "\t" << sphr <<endl;
						}
						itok = Literals.litBuf[itok+1];		// continue through the istart list
					}	// while itok
				plit = plit->pnext;	// continue through litIndex list
				}	// while plit
			} // if litIndex
		} // for ka

		fout.close();
		dictFile = dictFile->next;
	} // while
} // writeActorUse

void WriteFilesClass:: writeVerbUsage(void)
// writes the verbs dictionaries with usage statistics to fileName.use
{
	int ka;
	litStruct *plit;
	toktype itok;
	instring sphr,scode,spat;
	patStruct *pPat;
	RootsClass::rootStruct root;
	int total;
	ProcessorClass::dictListStruct *dictFile = &Processor.dictFileList;	
	int curDict = 0;
	
	while (dictFile) {
		if (Verb != dictFile->dictType) {
			dictFile = dictFile->next;
			continue;
		}	
		strcpy(sphr,dictFile->fileName);
		strcat(sphr,".usage");
		openFile(sphr);
		curDict = dictFile->index;

		MakeTimeDateStr(sphr,scode);
		fout << "Usage statistics for " << dictFile->fileName << endl;
		fout << sphr << scode << endl;
		fout << endl;

		for (ka = 0; ka<MAX_LIT_HASH; ka++) {   // go through litArray in alphabetical order
			if ((plit = Literals.litIndex[ka])) {  // deliberate use of assignment
				while (plit) {						// go through litIndex list
					itok = plit->istart;		// go through istart list
					while (itok) {
						root =  Roots.rootArray[Literals.litBuf[itok]];
						if ((Verb == root.wtype)  && (curDict == root.iDict)) {
							Phrases.get(sphr,root.phrase);  // get the phrase
							fout << root.used << "\t" << sphr <<endl;
							total = 0;
							
							pPat = root.ppat->pnext;	// go through patterns list
							while (pPat) {
								Phrases.get(scode,pPat->phrase); // get the phrase
								strcpy(spat,"- ");							// add the leading "- "
								strcat(spat,scode);
								fout << pPat->used << "\t" << spat <<endl;
								total += pPat->used;
								pPat = pPat->pnext;
							} // while pPat
						if (root.ppat->pnext) fout << total << "\tPattern total for " << sphr << endl;
						fout << endl;					
						}
						itok = Literals.litBuf[itok+1];		// continue through the istart list
					}	// while itok
				plit = plit->pnext;	// continue through litIndex list
				}	// while plit
			} // if litIndex
		} // for ka
		
		fout.close();
		dictFile = dictFile->next;
	} // while
} // writeVerbUse
