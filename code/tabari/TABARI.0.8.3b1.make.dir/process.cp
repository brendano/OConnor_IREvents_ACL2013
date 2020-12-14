
// 																																		TABARI Project
//___________________________________________________________________________________
// 																																	 			process.cp

//  This file contains the routines for handling the primary reading and coding of
//	text records
 
//  ----------------------------------------------------------------------------  
//  PROJECT FILE PROCEDURES 
//	             				
//   The Project file contains all of the information the program needs to do	
//   coding -- phrase files, problem files, indicator for records coded and		
//   so forth -- and also tracks the history of the coding for a data set.  It	
//   is stored as a single record of type Project_Rec.						
// 
//   FILES:                                								
//   textFile    : Source of input text 								
//   verbFile    : Verb phrase file     								
//   actorFile   : Actors file          								
//   optionsFile : Options file  (subtle, eh?)       						
// 
//__________________________________________________________________________________
//
//	 Copyright (c) 2000 - 2012  Philip A. Schrodt.  All rights reserved.
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

	ProcessorClass Processor;

//___________________________________________________________________________________
// 																																   Global Variables

extern TabariFlagsClass   TabariFlags;
extern CharStoreClass 	CharStore;
extern TokenStoreClass 	TokenStore;
extern LiteralsClass 		Literals;
extern CodeStoreClass 	CodeStore; 
extern RootsClass 		Roots;
extern PhrasesClass 		Phrases;
extern ReadFilesClass		ReadFiles;		
extern WriteFilesClass 	WriteFiles;
extern ParserClass		Parser;		
extern CoderClass			Coder;		
extern ModifyClass		Modify;
extern ReadXMLClass 		ReadXML;

	
//___________________________________________________________________________________

void ProcessorClass:: getsLine(void)
// safe version of getline: throws LINE_LONG exception if line is too long
// Note that at the moment LINE_LONG always results in a ShowFatalError, 
// so this doesn't finish reading the line 
{
char * ps = sLine;
int ka = 0;

	while (ka++ < MAX_TAB_INPUT) {
		fin.get(*ps);
		if (('\n' == *ps) || (fin.eof())) {
			*ps = '\0';  // normal exit point 
			return;
		}
		++ps;
	}
	*(ps-1) = '\0';  // terminate string so it can be displayed
	throw LINE_LONG;

} // getsLine

void ProcessorClass:: getsLineError(int err)
// processes errors returned from getsLine
{
	if (LINE_LONG == err) ShowFatalError("Input line too long in text file; check the file format\nLast line:\n",sLine,shError31);
	else ShowFatalError("Indeterminant error occurred reading input file\nLast line:\n",sLine,shError31); // shouldn't hit this...
} // getsLineError

void ProcessorClass:: addDictionary(char *sfilename, wordtype type)
// add dictionary to the dictList
{
	if (nextDict) {														// continue an existing list 
		nextDict->next = new(dictListStruct);
		nextDict = nextDict->next;
		strcpy(nextDict->fileName,sfilename);
		nextDict->next = NULL;
	}
	else {
		strcpy(dictFileList.fileName,sfilename);  // start the list here
		dictFileList.next = NULL;
		nextDict = &dictFileList;
	}
	curDict++;
	nextDict->dictType     = type;
	nextDict->index        = curDict;
//	Processor.fprob << "PC:cD M1 " << nextDict->fileName << " index " << curDict << endl;  // *** debug
	nextDict->isFlat       = true;
	nextDict->hasNounList  = false;
	nextDict->hasAdjctList = false;
	nextDict->tailLoc			 = -1;  // location and number of comments at end of file
	nextDict->tailLen			 = 0;
	nextDict->nChanges     = 0;
} // addDictionary

void ProcessorClass:: validateDate(char *ps)
// checks that string beginning at ps is more or less valid as a date
// Note that this does not look past the first 6 or 8 chars --putDate is sometimes called with a longer
// string, so extra characters will just be ignored
// Added: vers 7.5
{
char * end; // required for strtol
char year[5], month[3], day[3];
long lval;

    // Year
    if (TabariFlags.f4DigitYear) {
        strncpy(year,ps,4);  // year
        year[4] = '\0';
        ps += 4;
    } 
    else {
        strncpy(year,ps,2);
        year[2] = '\0';
        ps += 2;
    }
    lval = strtol(year, &end, 10);
    if (year == end) throw DATE_INVLD;
    if (TabariFlags.f4DigitYear) {
        if ((lval < 1850) || (lval>2030)) throw YEAR_OOR;  // ### this will eventually need updating...
    } 

     // Month
    strncpy(month,ps,2);
    month[2] = '\0';
    lval = strtol(month, &end, 10);
    if (month == end)  throw DATE_INVLD;
    if ((lval < 1) || (lval>12))  throw MONTH_OOR;

     // Day
    strncpy(day,ps+2,2);
    day[2] = '\0';
	lval = strtol(day, &end, 10);
	if (day == end)  throw DATE_INVLD;
    if ((lval < 1) || (lval>31)) throw DAY_OOR;  // not strictly accurate -- allows 970231 for example -- but close enough to avoid problems

} // validDate

void ProcessorClass:: readProject(char *sfilename)
// reads the project file
{
instring sInfo;
char *pst;
bool skiprun = false;

	fin.open(sfilename,ios::in); 
	if (fin.fail()) ShowFatalError("Unable to open the project file ",sfilename,shError03);

	strcpy(projectFile, sfilename);
	try {
	while (!fin.eof()) {	 // primary input loop
		while ((!fin.eof()) && ( '<' != sLine[0])) {
			getsLine();
			TrimBlanks(sLine);
		}

		if (('s' == tolower(sLine[1])) && ('e' == tolower(sLine[2]))) {  // skip over the <session> tags
			getsLine();		// get next line in the file
			continue;
		}
			
		pst = strchr(sLine,'>'); // find end of tag; 
		if (pst) {
			if (strchr(pst+1,'<')) Copy2Chr(sInfo,pst+1,'<');  // copy to terminating tag
			else strcpy(sInfo,pst+1);													// 	copy rest of string   
			}
		else {
			ShowWarningError("No closing '>' in tag :\n",sLine,"Continuing to read project file",shError00);
			getsLine();		// get next line in the file
			continue;
		}

		TrimBlanks(sInfo);
		if (!(*sInfo) && ('r' !=tolower(sLine[1])) ) {
			ShowWarningError("No content following tag :\n",sLine,"Continuing to read project file",shError00);
			getsLine();		// get next line in the file
			continue;
		}
		
		switch (tolower(sLine[1])) {   // process the various possible tags
			case 'a': 
				if ('c' == tolower(sLine[2])) {  // differentiate 'actor' and 'agent'
					addDictionary(sInfo, Actor);
					if (!firstActorDict) firstActorDict = curDict; 
				}
				else {
					addDictionary(sInfo, Agent);
					if (!firstAgentDict) firstAgentDict = curDict; 
				}
				break;

			case 'c': 
				strcpy(sCoder, sInfo);
				break;

			case 'd': 
				fhasDocs = true;
				break;

			case 'e': 
				if ('v' == tolower(sLine[2])) {  // differentiate 'event' and 'error'
					strcpy(eventFile, sInfo);
					TabariFlags.fwriteEvents = true;
				} 
				else strcpy(errorFile, sInfo);
				break;

			case 'f': 
				strcpy(freqFile, sInfo);
				TabariFlags.fhasFreqs = true;
				break;

			case 'i': 
				strcpy(issueFile, sInfo);
				TabariFlags.fhasIssues = true;
				break;

			case 'o': 
				strcpy(optionsFile, sInfo);
				break;

			case 'p': 
				strcpy(probFile, sInfo);
				if (strchr(sLine,'!')) fwroteProb = true; // prob file will be used for debugging output; don't delete
				break;

			case 'r': 
				skiprun = true;
				fhasRun = true;
				break;

			case 's':				{  // do system command
				if (skiprun) break;
				int ka = system(sInfo);
				if (-1 == ka) ShowWarningError("Unable to execute <system> command:\n",sInfo,shError34);
				else WriteLine("Ran command: ",sInfo);		// display last session inf
				break;
			}

			case 't': 	// store text file names in a linked list
//				strcpy(textFile, sInfo);  // ### why do this? -- just use sInfo directly... [01.11.23]
				if (nextFile) {														// continue an existing list 
					nextFile->next = new(textListStruct);
					nextFile = nextFile->next;
					strcpy(nextFile->fileName,sInfo);
					nextFile->next = NULL;
				}
				else {
					strcpy(textFileList.fileName,sInfo);  // start the list here
					textFileList.next = NULL;
					nextFile = &textFileList;
				}
				// check for XML tag
				if (strstr(sLine,sXMLField)) {
					nextFile->indexXML = ReadXML.setXMLType(sLine);
					ReadXML.SetXMLTags(nextFile->indexXML);  // reset the tag strings
				}
				else nextFile->indexXML = 0;
				break;

			case 'v': 
				addDictionary(sInfo, Verb);
				if (!firstVerbDict) firstVerbDict = curDict; 
				break;

			default:
				ShowWarningError("Unrecognized tag :\n",sLine,"Continuing to read project file",shError00);
		}
			
		getsLine();		// get next line in the file
	} // while !fin.eof
	} // try
	catch (int i) {  // most likely cause here is file in the wrong file format, so just quit
		getsLineError(i);
	} 
	fin.close();
												// verify that we've got the minimum files to work with
	if (!firstActorDict || !firstVerbDict || ('\0' == *textFileList.fileName)) ShowFatalError("At least one .actors, .verbs, and .text file is required to run program ",shError0A);;
	
	// get the skip; date, start
	MakeTimeDateStr(sStart,sInfo);		// get the start time
																		
	if ((sLine) && ('s' == sLine[1])) {						// this is actually processing the last *non-blank* line (which is okay)
		nSession = atoi(&sLine[9]);			// get last session number
		pst = strstr(&sLine[1],"<last ");	
		if (pst) {
			Copy2Chr(sInfo,pst+6,'>');
			nLast = atoi(sInfo);						// get last record coded info
		}
		else nLast = 0;

		WRITESTRING("Last coding session: ");		// display last session info
		pst = strstr(&sLine[1],"<end ");	
		if (pst) WRITESTRING(Copy2Chr(sInfo,pst+5,'>'));
		WRITESTRING(" ");
		pst = strstr(&sLine[1],"<date ");	
		if (pst) WRITESTRING(Copy2Chr(sInfo,pst+6,'>'));
		WRITESTRING("; Coder ");
		pst = strstr(&sLine[1],"<coder ");	
		if (pst) WRITESTRING(Copy2Chr(sInfo,pst+7,'>'));
		WriteLine(" "); // ####
	}
	else {
		nSession = 0;			// initialize the file
		nLast = 0;
	}
} // readProject

