// 																																		TABARI Project
//___________________________________________________________________________________
// 																																	 			xmlinput.cp

//  This file contains the routines for handling XML-coded input records
 
//  ----------------------------------------------------------------------------  
//  BBN FILES 
//	             				
//   ASSUMPTIONS:	
//   1. <NE> and <PRONOUN> XML fields are on a single line	
//   2. Filtering unprintable characters
//   3. Blank is inserted in the output stream at newline
//	 4. Record ID comes before DateInfo
//	 5. Currently assumes all XML files use 4-digit dates						
// 
//   RECOGNIZED TAGS 
//
//   BBN FORMAT
//
//		Information in story header
//   SERIAL:  any string				story sequence string
//   INFODATE:  YYYYMMDD				Story date
//			
//	Tags for skipping information:
//	<IGNORE>...</IGNORE>, <BRACKET>...</BRACKET> 
//		Any information inside these tags is skipped entirely.  These cannot be 
//    embedded to multiple levels; that is, skipping will stop when the 
//    terminating tag is reached.
//
//	All other tags except those noted below are skipped: the text is treated
//  as if the tags were not present.
//
//	PROCESSED XML TAGS
//	<TEXT> </TEXT>
//		Delimits the text to be searched for sentences
//
//	<S ID="n"> </S>
//		Delineates a single sentence.  The "n" in the ID= field is used as the 
//		sentence sequence number for purposes pronoun forwards
//
//	<NE WDN="string"> </NE>
//		Delineates a noun-phrase.  If a WDN string is present, it replaces the 
//    phrase; otherwise the phrase is copied.
//
//	<PRONOUN WDN="string"> </PRONOUN>
//		Delineates a pronoun reference.  If a WDN string is present, it replaces the 
//    pronoun; otherwise the pronoun is copied.
//
//	DESIGNATING XML FORMATING:
//		Use an XML="<type>" field in the <textfile> tag in the .project file, 
//    e.g. <textfile XML="BBN">
//		Valid tags: BBN
//								Verid
//		Note: case *is* significant in the tags
//		To add tags, see constructor for ReadXMLClass and the function 
//		int ReadXMLClass:: setXMLType(char * sLine)	                               								
//	                                								
//
//  NOTE TO PHIL H.: I haven't made this very general or put in much error checking 
//
//	### MODIFICATION:  ReadFilesClass:: getField is a nicely error-checked routine
//	that might be useful here if we started processing more fields
//__________________________________________________________________________________
//
//	 Copyright (c) 2002 - 2005 Philip A. Schrodt.  All rights reserved.
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
// 																																   Global Variables

extern TabariFlagsClass TabariFlags;
extern ProcessorClass Processor;

//___________________________________________________________________________________
// 																																   internal globals

ReadXMLClass 			ReadXML;

//___________________________________________________________________________________
// 																																   


#if FALSE
void ReadXMLClass:: writeXMLInfo(void)
// ### debugging output
{
	cout << "\nDate:" << Processor.sRecDate << endl;
//	printf("Sentence %2d\n", idSent);
	cout << "Text:\n" << Processor.sentText << endl;
	Pause();
} // writeXMLInfo
#endif

void ReadXMLClass:: checkLength(int incr, char * pst)
// increment idxText; throw STORY_LONG if too long
{
	if (idxText + incr >= MAX_SENTENCE) {
		pLast = pst;
		throw STORY_LONG;
	}
	idxText += incr;					// increment text location
} // checkLength

int ReadXMLClass:: setXMLType(char * sLine)
// evaluate the <text XML="..."> record and get the index of the XML file type
{
char *	pst = strstr(sLine,Processor.sXMLField);
char *	pend;   // end of legal match 
char *	pmatch;
 
	if (!pst)  return 0;	 // no XML field
	
	while ('\"' != *pst ) ++pst; // go to the " following WDN
	++pst;												// skip over "
	pend = pst;
	while ((*pend) && ('\"' != *pend )) pend++; // find the end of the target string
	
// check the tag possibilities
 	pmatch = strstr(pst,sBBNTag);   
	if ((pmatch) && (pmatch < pend)) return indexBBN;
 	pmatch = strstr(pst,sVeridTag);   
	if ((pmatch) && (pmatch < pend)) return indexVerid;
	return 0; 

} // setXMLType

void ReadXMLClass:: SetXMLTags (int index)
// reset the tags to the appropriate XML format
{
  indexCurrent = index;	

	if (index == indexBBN) {
	strcpy(sDateLine,"INFODATE:"); // date line
	strcpy(sRecIDLine,"SERIAL:"); // sequence line
	strcpy(sTextTag,"<TEXT>");
	strcpy(sTextEnd,"</TEXT>");
	strcpy(sSentTag,"<S ");
	strcpy(sSentEnd,"</S>");
	strcpy(sSentFld,"ID");		// sentence number
	strcpy(sNounTag,"<NE ");
	strcpy(sNounEnd,"</NE>");
	strcpy(sPronTag,"<PRONOUN ");
	strcpy(sPronEnd,"</PRONOUN>");
	strcpy(sNounFld,"WDN");	// standard text for NE and PRONOUN
	strcpy(sBracTag,"<BRACKET>");
	strcpy(sBracEnd,"</BRACKET>");
	strcpy(sIgnrTag,"<IGNORE>");
	strcpy(sIgnrEnd,"</IGNORE>");
	}
	else 	if (index == indexVerid) {
	// ### insert Verid initializations here
	}
	
} // SetXMLTags

bool ReadXMLClass:: openXMLFile(char * sfilename)
// open file, get first record ID and date
{
	// ### may not be the best place to do this...
	TabariFlags.f4DigitYear = true;

	fin.clear(); //  clear the eof flag from previous file operations
	fin.open(sfilename,ios::in); 
	if (fin.fail()) {
		ShowWarningError("Unable to open the following XML file:",sfilename, "File will be skipped", shError03);
		fin.clear();
		return false;
	}

	pLast = NULL;
	while ((!fin.eof()) && ('\n' != fin.peek() )) {
		fin.get(sLine, MAX_XML_INPUT); // skip over the first line
	}
	
	if (getXMLDate()) {
		ShowWarningError("Unable to find valid date or record ID field in XML file ",sfilename, "File will be skipped",shError62);
		fin.clear();
		return false;
	}
	else return true;

} //openXMLFile

void ReadXMLClass:: closeXMLFile(void)
// closes the XML text file
{
	fin.close();
} // closeXMLFile

void ReadXMLClass:: skipXMLRecords(void)
// skips nLast records.  This is called from ProcessorClass:: skipRecords(void)
// ### still needs to be implemented
// Implementation: just repeated call getXMLDate(), then count the <S> tags inside
// the record
{
	ShowWarningError("Record skipping has not yet been implemented for XML files","Manually skip records",shError00);
}	// skipXMLRecords

void ReadXMLClass:: insertReplace(char * pst)
// inserts a replacement marker into pSent 
{
	checkLength(3, pst);
	*pSent++ = ' '; // blank terminate string
	*pSent++ = REPLACE_LIT; // mark the start of the replacement
	*pSent++ = ' '; // blank terminate replacement delimiter
} // insertReplace

bool ReadXMLClass:: getXMLDate(void)
// get the sequence and date fields, read to the sTextTag marker
// can throw INPUT_EOF.
// <05.04.14> ### I'm not entirely sure that INPUT_EOF error is being
// handled correctly; probably was not tested
{
char *pdate;
char * pst;
int ka;

	while ((!fin.eof()) && (!strstr(sLine,sRecIDLine))) {
		fin.getline(sLine, MAX_XML_INPUT); // ### need to change MAX_TAB_INPUT
	}
	if (fin.eof()) throw INPUT_EOF;
																		// get record ID
	pst = sLine + strlen(sRecIDLine) + 1;
	while ((*pst) && (isspace(*pst))) ++pst;  // skip blanks
	pdate = Processor.sRecID; 							// transfer the ID string
	ka = 0;
	while (*pst) {
		*pdate++ = *pst++;
		if (++ka > MAX_RECID_LEN) {   // error check the length
			ShowWarningError("Record ID in XML file is too long",Processor.sRecID,"ID string has been truncated",shError61);
			return false;
		}
	}
	pdate = '\0';

	while ((!fin.eof()) && (!strstr(sLine,sDateLine))) {
		fin.getline(sLine, MAX_XML_INPUT); // ### need to change MAX_TAB_INPUT
	}
	if (fin.eof()) throw INPUT_EOF;
																		// get date
	pst = sLine;
	while ((*pst) && (!isdigit(*pst))) ++pst;  // find start of date
	pdate = Processor.sRecDate; 							// transfer the date
	while ((*pst) && (isdigit(*pst))) *pdate++ = *pst++;
	pdate = '\0';

	Processor.makeEnglishDate();
	
	while ((!fin.eof()) && (!strstr(sLine,sTextTag))) {
		fin.getline(sLine, MAX_XML_INPUT); // ### need to change MAX_TAB_INPUT
	}	

	if (fin.eof()) throw INPUT_EOF;
	
	return true;

} // getXMLDate

char * ReadXMLClass:: gotoEnd(char * pst)
// skip to the end of a tag, reading new lines as needed.  Returns a pointer
// to the first char after the end of the tag
{
	while ('>' != *pst) {
		if (*pst) ++pst;
		else {
			fin.getline(sLine, MAX_XML_INPUT);
			pst = sLine;
		}
	}	
	return (++pst);
} // gotoEnd

char *  ReadXMLClass:: doBRACKET(char * pst)
// skip over everything in <BRACKET>...</BRACKET>
{
	while ((!fin.eof()) && (!strstr(sLine,sBracEnd))) {
		fin.getline(sLine, MAX_XML_INPUT); // ### need to change MAX_TAB_INPUT
	}
	pst = strstr(sLine,sBracEnd); 
	return gotoEnd(pst);
} // doBRACKET