void ProcessorClass:: nextTextFile(char *s)
// gets the next string containing the next text file name
{
	if (nextFile) {
		strcpy(s, nextFile->fileName);
		fusingXML = (bool)nextFile->indexXML;		
		nextFile = nextFile->next;
	}
	else s[0] = '\0';
	return;
} // nextTextFile


void ProcessorClass:: writeProject(void)
// writes the project file
{
litstring sdate,stime;

	if (TabariFlags.fwriteEvents) {
		fout.close();				// close event file
		fout.clear();				// clear the eof flag (or if file already closed)
	}
	fout.open(projectFile,ios::app); 
	if (fout.fail()) ShowFatalError("Unable to reopen file ",projectFile,shError03);
	WRITEEOL();
	ResetCurs();
	WriteLine("Writing ", projectFile);
		
	MakeTimeDateStr(stime,sdate);
	TrimEndBlanks(sStart);
	TrimEndBlanks(stime);	
	fout << "<session " 	<< nSession;
	fout << "> <coder " 	<< sCoder;
	fout << "> <date "  	<< sdate;
	fout << "> <start " 	<< sStart;
	fout << "> <end "   	<< stime;
	fout << "> <records " << nRecords;
	fout << "> <actorchanges " << nActorChanges;
	fout << "> <verbchanges "  << nVerbChanges;
	fout << "> <last " 		<< nLast;
	fout << ">" << endl;
	
	fout.close();		// close project file
} // writeProject

void ProcessorClass:: writeDoc(void)
// writes the documentation header to the event file
{
litstring sdate,stime;
instring sInfo;
char *pst;

	WriteLine("Writing event file documentation...");

	// gets the first date in the first file and create id string
	nextFile = &textFileList;	// set nextFile to start of text file list
  openNextText();
  try {getsLine();}
	catch (int err) { getsLineError(err);}

	if (TabariFlags.f4DigitYear) {
		strncpy(sDoc,sLine,8);
		sDoc[8] = '\0';
	}
	else {
		strncpy(sDoc,sLine,6); 
		sDoc[6] = '\0';
	}
  strcat(sDoc,"\tDOC\tDOC\t999\t");
  fin.close();
  
	fin.open(projectFile,ios::in); 
	if (fin.fail()) ShowFatalError("writeDoc is unable to reopen project file ",projectFile,shError03);

	MakeTimeDateStr(stime,sdate);
	TrimEndBlanks(stime);	
	fout << sDoc << "Data generated by TABARI version " << sRelease << endl;
	fout << sDoc << sdate << " " << stime << endl;
	fout << sDoc << "Session " << nSession << "; Coder " << sCoder << endl;

	sLine[0] = '#'; // force read
	try {
		while (!fin.eof()) {	 // get the <dochead> lines
			while ((!fin.eof()) && ( '<' != sLine[0])) {
				getsLine();
				TrimBlanks(sLine);
			}
			if (strstr(sLine,"<sess")) break;  // no need to read <session> records
			if (strstr(sLine,"<dochead")) {
				pst = strchr(sLine,'>'); // find end of tag; 
				if (pst) strcpy(sInfo,pst+1);													// 	copy rest of string   
				else continue;
				TrimBlanks(sInfo);
				fout << sDoc << sInfo << endl;
			}
		sLine[0] = '#'; // force read
		}

		fin.close();  // ### is there a better way to rewind a file??
		fin.open(projectFile,ios::in); 
		sLine[0] = '#'; // force read
		while (!fin.eof()) {	 // copy the functional lines
			while ((!fin.eof()) && ( '<' != sLine[0])) {
				getsLine();
				TrimBlanks(sLine);
			}
			if (strstr(sLine,"<sess")) break;  // no need to read <session> records
			if (('<' == sLine[0]) && (!strstr(sLine,"<doc")))  {
				TrimBlanks(sLine);
				fout << sDoc << sLine << endl;
			}
			sLine[0] = '#'; // force read
		}
		fin.close(); 
		fin.open(projectFile,ios::in); 
		sLine[0] = '#'; // force read
		while (!fin.eof()) {	 // get the <doctail> lines
			while ((!fin.eof()) && ( '<' != sLine[0])) {
				getsLine();
				TrimBlanks(sLine);
			}
			if (strstr(sLine,"<sess")) break;  // no need to read <session> records
			if (strstr(sLine,"<doctail")) {
				pst = strchr(sLine,'>'); // find end of tag; 
				if (pst) strcpy(sInfo,pst+1);													// 	copy rest of string   
				else continue;
				TrimBlanks(sInfo);
				fout << sDoc << sInfo << endl;
			}
		sLine[0] = '#'; // force read
		}
	}
	catch(int i) {getsLineError(i);} 	
	
	fin.close();		// close project file
} // writeDoc

void ProcessorClass:: initSession(void)
// initialize coding session
{
char snum[] = "    ";

	++nSession;								// increment session number
	nRecords = 0;
	Int2Str(snum,nSession);		// get suffix for output files <08.12.30> ### use a C function here?
	WriteLine("Coding session ",snum);

#if TAB_DEBUG
	strcpy(sCoder,"debug");
#else
	if (!sCoder[0]) {   // may have already been set by <coder> in .project filestring
		GetInput(sCoder,"Enter coder ID");
		if (!sCoder[0]) strcpy(sCoder,"***");
	}
#endif

	if (TabariFlags.fwriteEvents) {
		strcat(eventFile,".");
		strcat(eventFile,snum);
		fout.open(eventFile,ios::out); 
		if (fout.fail()) ShowFatalError("Unable to open event file ",eventFile,shError03);
		if (fhasDocs) writeDoc();
	}	

	if (*probFile) {
		if (fwroteProb) strcat(probFile,".debug");  // debugging option
		else {
			strcat(probFile,".");
			strcat(probFile,snum);
		}
//		WriteLine("Open Probfile: ",probFile); // *** debug
		fprob.open(probFile,ios::out); 
		if (fprob.fail()) ShowFatalError("Unable to open problems file ",probFile,shError03);
	}
		
	ReadFiles.readOptions(optionsFile);

	nextDict = &dictFileList;  // read various dictionaries
	while (nextDict) {
		curDict = nextDict->index;
		if (Actor == nextDict->dictType) ReadFiles.readActors();
		else if (Agent == nextDict->dictType) ReadFiles.readAgents(nextDict->fileName);
		else if (Verb == nextDict->dictType)  ReadFiles.readVerbs(nextDict->fileName);
		nextDict = nextDict->next;
	}

	if (*issueFile) ReadFiles.readIssues(issueFile);

	if (*freqFile)  {
		ReadFiles.readFreqs(freqFile);
		if (fhasDocs) { // add frequency identifiers to documentation
			freqStruct * pHead = &Coder.freqHead; // current issue header
			fout << sDoc << "\tFREQUENCIES:";  // write full descriptors
			while (pHead) {		// traverse list
				if (pHead->abbrev) 			fout <<  '\t' << pHead->phrase;
				else fout << "\t---";
				pHead = pHead->pnext;
			}
			fout << endl;
			pHead = &Coder.freqHead; // current issue header
			fout << sDoc << "\tFREQS:";   // write abbrevs
			while (pHead) {		// traverse list
				if (pHead->abbrev) 			fout <<  '\t' << pHead->abbrev;
				else fout << "\t -- ";
				pHead = pHead->pnext;
			}
			fout << endl;
		}
	}
	nextFile = &textFileList;	// set nextFile to start of text file list
	
#if !(TAB_DEBUG)
	if ((nLast > 0) && (-1 == TabariFlags.fAutoCodemode)  && (TabariFlags.fCheckSkip))  {
		instring sprompt;
		sprintf(sprompt,"Skip the %ld records coded in previous sessions",nLast);
		if (GetAnswer(sprompt, 'Y','N')) skipRecords();
		else {
			nLast = 0;
			openNextText();
		}
	}
	else openNextText();
	
#else
	openNextText();
#endif

} //initSession

bool ProcessorClass:: openNextText(void)
// opens the next text file; returns false when list is finished
{
	nextTextFile(textFile);
	while (textFile[0]) {
		if (fusingXML) {
			if (ReadXML.openXMLFile(textFile)) break;
		} else {
			fin.clear(); //  clear the eof flag from previous file operations
			fin.open(textFile,ios::in); 
			if (!fin.fail()) break;  // everything is okay, so drop out of loop
			ShowWarningError("Unable to open the text file ",textFile,"File will be skipped",shError08);
	  	fin.clear();  // clear the error 
		}
		nextTextFile(textFile);  // last file failed, so try next one
	}
	if (textFile[0]) return true; // we opened a file, so return true
	else return false;            // none of the files could be opened
} // openNextText

void ProcessorClass:: closeText(void)
// closes the text file
{
	if (fusingXML) ReadXML.closeXMLFile();
	else fin.close();
} // closeText

void ProcessorClass:: openError(void)
// opens the error file
{
litstring sdate, stime;

	ferror.open(errorFile,ios::out); 
	if (ferror.fail()) ShowFatalError("Unable to open error file ",errorFile,shError03);
	fopenError = true;
	MakeTimeDateStr(stime,sdate);
	TrimEndBlanks(stime);	
	ferror << "TABARI Error file\nData generated by TABARI version " << sRelease << endl;
	ferror << sdate << " " << stime << endl;
	ferror << "Session " << nSession << "; Coder " << sCoder << endl;
} // openError

void ProcessorClass:: writeError(const char *s, char *s1, int ierror)
// writes to the error file. An error of a specific type is only written once per record,
// and only a total of MAX_ERROR errors are written for a record. 
{
char * pst = sentText;
int ka = 0;

	if (!(*errorFile)) return;  // .project did not request an <errorfile> 
	if (!fopenError) openError();
	
	while ((ka < MAX_ERROR) && (ierror != didError[ka]) && (didError[ka]>0)) {
//	ferror << "ierror1: " << ierror << "  " << ka << "  " << didError[ka] <<  endl;
		++ka; // check whether this error has already been reported
	}
	
	if (ierror == didError[ka]) {
//		ferror << "ierror3: " << ierror << "  " << ka << "  " << didError[ka] <<  endl;
		return; // we already printed this one
	}
	if (ka >= MAX_ERROR) return;     // printed all of the errors we are going to print for this case...
//	ferror << "ierror2: " << ierror << "  " << ka << "  " << didError[ka] <<  endl;
	didError[ka] = ierror;  // record this type of error
	
	ferror << "\nInput record: " << sRecEngDate << "  "<< sRecID <<  endl;
	ferror << "Input text: ";
	while (*pst) ferror << *pst++;
	ferror << endl;
//	ferror << "ierror: " << ierror << endl;
	ferror << s << s1 << endl;
	ferror.flush(); // since this could generate a crash real soon...
} // writeError

void ProcessorClass:: closeError(void)
// closes the error file
{
litstring sdate, stime;

	WriteLine("Coder/Parser errors reported in ",errorFile);
	MakeTimeDateStr(stime,sdate);
	ferror << "\nFile closed : "  << sdate << " " << stime << endl;
	ferror.close();
} // closeText

void ProcessorClass:: changeDict(int iDict, wordtype wtype)
// sets curDict; increments changes count
{
dictListStruct *dictFile = &Processor.dictFileList;
	if (iDict) curDict = iDict;    // this is used in initRoot
	else {
		if (Verb  == wtype)	curDict = firstVerbDict;
		else if (Agent == wtype) curDict = firstAgentDict;
		else								curDict = firstActorDict;
	}
	
	while (dictFile) {  // increment nChange
		if (curDict == dictFile->index) {
			dictFile->nChanges++;
			if      (Noun == wtype) 	dictFile->hasNounList = true;
			else if (Adjctv == wtype) dictFile->hasAdjctList = true;
			break;
		}
		else dictFile = dictFile->next;
	} // while
	if (!dictFile) ShowFatalError("Unrecoverable dictionary index error in ProcessorClass:changeDict",shError98);

	fchangeDict = true;
	
	if (Verb == dictFile->dictType) ++nVerbChanges;  // summary used in the project file
	else 	++nActorChanges;  // this is now both actors and agents
} // changeDict

void ProcessorClass:: writeUsage(void)
// writes .use files
{
	WriteLine(""); // \n
	if (TabariFlags.fActorUsage) {
		if (TRUE_3WAY == TabariFlags.fActorUsage)	WriteFiles.writeActorUsage(true);
	}
	else if (GetAnswer((char *)"Write use report for ", (char *)"actors"))	WriteFiles.writeActorUsage(true);

	if (TabariFlags.fAgentUsage) {
		if (TRUE_3WAY == TabariFlags.fAgentUsage)	WriteFiles.writeActorUsage(false);
	}
	else if (GetAnswer((char *)"Write use report for ", (char *)"agents"))	WriteFiles.writeActorUsage(false);

 	if (TabariFlags.fVerbUsage) {
		if (TRUE_3WAY == TabariFlags.fVerbUsage)	WriteFiles.writeVerbUsage();
	}
	else if (GetAnswer((char *)"Write use report for ", (char *)"verbs"))	WriteFiles.writeVerbUsage();
} // writeUsage

void ProcessorClass:: backFiles(void)
// writes .back files
{
filestring start, sdate;
dictListStruct *dictFile = &Processor.dictFileList;	

	if (!fchangeDict){	
		WriteLine("\a\nNo changes have been made; backup skipped");
		return;
	}
		
	MakeTimeDateStr(start, sdate);
	strcpy(scommNew,sCoder);
	strcat(scommNew,&sdate[3]);  //skip the day-of-week 
	WriteLine("");
		
	while (dictFile) {
		if (dictFile->nChanges) { 
			if (GetAnswer((char *)"Save a backup for ", dictFile->fileName)){
				strcpy(start,dictFile->fileName);  // temporarily rename the existing file
				strcat(start,".temp");
				if (rename(dictFile->fileName,start)) ShowWarningError ("Adding '.temp' to ", dictFile->fileName, "Existing file will be overwritten",shError09);

				// write the current files
				if ((dictFile->dictType == Actor) || (dictFile->dictType == Agent)) WriteFiles.writeActors(dictFile);
				else WriteFiles.writeVerbs(dictFile);		

				strcpy(sdate,dictFile->fileName);  // rename the file we just wrote with .back
				strcat(sdate,".back");
				if (rename(dictFile->fileName,sdate)) ShowWarningError ("Adding '.back' to ", dictFile->fileName, "Existing file will be overwritten",shError09);

				 // restore the name of the original file
				if (rename(start, dictFile->fileName)) ShowWarningError ("Restoring the original name of ", dictFile->fileName, "Original file still has .temp suffix",shError09);

			} // GetAnswer
		} // if nChange
		dictFile = dictFile->next;
	} // while
	
} // backFiles