char *  ReadXMLClass:: doIGNORE(char * pst)
// skip over everything in <IGNORE>...</IGNORE>
// (currently same function as doBRACKET)
{
	while ((!fin.eof()) && (!strstr(sLine,sIgnrEnd))) {
		fin.getline(sLine, MAX_XML_INPUT); // ### need to change MAX_TAB_INPUT
	}
	pst = strstr(sLine,sIgnrEnd);
	return gotoEnd(pst);
} // doIGNORE

char *  ReadXMLClass:: startSent(void)
// find the start of the next sentence, using current sLine as the starting point
// returns NULL if eof 
// ### still need to deal with IO conditions
{
char * pst;

	if (pLast == NULL) pLast = sLine;
	if (strstr(pLast,sSentTag)) pst = strstr(pLast,sSentTag);  // next sentence is in current sLine
	else {  										// read until we find it
		fin.getline(sLine, MAX_XML_INPUT); 
		while ((!fin.eof()) && 
					(!strstr(sLine,sSentTag)) &&
					(!strstr(sLine,sTextEnd))) {
			fin.getline(sLine, MAX_XML_INPUT);
		}	
		if (fin.eof()) return NULL;

		if (strstr(sLine,sTextEnd)) { // need to get next block
			if (getXMLDate()) return NULL; // ### need to deal with the skip contingency
			while ((!fin.eof()) && (!strstr(sLine,sSentTag))) {
				fin.getline(sLine, MAX_XML_INPUT);
			}
		}	
		if (fin.eof()) return NULL;

		pst = strstr(sLine,sSentTag);
	} 

	idxText = 0; // current position in text

	pst = strstr(pst,sSentFld);  // get ID=" field
	while ('\"' != *pst ) ++pst; // go to the next "
	Processor.nSeq = atoi(++pst);					
	return gotoEnd(pst);	// skip remainder of tag		

} // startSent

char * ReadXMLClass:: doNE(char * pst)
// process the NE header
// (this is currently also used for the pronouns)
// this can throw STORY_LONG
{
	
	pst = strstr(pst,sNounFld);
	if (!pst) { // no WDN field, so do nothing
		inReplace = false;
		return gotoEnd(pst);	// skip remainder of tag	
	}	
	while ('\"' != *pst ) ++pst; // go to the " following WDN
	++pst;												// skip over "
	inReplace = true;
	while ('\"' != *pst ) {
		*pSent++ = *pst++;  	// copy text
		checkLength(1, pst);	// increment text location
	}
	insertReplace(pst);
	return gotoEnd(pst);	// skip remainder of tag		
} // doNE

char * ReadXMLClass:: doPRONOUN(char * pst)
// process the PRONOUN header
{
	return doNE(pst);
} // doPRONOUN

void ReadXMLClass:: readXMLRecord(void)
// read the next sentence from an XML record;moves the text into sentText[]
// can throw exceptions INPUT_EOF and STORY_LONG
{
char *	pst = startSent();

	pSent = Processor.sentText; 
	*pSent = '\0';
	if (!pst) throw INPUT_EOF;

	while (TRUE) {
		if (!*pst) {  // hit the eol; get the next line
			fin.getline(sLine, MAX_XML_INPUT);
			pst = sLine;
			*pSent++ = ' ';  // insert blank line into text
			continue;
		}
		if ('<' == *pst) {  // process the tags

			if ('/' == *(pst+1)) {  // process the terminators
					if ((pst == strstr(pst,sNounEnd)) || 
							(pst == strstr(pst,sPronEnd)))	{
						if (inReplace) {
							insertReplace(pst);
							inReplace = false;
						}
						pst = gotoEnd(pst);
					}
					else if (pst == strstr(pst,sSentEnd)) {  // hit end of sentence
						if (*(pSent-1) != ' ') {
							checkLength(1,pst);
							*pSent++ = ' ';	// make sure to blank terminate
						}
						*pSent = '\0';  // terminate the text
						pLast = gotoEnd(pst);
					//	writeXMLInfo();  // ### debugging
						return;		// normal exit point
					}
				else 	pst = gotoEnd(pst);	 // skip over any other terminators
			} // terminators
			
			else if (pst == strstr(pst,sNounTag)) pst = doNE(pst);
			else if (pst == strstr(pst,sPronTag)) pst = doPRONOUN(pst);
			else if (pst == strstr(pst,sBracTag)) pst = doBRACKET(pst);
			else if (pst == strstr(pst,sIgnrTag)) pst = doIGNORE(pst);
			else 	pst = gotoEnd(pst);	 // skip over any tag we don't recognize 	
		}  // tags
		else {
			if (isprint(*pst)) {  // filter out unprintable chars
				*pSent++ = *pst++;  // copy text
				checkLength(1,pst);
			}
			else ++pst;
		}
	} // while (TRUE)
			
} // readXMLRecord

void ReadXMLClass:: skipXMLRemainder(void)	
// skip remainder of a long sentence
{
char * pst;
	while ((!fin.eof()) && (!strstr(sLine,sSentEnd))) {
		fin.getline(sLine, MAX_XML_INPUT); // ### need to change MAX_TAB_INPUT
	}
	pst = strstr(sLine,sSentEnd); 
	pLast = gotoEnd(pst);
} // skipXMLRemainder
	