void ProcessorClass:: doFinalSysCalls(void)	
// handles the <system> calls following <run>
{
instring sInfo;
char *pst;
bool dorun = false;

	fin.open(projectFile,ios::in); 
	if (fin.fail()) ShowFatalError("Unable to open the project file ",projectFile,shError03);

	while (!fin.eof()) {	 // primary input loop
		while ((!fin.eof()) && ( '<' != sLine[0])) {
			getsLine();
		}

		if (('s' == tolower(sLine[1])) && ('e' == tolower(sLine[2]))) { break;} // terminate when we hit <session> tags
			
		if (('s' != tolower(sLine[1])) && ('r' != tolower(sLine[1]))) {  // skip over everything except <system> and <run>
			getsLine();		// get next line in the file
			continue;
		}

		TrimBlanks(sLine);
		pst = strchr(sLine,'>'); // find end of tag; 
		if (pst) {
			if (strchr(pst+1,'<')) Copy2Chr(sInfo,pst+1,'<');  // copy to terminating tag
			else strcpy(sInfo,pst+1);													// 	copy rest of string 
			while ((pst = strstr(sInfo, "$SESSN"))) { // replace $SESSN strings with session number
				char sess[5];
				instring sNew;
				char * pnew = sNew;
				char * psr = sInfo;
				sprintf(sess,"%d",nSession);  // convert session number to string
//				WriteLine("sInfo:",sInfo);
//				WriteLine("sess:",sess);
				while (psr < pst) *pnew++ = *psr++;  // copy chars before match
				*pnew = '\0';
//				WriteLine("sNew1:",sNew);
				strcat(sNew,sess); // add session num string
//				WriteLine("sNew2",sNew);
				strcat(sNew,pst+6); // copy everything else
//				WriteLine("sNew3",sNew);
				strcpy(sInfo,sNew);
//				WriteLine("Ran command: ",sInfo);
			}
		}			
		else {
			ShowWarningError("No closing '>' in tag :\n",sLine,"Continuing to read project file",shError00);
			getsLine();		// get next line in the file
			continue;
		}

		TrimBlanks(sInfo);
		
		if ('s' == tolower(sLine[1])) {   // do system command
			if (dorun) { 
				int ka = system(sInfo);
				if (-1 == ka) ShowWarningError("Unable to execute <system> command:\n",sInfo,shError34);
				else WriteLine("Ran command: ",sInfo);		// display last session inf
			}
		}
		else dorun = true;
			
		getsLine();		// get next line in the file
	} // while !fin.eof
	fin.close();
} // doFinalSysCalls

void ProcessorClass:: endSession(void)
// end the coding session
{

 ClearScreen();
 //	if (!TabariFlags.fActorUsage || !TabariFlags.fVerbUsage) ClearScreen();  
	WriteLine("Q)uit entered in menu; finishing TABARI session");
	fin.close();
	fin.clear();				// clear the eof flag (or if file already closed)

	if (*probFile) {
//		WriteLine("Probfile: ",probFile); // *** debug
		fprob.close();
		if (fprob.fail()) ShowWarningError("Unable to close problems file ",probFile,shError03);
		if (!fwroteProb) remove(probFile);  // get rid of problems file if we haven't used it
	}
	
	if (fopenError) closeError();
	
	writeProject();			// Note: the event file is closed in this function
		
#if TAB_DEBUG
	remove(eventFile);
	exit(0);
#endif

	if (fchangeDict){		
		filestring start;
		filestring sdate;
		dictListStruct *dictFile = &Processor.dictFileList;	
		bool freply = false;

//		WriteLine("PC:eS Mk1"); // *** debug
		MakeTimeDateStr(start, sdate);
		strcpy(scommNew,sCoder);
		strcat(scommNew,&sdate[3]);  //skip the day-of-week 
			
		while (dictFile) {
//					WriteLine("PC:eS Mk2 ", dictFile->fileName); // *** debug
			if (dictFile->nChanges) { 
//						WriteLine("PC:eS Mk3 ", dictFile->fileName);  // *** debug
				freply = GetAnswer((char *)"Save changes to ", dictFile->fileName);
				if (!freply) {
					char s[48];
					sprintf(s, "You are about to discard %d changes;", dictFile->nChanges); // ### update this for the individual dictionary counts?
					freply = !GetAnswer(s,(char *)" do you *really* want to do this");
				}
				if (freply) {
					strcpy(start,dictFile->fileName);
					strcat(start,".old");
					remove(start);
					if (rename(dictFile->fileName,start)) ShowWarningError ("Adding '.old' to ", dictFile->fileName, "Existing file will be overwritten",shError00);
					if (Verb == dictFile->dictType) WriteFiles.writeVerbs(dictFile);	
					else														WriteFiles.writeActors(dictFile);  // actors and agents 
				}
			} // if nChange
			dictFile = dictFile->next;
		} // while
	} // if
		
#if false  // deactivated in version 0.8+
	if (pTimeModList) {
		WriteLine("Sorting time-shifted events");
		sortTimeOutput();
	}
#endif
	
	if (fhasRun) doFinalSysCalls();

	if (TabariFlags.fAutoCodemode < 0) {
		ResetCurs();
		WriteNewLine("Press any key to end TABARI program");
		getUpper();
	}
	exit(0);
} // endSession

void ProcessorClass:: doOther(void)
// handles the O)ther option
// currently returns to the main menu after selection
{
int imenu;
	imenu = OtherMenu((bool) *probFile);
	switch (imenu) {
		case 1: backFiles();
						break;
		case 2: writeProblem();
						break;
		case 3: showMemory();
						setupMenu(false);
						break;
		case 4: writeUsage();
						break;
		case 5: doValidation();  // this option is invisible to the user
						break;
		case 6: Parser.showTags();  // this option is invisible to the user
						break;
	}
} // doOther

void ProcessorClass:: reverseRecord(int irev)
// reverses the read
// value of irev is currently ignored
{

		--irecback;
		if (irecback < 0) irecback = MAX_REC_BACK-1;
		if (backrec[irecback] >= 0) {
			ClearScreen();
			WriteLine("Reading previous record\n");
			readRecord(false);
			showRecord();
		}
		else {
			ShowWarningError("Already at beginning of file","R)ecode or select N)ext record",shError22);
			++irecback;
		}
} // reverseRecord

void ProcessorClass:: skipRecords(void)
// skips nLast records
{
int ka = 0;
char sout[32];

	if (fusingXML) {
		ReadXML.skipXMLRecords();
		return;
	}	

	sprintf(sout, "\nSkipping %ld records",nLast);
	WriteLine(sout);
	try {
		while (openNextText()) {
			while ((ka<=nLast) && !fin.eof()) {
				while ((!fin.eof()) && (sLine[0])) getsLine();
				 getsLine();
				 ++ka;
			}
			if (ka > nLast) return;	
			closeText();
		}
	}
	catch(int i) {getsLineError(i);} 	
	// if we get here, there was a problem
	ShowFatalError("Text file(s) ended before all records could be skipped.\nCheck the <last> field in final session in .project file",shError02);
} // skipRecords 

void ProcessorClass:: readRecord(bool forward)
// reads text record: gets date, gets record ID, and moves the text into sentText[]
// can throw INPUT_EOF and STORY_LONG
// if (forward), get next record, save new backrec; otherwise seekg(backrec[]);
{
char *pst;
char sSeqNo[MAX_RECID_LEN];		// sequence number
int endDate = 0;
int len = 0;

	if (forward) {
		++irecback;
		if (irecback >= MAX_REC_BACK) irecback = 0;  // treat backrec as a circular array
		backrec[irecback] = fin.tellg();
	}
	else fin.seekg(backrec[irecback]);
	
  try {
		while ((!fin.eof()) && (!isdigit(sLine[0]))) getsLine(); // this can throw LINE_LONG
		if (fin.eof()) throw INPUT_EOF;  // normal exit
	}
	catch (int err) {
		if (INPUT_EOF == err) throw INPUT_EOF;   // INPUT_EOF is caught in readNextRecord
		else getsLineError(err);
	} 	
	++nLast;			// increment the number of records coded ( ### isn't this redundant with nRecords? 01.11.10)

	if (TabariFlags.f4DigitYear) {
		for (int ka=0; ka<8; ka++) sRecDate[ka] = sLine[ka];
        sRecDate[8] = '\0';
        endDate = 8;
	}
	else  {
		for (int ka=0; ka<6; ka++) sRecDate[ka] = sLine[ka];
        sRecDate[6] = '\0';
        endDate = 6;
	}

    validateDate(sRecDate);
	
	makeEnglishDate();
    
	if (ReadFiles.iIDInfoLoc >= 0) {  								// process the FORWARD fields
	// Note that the story ID is not actually stored separately, but is just handled by a
	// loc and len reference in the full record ID.  This allows it to be any length
		len = (int)strlen(sLine);
		Parser.fcanForward = false;
		if (len > endDate+ReadFiles.iIDInfoLoc) {
			strncpy(sRecID,&sLine[endDate+ReadFiles.iIDInfoLoc],ReadFiles.iIDInfoLen);
			sRecID[ReadFiles.iIDInfoLen] = '\0';  // strncpy doesn't automatically null-terminate		
			if (strncmp(&slastRecID[ReadFiles.iStoryLoc], &sRecID[ReadFiles.iStoryLoc], ReadFiles.iStoryLen)) { // new story ID
				lastSeq = -1;			// signal new story
			}
			strcpy(slastRecID, sRecID);  		// save current RecID
			strncpy(sSeqNo,&sLine[endDate+ReadFiles.iSentLoc],ReadFiles.iSentLen);	// get the sequence number	
			sSeqNo[ReadFiles.iSentLen] = '\0';
			nSeq = atoi(sSeqNo);
			if (Parser.fuseForward && (lastSeq >= 0) && (nSeq == lastSeq+1)) Parser.fcanForward = true;
			lastSeq = nSeq;

			if (ReadFiles.fhasDoc) {  // get document ID
				int kb = 0;
				char * 	psa = sDocID; 
				pst = &sLine[endDate + ReadFiles.iSentLoc+ReadFiles.iSentLen]; // move past sentence ID 
				while ((*pst) && (isspace(*pst))) ++pst; // move to first non-blank char
				while (*pst) {
					if (++kb > MAX_DOCID_LEN) break;  // check for overflow; could throw a specific error here
					*psa++ = *pst++;
				}
				*psa = '\0';
			}
		}
		else {   // story ID doesn't match FORWARD format
			strncpy(sRecID,&sLine[endDate],MAX_RECID_LEN); 
			lastSeq = -1;
			nSeq = 0;
		}
	} 
	else { // default: just get the rest of the line, skipping white space
		pst = &sLine[endDate];
		while ((*pst) && (isspace(*pst))) ++pst;
		strncpy(sRecID,pst,MAX_RECID_LEN);
	}		

	try {
		int ka = 0;   // array bounds check for sentText[]
		pst = sentText;										// get text, put in sentText[]
		getsLine();
		while ((!fin.eof()) && (*sLine)) {
			char *psa = sLine;
			while (*psa) {
				if (++ka >= MAX_SENTENCE) throw STORY_LONG;
				*pst++ = *psa++;
			}
			if (*(pst-1) != ' ') {
				if (ka < MAX_SENTENCE) {
					*pst++ = ' ';	// blank terminate
					++ka;
				}
				else throw STORY_LONG;
			}
			getsLine();
		}
	}
	catch (int err) {
		if (STORY_LONG == err) throw STORY_LONG; // this will be caught by readNextRecord and story truncated
		else getsLineError(err);}

	*pst = '\0';
		
	++nRecords;
	
	for (int ka=0; ka<MAX_ERROR; ka++) {didError[ka]=0;}  // initialize didError array -- see writeError()
		
} // readRecord

void ProcessorClass:: skipRemainder(void)	
// skip remainder of a long story
{
	try {
		while ((!fin.eof()) && (sLine[0])) getsLine();
			getsLine();
	}
	catch (int err) { getsLineError(err); }
} // skipRemainder

void ProcessorClass:: makeEnglishDate(void)
// Converts sRecDate to an English form; also sets julianDate
// moves the text into sentText[]
{
int ka;
char *	pst;

	julianDate = MakeJulian(sRecDate);
																// make English version of date
																		// get date
//	TabariFlags.f4DigitYear = false;  // *** testing
//	strcpy(sLine,"011012IDF");

	if (TabariFlags.f4DigitYear) pst = &sRecDate[4];
	else pst = &sRecDate[2];

	sRecEngDate[0] = *(pst+2);
	sRecEngDate[1] = *(pst+3);
	ka = (*pst) * 10 + *(pst+1) - 528;
	if ((ka < 0) || (ka > 12)) ka = 0;
	ka *=3;
	sRecEngDate[3] = sMonths[ka];
	sRecEngDate[4] = sMonths[ka+1];
	sRecEngDate[5] = sMonths[ka+2];
	if (TabariFlags.f4DigitYear) {
		for (ka=0; ka<4; ka++) sRecEngDate[ka+7] = sRecDate[ka];
		sRecEngDate[11] = '\0'; 
	}
	else  {
		sRecEngDate[7] = sRecDate[0];
		sRecEngDate[8] = sRecDate[1];
		sRecEngDate[9] = '\0';
	}
//	cout << sRecDate << " = " << sRecEngDate << endl; // *** testing
	
} // makeEnglishDate

void ProcessorClass:: showDate(void)	
// display only date, ID and records coded: used in autocode
{
char sout[32];

	move(1,0);
	clrtoeol();
	addstr("Date: ");
	addstr(sRecEngDate);
	addstr( "  Record ID: ");
	addstr(sRecID);
	sprintf(sout,"  Texts: %ld", nRecords);
	addstr(sout);
	sprintf(sout,"  Events: %ld", totalEvents);
	addstr(sout);
	refresh();
} // showDate

void ProcessorClass:: showRecord(void)
// display the record on the screen with date and ID.  Puts line breaks at
// spaces and recognizes "\n".
// Note that if \n occurs at the end of a line, an extra blank is inserted at the 
// beginning of the new line because blanks are always added (if needed) to the end 
// of input lines.  So, counter-intuitively put the \n at the beginning of lines to 
// avoid this
{
char * pst = sentText;
char * pold;
int nline = 0;

	ClearScreen();
	refresh();
	
	addstr("Date: ");
	addstr(sRecEngDate);
	if (ReadFiles.fhasDoc) {
		nline = (int)(23 - strlen(sDocID));
		if (nline > 0) while (--nline) addch(' ');
		addstr(" Record : ");
		addstr(sRecID);
		addch(' ');
		addstr(sDocID);
		addch('\n');
	} 
	else {
		addstr("                       Record : ");
		addstr(sRecID);
		addch('\n');
	}
	addch('\n');
	
	while (*pst) {
		pold = pst;
		while (!isspace(*pst)) {
			if (('\\' == *pst) && ('n' == *(pst+1))) {  // deal with \n
				while (pold < pst) addch(*pold++);  // output the line
				addch('\n');
				nline = 0;		// reset for a new line
				pst += 2;
				pold = pst;
			}
			else ++pst;
		}
		if (nline + (pst - pold) >= WINDOW_WIDTH) {  // go to next line
			addch('\n');
			nline = 0;		// reset for a new line
		}
		nline += (pst - pold) + 1;
		while (pold <= pst) addch(*pold++);  // output the line
		++pst;
	}
	addstr("\n\n");
	ResetCurs();

} // showRecord


void ProcessorClass:: showMemory(void)
// shows the current memory use
{
char sout[128];
	ClearScreen();
	WRITESTRING("\n ----- MEMORY STATUS -----\n\n");
	WRITESTRING("Dictionary Memory used:\n");
	sprintf(sout,"Text      %7ld  (%7ld maximum)\n", (long)(CharStore.pCharLast - CharStore.charBuf), (long)MAX_CHAR);
	WRITESTRING(sout);
	sprintf(sout,"Tokens    %7d  (%7d maximum)\n",TokenStore.iToken,MAX_TOKENS);
	WRITESTRING(sout);
	sprintf(sout,"Codes     %7d  (%7d maximum)\n",CodeStore.iCode,MAX_CODES);
	WRITESTRING(sout);
	sprintf(sout,"Phrases   %7d  (%7d maximum)\n",Phrases.iPhrase,MAX_PHRASES);
	WRITESTRING(sout);
	sprintf(sout,"Literals  %7d  (%7d maximum)\n",Literals.iLit,MAX_LITERALS);
	WRITESTRING(sout);
	sprintf(sout,"Roots     %7d  (%7d maximum)\n",Roots.iRoot,MAX_ROOTS );
	WRITESTRING(sout);
	sprintf(sout,"Patterns  %7d  (%7d maximum)\n",Roots.iPattern, MAX_PATTERN);
	WRITESTRING(sout);
	WRITESTRING("\nSentence Memory used:\n");
	sprintf(sout,"Words        %4d  (%4d maximum)\n",Parser.iLex,MAX_LEX);
	WRITESTRING(sout);
	sprintf(sout,"Lexical Tags %4d  (%4d maximum)\n",Parser.iTag,MAX_TAGS);
	WRITESTRING(sout);
	sprintf(sout,"Matched Lit. %4d  (%4d maximum)\n",Parser.iLitr,MAX_LITRLIST);
	WRITESTRING(sout);
	WRITESTRING("\nEvent Memory used:\n");
	sprintf(sout,"Events        %3d  (%4d maximum)\n",Coder.nEvents + 1,MAX_EVENTS);
	WRITESTRING(sout);
	ResetCurs();
} // showMemory

void ProcessorClass:: showFreqs(void)	
// display the freqs for the record. 
// ### note that this is currently configured for a maximum of 24 4-char
//     abbrevs on a 120-wide string; just add an outer loop to get more
//     displayed
{	
freqStruct * pHead = &Coder.freqHead; // current issue header
int ka;
char s[4];

	WRITEEOL();
	while (pHead) {		// traverse list
		if (pHead->abbrev) WRITESTRING(pHead->abbrev);
		else WRITESTRING(" -- ");
		WRITECHAR(' ');
		pHead = pHead->pnext;
	}
	WRITEEOL();
	for (ka = 0; ka < Coder.iFreq; ++ka) {
		sprintf(s,"%3d  ",Literals.kountFreq[ka]);
		WRITESTRING(s);
	}
	WRITEEOL();
} // showFreqs

void ProcessorClass:: showEvents(void)	
// display the coded events, and write to event file if this is enabled
{	
int ka;
instring s;

	if  (Coder.fhasProblem) WriteLine(Coder.sProblem);
	else {
		WriteLine("Coded events:");		
		for (ka=0; ka<=Coder.nEvents; ++ka) {
			if ((!ka) && (TabariFlags.fNonEvents) && (!Coder.eventArray[0].verbroot)) Coder.getNonEventString(s);
			else Coder.getEventString(s,ka);
			WRITESTRING(s);
			if (TabariFlags.fhasIssues) WriteLine("\t",Coder.sIssueCodes);
			else WriteLine(""); // simple \n plus refresh
			if (TabariFlags.cursr > WINDOW_HEIGHT - 6) {
				WriteLine("\nToo many events to display; all events were written to event file");
				break;
			}				
						
		}
		if (TabariFlags.fhasFreqs) showFreqs();
		if (TabariFlags.fwriteEvents) writeEvents();
		else totalEvents += Coder.nEvents + 1;  // increment here only if not writing
	}
} // showEvents

void ProcessorClass:: writeIDs(int loc, int len)	
// write ID strings to event file fout
// '*' is written for non-printing chars; this will handle incorrectly 
// formatted ID strings
{
int ka = 0;
char *ps = &sRecID[loc - ReadFiles.iIDInfoLoc];

    fout.put('\t');
	while(ka < len) {
		if (isprint(*ps)) fout.put(*ps); 
		else  fout.put('*');
		++ps;
		++ka;
	}
} // writeIDs

void ProcessorClass:: writeProbIDs(int loc, int len)	
// write ID strings to problem file fprob
// ### NOTE: Presumably there is a simple way to do this as a variation of 
// ### ProcessorClass:: writeIDs that has the output stream passed as a
// ### parameter, but couldn't get this to compile at the moment 
// '*' is written for non-printing chars; this will handle incorrectly 
// formatted ID strings
{
int ka = 0;
char *ps = &sRecID[loc - ReadFiles.iIDInfoLoc];

	fprob.put('\t');
	while(ka < len) {
		if (isprint(*ps)) fout.put(*ps); 
		else  fprob.put('*');
		++ps;
		++ka;
	}
} // writeIDs

void ProcessorClass:: writeXMLIDs(void)	
// write ID strings to event file fout for XML files
// Differences: 
//		1. storyID is the same as  sRecID (typically only one is used)
//		2. the integer seqNo is used rather than copying the string
{
	if (ReadFiles.fOutputID) fout << '\t' << sRecID;
	if (ReadFiles.fOutputStory) fout << '\t' << sRecID;
	if (ReadFiles.fOutputSeq) fout << '\t' << sRecID;
} // writeXMLIDs


void ProcessorClass:: writeEvents(void)	
// writes coded events to event file fout
{
int ka;
instring s;

	if (Coder.fhasProblem || !TabariFlags.fwriteEvents) return;
//	if (fisShifted) addTimeModList();  // deactivated version 0.8+

	totalEvents += Coder.nEvents + 1;
	for (ka=0; ka<=Coder.nEvents; ++ka) {
		if ((!ka) && (TabariFlags.fNonEvents) && (!Coder.eventStrgs[0].verbroot)) fout << Coder.getNonEventString(s);
		else if (2 == TabariFlags.fNonEvents) return; // NONEVENTS: ONLY
		else {
			fout << Coder.getEventString(s,ka);	
			if (ReadFiles.fOutputPats) fout << Coder.getPatterns(s,ka);  // add matching patterns
		}
		
		if (fusingXML) writeXMLIDs();
		else
		if ((ReadFiles.iIDInfoLoc >= 0) ||  // ID was specified using FORWARD
		     ReadFiles.fOutputDoc) { 
			if (ReadFiles.fOutputID) writeIDs(ReadFiles.iIDInfoLoc, ReadFiles.iIDInfoLen);
			if (ReadFiles.fOutputStory) writeIDs(ReadFiles.iStoryLoc, ReadFiles.iStoryLen);
			if (ReadFiles.fOutputSeq) writeIDs(ReadFiles.iSentLoc, ReadFiles.iSentLen);
			if (ReadFiles.fOutputDoc) {
				fout.put('\t');
				fout << sDocID;
			}
		}
		else if (ReadFiles.fOutputID) fout <<'\t'<< sRecID;  // just use default ID
		
//		fout << " issues " << TabariFlags.fhasIssues << "freqs " << TabariFlags.fhasFreqs << endl; // debug
		if (TabariFlags.fhasIssues) fout << '\t' << Coder.sIssueCodes;
		if (TabariFlags.fhasFreqs) {
			for (ka = 0; ka< Coder.iFreq; ++ka) fout << '\t' <<  Literals.kountFreq[ka];
		}

		if (ReadFiles.fOutputText) {
			char * pst = sentText;
			fout.put('\t');
			while (*pst) fout.put(*pst++);
		}

		fout.put('\n');
	}
} // writeEvents

void ProcessorClass:: writeProblem(void)	
// writes information to the problem file
// ### currently doesn't write XML IDs
{
int nline = 0;
instring s;
char *ps = s;
char * pold;

	fwroteProb = true;   // wrote to the problems file; don't delete it

	fprob << "Record identification: ";
	if (ReadFiles.iIDInfoLoc >= 0) { // ID was specified using FORWARD
		if (ReadFiles.fOutputID) writeProbIDs(ReadFiles.iIDInfoLoc, ReadFiles.iIDInfoLen);
		if (ReadFiles.fOutputStory) writeProbIDs(ReadFiles.iStoryLoc, ReadFiles.iStoryLen);
		if (ReadFiles.fOutputSeq) writeProbIDs(ReadFiles.iSentLoc, ReadFiles.iSentLen);
	}
	else if (ReadFiles.fOutputID) fprob <<'\t'<< sRecID;  // just use default ID

	if (ReadFiles.fhasDoc) fprob << "\nDocument ID: " << sDocID;
	fprob << "\nCoder: " << sCoder;
	MakeTimeDateStr (s, &s[32]);  
	fprob << "\nTime : " << s << "  " << &s[32];
	fprob << "\n\n--- Problem Description ---\n";
	
	WriteLine("Describe the problem; enter a blank line when finished");
	do {
		WRITESTRING("> ");
		if (!safeGetLine(s)) WriteLine("Line has been written to problems file");
		fprob << s << endl;
	} while ('\0' != s[0]);
	
	fprob << "--- Original Text ---\n";
	ps = sentText;
	while (*ps) {
		pold = ps;
		while (!isspace(*ps)) {
			if (('\\' == *ps) && ('n' == *(ps+1))) {  // deal with \n
				while (pold < ps) fprob << *pold++;  // output the line
				fprob << endl;
				nline = 0;		// reset for a new line
				ps += 2;
				pold = ps;
			}
			else ++ps;
		}
		if (nline + (ps - pold) >= WINDOW_WIDTH) {  // go to next line
			fprob << endl;
			nline = 0;		// reset for a new line
		}
		nline += (ps - pold) + 1;
		while (pold <= ps) fprob << *pold++;  // output the line
		++ps;
	}

	if (Coder.sShiftDate[0]) fprob << "\n*** Record was time shifted: " << Coder.sShiftDate << "  " << Coder.sTimeShift << endl;

	if (Coder.fhasProblem) {
		fprob << "\n*** Coder detected a problem:\n";
		fprob << Coder.sProblem << endl;
		fprob << "\n\n======================\n\n";
 	  return;
	}
	
	if (TabariFlags.fwriteEvents) {
		fprob << "\n\n--- Coded events ---\n";
		for (nline = 0; nline <= Coder.nEvents; ++nline) {
			Coder.getEventString(s,nline);
			ps = s;
			while(*ps) fprob.put(*ps++);
			fprob.put('\n');
		}
	}
	else fprob << "\n*** No events were coded\n";
			
	if (TabariFlags.fhasIssues) fprob << "Issues codes\n" << Coder.sIssueCodes << endl;
	
	Parser.writeParsing("P)roblem option");

	fprob << "\n\n======================\n\n";
	
} // writeTABProblem

void ProcessorClass:: autoCode(void)	
// autocoding option
{
int level = AutocodeSetup();
litstring start,send,sdate;
clock_t startsec, endsec;
float totalsec;
// errstring s;  // DEBUG
int kfiles = 1; // keeps track of lines written to display

	if (3 == level) return;  // Cancel set in menu

	if (4 == level) { // quiet mode set in command line
		level = 0;
	}

	ClearScreen();
	MakeTimeDateStr(start, sdate);
	startsec = clock();

	if (0 == level) {				// minimal display:file names only
		do{
			WriteLine("Autocoding ",textFile);
			while (readNextRecord()) {
				Parser.doParsing();
				Coder.codeEvents();
				writeEvents();
			} 	
			closeText();
			if (kfiles++ > WINDOW_HEIGHT) {
				kfiles = 0;
				ClearScreen(); 
			}
	  }
	  while (openNextText());
	}
	else if (1 == level) {	// date display
		WriteLine("Autocoding ",textFile);
		do{
			move(0,0);
			clrtoeol();
			addstr("Autocoding ");
			addstr(textFile);
			while (readNextRecord()) {
				showDate();
				Parser.doParsing();
				Coder.codeEvents();
				writeEvents();
			} 	
			closeText();
	  }
	  while (openNextText());
	}
	else {				// full display
		do{
			while (readNextRecord()) {
				ClearScreen();
				showRecord();
				Parser.doParsing();
				Coder.codeEvents();
				showEvents();
			} 	
			closeText();
	  }
	  while (openNextText());
	}

	endsec = clock();
	if (kfiles > WINDOW_HEIGHT-12) ClearScreen();
	MakeTimeDateStr(send, sdate);
	totalsec = (float)(endsec - startsec)/CLOCKS_PER_SEC;
	WriteLine("");	
	WriteAlert("Autocoding finished.");
	WriteLine("Start time   : ",start);
	WriteLine("Final time   : ",send);
	WriteLong("Records coded: ",nRecords);
	WriteLong("Events coded : ",totalEvents);
	WriteFloat("Time (secs)  : ",totalsec);
	WriteFloat("Recs per sec : ",((float) nRecords)/totalsec);
	WriteFloat("Evts per sec : ",((float) totalEvents)/totalsec);
	WriteLine("");	
	ResetCurs();

	nLast = 0;
	if (TabariFlags.fAutoCodemode != 4) { // '4' is autocoding quiet mode
		if (!TabariFlags.fActorUsage || !TabariFlags.fVerbUsage || !TabariFlags.fAgentUsage) {
			if (GetAnswer((char *)"Write dictionary usage files",'Y','N')) writeUsage();
		}
		else writeUsage();
	}

	endSession();
	 
} // autoCode


bool ProcessorClass:: readNextRecord(void)
// read next record; catch INPUT_EOF and STORY_LONG
{
	while (true) { // loop will skip incorrectly formatted dates
		try {
			if (fusingXML) {
				ReadXML.readXMLRecord();
				++nRecords;
			++nLast;			// increment the number of records coded ( ### isn't this redundant with nRecords? 01.11.10)
			}
			else readRecord(true);
			return true;
		}
		catch (int err) {
			if (INPUT_EOF == err) {
				return false;
			}
			if (STORY_LONG == err) {
				try {
					if (fusingXML) ReadXML.skipXMLRemainder();
					else skipRemainder();
				}
				catch (int err) {
					if (LINE_LONG == err) {
						ShowFatalError("Input line too long in text file; check the file format\nLast line:\n",sLine,shError31);
						return false;
					}
				}
																					// terminate the text neatly
				strcpy(&sentText[MAX_SENTENCE-23]," [STORY TRUNCATED...] ");
				writeError("Input record too long, story was truncated before coding", (char *)"",STORY_LONG);
				++nRecords;	
				return true;
				}
			if (LINE_LONG == err) {
				ShowFatalError("Input line too long in text file; check the file format\nLast line:\n",sLine,shError31);
				return false;
			}
			if ((err >= DATE_INVLD) && (err <= DAY_OOR)) { // badly formatted date, so skip the record
				if (*errorFile) { // .project has an <errorfile> 
					if (!fopenError) openError();
					ferror << "\nProblem: Incorrectly formatted date; record was not coded" <<  endl;
					ferror << "Date string: " << sRecDate <<  endl;
					ferror << "Input line: " << sLine <<  endl;
				}
				if (TabariFlags.fAutoCodemode < 0) {
					ClearScreen();
					ShowWarningError ("Incorrectly formatted date \"",sRecDate,"\"  Record will be skipped.", shError51);
				}
				skipRemainder();
			}
			else { 
				ShowFatalError("Indeterminant input error\nLast line:\n",sLine,shError33);
				return false;
			}
		} // catch
	} //while

} // readNextRecord

void ProcessorClass:: doProcess(char *sprojectname)	
// driver function
{
int imenu;
	if (TabariFlags.fAutoCodemode > -1) { // -a in command line
		WriteLine("Autocoding from ",sprojectname);
		readProject(sprojectname);
		initSession();
		autoCode();
		setupMenu(false);
		endSession();
	}
	
	WriteLine("Initializing from ",sprojectname);
	readProject(sprojectname);
	initSession();
	do  {
		while (readNextRecord()) {
			imenu = 3;
			while (imenu) {
				switch (imenu) {
					case 0: break;
					case 1: Parser.showParsing();
									Parser.htmlParsing();
									break;
					case 2: Modify.doModify();
					case 3: ClearScreen();
									showRecord();
									Parser.doParsing();
//									Parser.showParsing(); Pause(); // *** debug 									
									Coder.codeEvents();
									showEvents();
 									setupMenu(false);
									break;
					case 4: autoCode();
									setupMenu(false);
									break;
					case 5: doOther();
									break;
					case 6: endSession();
					case 7: reverseRecord(1);
				}
				imenu = RecordMenu();
			} // while irec
		} 	// while readNextRecord
		closeText();
	}
	while (openNextText());
	
	WriteAlert("Last input record has been processed\n\n");
	if (!TabariFlags.fActorUsage || !TabariFlags.fVerbUsage || !TabariFlags.fAgentUsage) {
		if (GetAnswer((char *)"Write dictionary usage files",'Y','N')) writeUsage();
	}
	else writeUsage();
	endSession();

} // doProcess

//___________________________________________________________________________________
// 																														Time-shifting routines

#if FALSE 
# Deactivated in version 0.8+ : sorting the output is the responsibility of the user

void ProcessorClass:: addTimeModList(void)
// adds next record to the TimeModList at the appropriate point based on julianDate
{
timeModListStruct * pcur;
timeModListStruct * pnext;

	if (!pTimeModList) {  // haven't started pTimeModList, so initialize it
		pTimeModList = new timeModListStruct;
		pnext = NULL;
		pcur = pTimeModList;
	}
	else {
		if (julianDate <= pTimeModList->julian) {  // insert at head of list
			pnext = pTimeModList;
			pTimeModList = new timeModListStruct;
			pcur = pTimeModList;		
		}
		else {  // find appropriate insertion point
			pcur = pTimeModList; 
			while ((pcur->next) && (julianDate > pcur->next->julian)) pcur = pcur->next;
			pnext = pcur->next;
			pcur->next = new timeModListStruct;
			pcur = pcur->next;
		}
	}
	pcur->next = pnext;
	pcur->julian = julianDate;
	pcur->floc   = fout.tellp();
	pcur->nrec   = Coder.nEvents + 1;;
	fisShifted = false;
//	showTimeModList(); // ### DEBUGGING
} // addTimeModList


void ProcessorClass:: showTimeModList(void)
// displays current TimeModList 
{
timeModListStruct * pcur =  pTimeModList; 

	cout << "Inserted " << julianDate << endl;

	while (pcur) {
		cout << pcur->julian << "  " << pcur->nrec << "  " << pcur->floc << endl;
		pcur = pcur->next;
	}
} // showTimeModList

void ProcessorClass:: writeTimeShift(timeModListStruct * ptime)
// writes a set of time-shifted records
{
int ka;

	fin.seekg(ptime->floc);
	if (fin.fail()) ShowWarningError("Error in time-shift seek. (ProcessorClass:: sortTimeOutput) ", "Enter <return>", shError00);
	for (ka = 0; ka < ptime->nrec; ka++) {	// write records in this group
		fin.getline(sLine,MAX_TAB_INPUT);
		if (fin.fail()) ShowWarningError("Error in time-shifted read. (ProcessorClass:: sortTimeOutput) ", "Enter <return>", shError00);
		if (cTimeShift == sLine[0]) fout << &sLine[1] << endl;			// write record, skipping marker
//  else we got here by accident -- put an assert here... ###
//		fout << sLine << "  " << ka << endl;			// write record with marker  ### DEBUG
		if (fout.fail()) ShowWarningError("Error in time-shifted write. (ProcessorClass:: sortTimeOutput) ", "Enter <return>", shError00);
	}
} // writeTimeShift

void ProcessorClass:: sortTimeOutput(void)
// pulls out the time-shifted records, then merges them into the correct order
// generates a temporary file "TABARI.shiftemp00" that is equal in size to the
// event output file.
// Note: this does NOT delete the items in the list beginning at pTimeModList
//       because it is the last function called the program terminates.
// ### currently not error checking the individual I/O operations
{
	toktype nextjulian = 0;
	toktype recjulian = 0;
	char slastdate[9];
	timeModListStruct * ptime;
	commloc finloc;
	
	if (TabariFlags.f4DigitYear) strcpy(slastdate,"--------");
	else strcpy(slastdate,"------");
	
	ptime = pTimeModList;
	if (ptime) nextjulian = ptime->julian;  // set nextjulian
	else return;														// there were no time-shifted records, so do nothing

	fin.open(eventFile,ios::in); 
	if (fin.fail()) ShowFatalError("Unable to reopen the event file (ProcessorClass:: sortTimeOutput) ",eventFile,shError03);
	fout.open("TABARI.shiftemp00",ios::out); 
	if (fout.fail()) ShowFatalError("Unable to open a temporary file for sorting time-shifted records (ProcessorClass:: sortTimeOutput) ",shError08);

	while (!fin.eof()) {
		finloc = fin.tellg();		// save the current input file location
  	if (fin.fail()) ShowWarningError("Error in time-shifted file position save. (ProcessorClass:: sortTimeOutput) ", "Enter <return>", shError00);
		fin.getline(sLine,MAX_TAB_INPUT);
		if (fin.eof()) break;
		if (cTimeShift == sLine[0]) continue;	// time-shifted record; skip it
		if (TabariFlags.f4DigitYear) {				// check if we've got a new date
			if (strncmp(slastdate,sLine,8)) {		// ### note that this is an opportunity to check for mis-ordered dates
				strncpy(slastdate,sLine,8);
				recjulian = MakeJulian (slastdate);
			}
		}
		else {  // do the 6-digit year
			if (strncmp(slastdate,sLine,6)) {    // ### note that this is an opportunity to check for mis-ordered dates
				strncpy(slastdate,sLine,6);
				recjulian = MakeJulian(slastdate);
			}
		}
		if ((nextjulian) && (nextjulian < recjulian)) {	// need to insert time-shifted records
			while ((nextjulian) && (nextjulian < recjulian)) {	// insert time-shifted records
//				cout << '*' << nextjulian << "  " << ptime->nrec << endl;
				writeTimeShift(ptime);
				ptime = ptime->next;
				if (ptime) nextjulian = ptime->julian;  // reset nextjulian
				else nextjulian = 0;										// set to zero if finished
			}
			fin.seekg(finloc);	// restore the file location
			if (fin.fail()) ShowWarningError("Error in time-shift file position restore. (ProcessorClass:: sortTimeOutput) ", "Enter <return>", shError00);
			fin.getline(sLine, MAX_TAB_INPUT);  // go back and get the line that triggered time-shift write
			if (fin.eof()) break;
		}
		fout << sLine << endl;			// write regular record
//		cout << recjulian << endl; 
	}		

	fin.clear();
	while (ptime) {	// insert records at end of list
		writeTimeShift(ptime);
		ptime = ptime->next;
	}

// clean up the files
	if (fin.eof()) fin.clear();
	fin.close();
	fout.close();
	if (fin.fail() || fout.fail())
		ShowFatalError("Time-shifted output is in the file \"TABARI.shiftemp00\"\nUnable to close files. (ProcessorClass:: sortTimeOutput) ",shError04);
	if (remove(eventFile)) 
		ShowFatalError("Time-shifted output is in the file \"TABARI.shiftemp00\"\nUnable to delete the output file. (ProcessorClass:: sortTimeOutput) ",eventFile,shError09);
	else if (rename("TABARI.shiftemp00", eventFile))
		ShowFatalError("Time-shifted output is in the file \"TABARI.shiftemp00\"\nUnable to rename the file. (ProcessorClass:: sortTimeOutput)",shError09);	
			
} // sortTimeOutput(void)
#endif				

//___________________________________________________________________________________
// 																														   Verification routines

void ProcessorClass:: doValidation(void)	
// verification option
{
int nErrors = 0;
	do {
		while (readNextRecord()) {
			ClearScreen();
			showRecord();
			Parser.doParsing();
			Coder.codeEvents();
			showEvents();
			try { checkEvents(); }
			catch (int i) {
                ++nErrors;
				if (0 == i) WriteNewLine("No verification record available");
				else if (1 == i) WriteNewLine("Verification record shows fewer events than were coded");
				else if (2 == i) WriteNewLine("Verification record shows more events than were coded");
				else if (4 == i) WriteNewLine("Time-shifting activated and verification record does not contain a date");
				else if (5 == i) WriteNewLine("Time-shifting activated and verification record does not match");
                else WriteNewLine("Verification record does not match");
                Pause();
			}	
		} 	
		closeText();
  }
  while (openNextText());

	WriteAlert("Validation finished.");
	WriteLong("Records coded: ",nRecords);
	WriteLong("Records wrong: ",nErrors);	
	WriteLong("Events coded : ",totalEvents);

	nLast = 0;
	endSession();
	 
} // doValidation

void ProcessorClass:: checkEvents(void)	
// Verification check. Returns if the events are okay, otherwise throws an error that is caught in doValidation
{
int ka;
int kf;
int nfields = 3; // just check source, target, event
instring s;
char *ps = s;
char *psa = strstr(sentText,"/*");  // skip to the start of comment block

//    Processor.fprob << endl << Processor.sentText << endl; // *** debug
	if (!psa) throw 0;  // no comment block in text
	psa = strchr(psa+2,'{');
	if (!psa) throw 0;  // no verification records in text
    if ((ps = strstr(psa,"OPTION:"))) {
       while (ps) {
            Copy2Chr(s, (ps+8), '}');
            ReadFiles.doOptionCommand(s);
            ps += strlen(s)+1;
            ps = strstr(ps,"OPTION:");
       }
        return;
    }	
	if  (Coder.fhasProblem) {
//        Processor.fprob << Coder.sProblem << endl; // *** debug
		if ('-' == *(psa - 1)) return; // skip check
		if (strstr(Coder.sProblem,"No eve") && strstr(psa,"---")) return; // no event
		if (strstr(Coder.sProblem,"No val") && strstr(psa,"---")) return; // no valid events
		if (strstr(Coder.sProblem,"Input err") && strstr(psa,"---")) return; // input error
		if (strstr(Coder.sProblem,"ard co") && strstr(psa,"###")) return; // discard
		if (strstr(Coder.sProblem,"omplex") && strstr(psa,"+++")) return; // complex
		throw 3;
	}
	else {
        if (TabariFlags.fhasTimeList) {  // get a date and validate it
            char *psb = strstr(psa,"}");  // skip to the start of comment block
            char psc[16];
            int ka = 0;
            while ((*psb) && (!isdigit(*psb))) ++psb;
            if (!*psb) throw 4;
            while ((*psb) && (ka < 14) && (isdigit(*psb))) {  // copy consecutive digits
                psc[ka] = *psb;            
                ++psb;
                ++ka;
            }
            psc[ka] = '\0'; 
			Coder.getEventString(s,0);		// Coder.fhasProblem traps the no event problem, right?
            if (!strstr(s,psc)) throw 5;    // not terribly selective but should work
        }
        
        for (ka=0; ka<=Coder.nEvents; ++ka) { // check all events
			Coder.getEventString(s,ka);		// move event string into s
			strcat(s,"\t");					// add terminal tab
			psa = strchr(psa,'{');			// move to start of next verification block
			if (!psa) throw 1;  			// more coded events than expected
			++psa;
            if ('-' == *(psa - 2)) return; // skip check of remaining records
			ps = s;
			while ((*ps) && (*ps != '\t')) ++ps;  			// skip date field
			++ps;
			if (TabariFlags.fhasAttribList) nfields = 4; 	// also check attribution
			while ((*psa) && (' ' == *psa)) ++psa; 			// skip leading blanks in text
			for (kf = 0; kf < nfields; ++kf) {              // check each field
				while (*ps == *psa) {  						// check that the codes match
					++ps;
					++psa;
				}
				if ((*ps != '\t') || 
				    ((*psa != ' ') && (*psa != '/') && ('}' != *psa))) throw 3; // match failed 
				while ((*psa) && 
							 ((' ' == *psa) || 
							  ('/' == *psa) || 
							  ('}' == *psa))) ++psa; // skip trailing blanks and separator in text
				++ps;														// skip tab in event
			} // for kf
//			if (TabariFlags.fhasIssues) cout << "\t" << Coder.sIssueCodes;
//  (It would be fairly straightforward to concatenate this, and go through a longer match,
//   but it isn't needed at the moment...
		} // for ka
		if (strchr(psa,'{')) throw 2;  // more verification blocks than events
	} 	// else
} // checkEvents
