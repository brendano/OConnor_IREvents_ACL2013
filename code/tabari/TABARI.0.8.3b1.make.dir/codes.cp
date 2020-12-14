// 																																		TABARI Project
//__________________________________________________________________________________
// 																																				codes.cp

// This file contains an assortment of utilities for managing codes

// Code storage strategy:
// Codes are stored as a sequence of 32-bit words in the TokenBuf array.  1-bit 
// flags in the high-order bits -- and the local sequencing of the words -- 
// determines the content of each words.  These sequences are interpreted by 
// various storage and access routines in the CodeStoreClass.  The storage scheme
// is designed to:
//   (a) Minimize the storage required for the most common type of codes;
//   (b) Provide a straightforward for the new codes types;
//   (c) Substantially extend the 1024 code limitation of KEDS, while still
//       making efficient use of storage
// [All of this is a way of apologizing for the fact that yes, this is complicated,
// but it is also fast and efficient].

// All objects with codes have a pointer to a 1-word "header" that determines the 
// type of information that follows. In addition, the structure of event, actor and
// issue codes is quite different; so type of code being accessed also determines
// how the TokenBuf words are interpreted.

// Note: <09.09.17> There are some older work-arounds in the code that were used
// to get around a 12-bit indexing scheme that was used prior to version 0.7.4 
// when TokBuf consisted of 16-bit words and was limited to 65536 entries. The
// current version uses 32-bit words and 28-bit indexing, and should accommodate
// ay dictionaries that are foreseeable at present. 

// In this description, bits are numbered from right to left starting at zero (i.e. 
// 0x80000000 is bit 31; 0x00000040 is bit 6, etc).  

// =========== EVENT CODES ===========
//
//  Header and paired code
//   bit      	
//  31-30  00 : No precedence 
//         01 : subordinate code
//         10 : dominant code
//         11 : [currently unused: getEventString will need to be changed to use it]
//  29-28  00 : simple code    
//         01 : paired code follows
//         10 : issue follows
//         11 : STA follows
//  27-00	    : code index								
//
// Issues  [this is still tentative...a hex serial for the issue might be added...###]
//  31-30      : [currently unused]
//  29-28  00 : termination    
//         01 : [currently unused]
//         10 : issue follows
//         11 : STA follows
//  27-00     : index of issue code sequence 
//
// Source, target and attribution (STA) links
//  31-30  00 : source 
//         01 : target
//         10 : attribution
//         11 : special code -- all other code formats
//  29		    : continuation
//  27-00     : index of actor/agent code sequence 

// Special formats
// ### need to figure out storage of issue codes
//  31-30  00 : source 
//         01 : target
//         10 : attribution
//         11 : special code -- all other code formats [currently unused]
//  29		    : continuation
//  27-00     : index of actor/agent code sequence 

// ====== ACTOR AND AGENT CODES ======

// These form a hierarchy:
//   [Date restrictions]					: interpreted through date headers
//      [Compound codes]					: contiguous; end of list has bit-13 = 0
//         [Actor:Agent pairs]		: interpreted through bit 14
//
// Header tags:
//   bit
//  31-30  00 : simple actor or agent code
//         01 : actor:agent pair
//         10 : date header
//         11 : special code -- all other code formats [currently unused]
//
//  29      1 : if the code is continued in a compound
//          0 : if code is isolated or the final code of a compound
//  28-00     :code index								

// Notes on actor and agent codes
//
// 1. The header for a simple code is simply its code index, which can be used
//    without further processing.  
// 
// 2. Compound actor and actor:agent codes are simply stored in sequence.  If the 
//    continuation bit is set then another code follows.  This is list is followed
//    until the continuation bit is off
//
// 3. Isolated actor and agent codes are stored in the same manner; the routine
//    that accesses the code is assumed to know the type.
 
// FORMAT OF DATE RESTRICTION HEADER
//												
// bit 							[least significant = 0]
// 31     1
// 30     0
// 29     continuation?
// 28-27	00: default code; next word is start of code sequence	
//		    01: next word is a <[date] constraint 
//		    10: next word is a >[date] constraint 
//		    11: next two words are [date]-[date] constraints
// 26-25  Currently unused
// 24-17	Number of words to next date header; if zero, this is in next word
//       ### <09.09.17> how is this used? -- when date restrictions are added?
//
// Date restrictions are handled by simply skipping through this list until a 
// code sequence matching the date is found.  The output system puts the default
// code at the end of the list irrespective of how it was originally entered.
//
// Date constraints are Julian dates (unsigned word) with a base at 1 January 1904
// (same as Macintosh MS-Excel).  This will handle dates until around 2085 (the base
//  can be adjusted by changing a constant, but if the current system *does not*  
// handle century leap years correctly [but it will, I'm sure...###]
//

// =========== TIME SHIFT CODES ===========
//
//  These are used in the <TIME> segment
//   bit      	
//  31-33	 00 : Increment 
//         10 : Decrement
//         01 : Value is a code index
//         11 : not used
//  29-00	    : value								

//__________________________________________________________________________________
//
//	Copyright (c) 2002-2009  Philip A. Schrodt.  All rights reserved.
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
// 																																		Declare globals

extern CharStoreClass CharStore;
extern TokenStoreClass TokenStore;
extern ProcessorClass Processor;

//___________________________________________________________________________________
// 																																	Utility functions

void CodeStoreClass::codeTest(void)
// this is a dummy function for testing components of this class
// ### need to go through and figure out the response to various errors
{
//instring s, sa;
/*
instring sdate;
toktype itok;
toktype ka;
bool  isCode;
toktype actcode, agtcode;
tokptr pt;

#if FALSE
	while (true) {					// test Julian system
		cin >> s;
		if (!isdigit(*s)) break;
		cout << "Input :" << s << endl;
		itok = MakeJulian(s);
		cout << "Julian:" << itok << endl;
		JulianString(sa,itok);
		cout << "Output:" << sa << endl;
	}
#endif	

#if TRUE									// test parseActorString
	cout << "parseActorString" << endl;
	strcpy(s,"ABC:CDE");
//	strcpy(s,"[ABC:CDE/GHI:JKL/ZXC/YHG]");
//	strcpy(s,"[ABC:CDE/GHI:JKL/ZXC/YHG <980101] [ABC/DFG >990101] [ABC:XYZ123/DFG]");
	strcpy(s,"[ABC:CDE/GHI:JKL/ZXC/YHG <980101][ABC:CDE/GHI:JKL/---/YHG 980101-990101] [ABC/DFG >990101] [QWE]");
	cout << "Input :" << s << endl;
	parseActorString(sa,s);
	cout << "Output:" << sa << endl;
#endif	

#if TRUE										// test parseEventString
	cout << "\nparseEventString"  << endl;
	strcpy(s,"[ABC:CDE]");
//	strcpy(s,"[ABC:CDE/GHI//ZXC/YHG]");
	strcpy(s,"[ABC:CDE/GHI//ZXC/YHG +([ABC:DFG/GHJ<980101]) $([---]) ]");
	cout << "Input :" << s << endl;
	parseEventString(sa,s);
	cout << "Output:" << sa << endl;
#endif	

#if TRUE										// test store/getActorString
	cout << "\ngetActorString"  << endl;
	strcpy(s,"[ABC:CDE]");
	strcpy(s,"[ABC]");
//	strcpy(s,"[ABC:CDE/GHI:JKL/ZXC/YHG]");
//	strcpy(s,"[ABC:CDE/GHI:JKL/ZXC/YHG <980101] [ABC/DFG >990101] [ABC:XYZ123/DFG]");
//	strcpy(s,"[ABC:CDE/GHI:JKL/ZXC/YHG <980101][ABC:CDE/GHI:JKL/---/YHG 980101-990101] [ABC/DFG >990101] [QWE]");
	strcpy(s,"[LEBAM1:ML1/LEBHZ1:GR1 <930101] [LEBAM2:ML2/LEBHZ2:GR2 930102-940101] [LEBAM3:ML3/LEBHZ3:GR3 >950101]");
	cout << "Input :" << s << endl;
	itok = storeActorString(s);
	getActorString(sa,itok);
	cout << "Output:" << sa << endl;
#endif	

#if TRUE										// test nextActorCode
		strcpy(sdate,"19981220");
//		strcpy(s,"[ABC]");
//		strcpy(s,"[ABC:CDE]");
//	strcpy(s,"[ABC:CDE/GHI:JKL/ZXC/YHG]");
//	strcpy(s,"[ABC:CDE/GHI:JKL/ZXC/YHG <980101] [ABC/DFG >990101] [ABC:XYZ123/DFG]");
	strcpy(s,"[ABC:CDE/GHI:JKL/ZXC/YHG <980101][ABC:CDE/GHI:JKL/---/YHG 980101-990101] [ABC/DFG >990101] [QWE]");
	Processor.julianDate = MakeJulian(sdate);
	cout << "Input :" << s << endl;
	cout << "Date  :" << sdate << endl;
	itok = storeActorString(s);
	pt = NULL;
	do {
		nextActorCode(actcode,agtcode,&pt,itok);
		if (actcode) cout << "Actor: " << getCodePtr(actcode) << endl;
		if (agtcode) cout << "Agent: " << getCodePtr(agtcode) << endl;
	} while (pt);

#endif	

#if TRUE										// test store/getEventString
//	strcpy(s,"ABC!:CDE?");
//	strcpy(s,"[ABC:CDE/GHI/ZXC/YHG]");
	strcpy(s,"[ABC:CDE/GHI//ZXC/YHG +([ABC:DFG/GHJ <980101]) $([---]) ]");
	cout << "Input :" << s << endl;
	itok = storeEventString(s);
	getEventString(sa,itok);
	cout << "Output:" << sa << endl;
#endif	

#if FALSE										// test store/getTimeCode
	strcpy(s,"[ 123 ]");
//	strcpy(s,"[---]");
//	strcpy(s,"[-1023]");
//	strcpy(s,"[10023]");
	WriteLine("Input :",s);
	itok = storeTimeCode(s);
	ka = getTimeCode(itok,isCode);
	getTimeString(sa,itok);
	WriteLine("Output:",sa);
#endif	
*/
} // codeTest


toktype CodeStoreClass::findCode (char *scode)
// Finds code index in Code_Buf; set to 0 if doesn't exist 
{	toktype ka = 1;
//		cout << "fC :" << scode << endl;  // *** debugging
		while ((ka < iCode) && (strcmp(scode, codeArray[ka].pcode))) { ka++;
//			cout << "  " <<  ka << " " << codeArray[ka].pcode << endl;  // *** debugging
		}
		if (ka < iCode) return ka;
		else return 0;
} // findCode 


toktype CodeStoreClass::addCode (char *scode)
// Finds code index in Code_Buf if it exists or else adds it; stores null text 
// [single-parameter version of the original addCode: see notes]
{	
	toktype	ka = findCode(scode);
	
	if (!ka) {
		if (iCode >= MAX_CODES - WARN_LEVEL) {
			if (iCode < MAX_CODES)
				ShowWarningError("Running out of memory in CodeStoreClass::codeArray", "Program will continue",shError10); 
			else ShowFatalError("Allocated memory exceeded in CodeStoreClass::codeArray",shError11);
		}
		codeArray[iCode].pcode =  CharStore.putCharStr(scode);
		codeArray[iCode].ptext = CharStore.charNullString;
		++iCode;
		return (iCode-1);
	}
	else return ka;
} // addCode 

toktype CodeStoreClass::addCode (const char *scode, char *stext)
// Finds code index in Code_Buf if it exists or else adds it; also stores text 
// This version is used only in the initialization 
// ### and therefore could probably eliminate some of the contigencies
{	
	toktype	ka = findCode(const_cast<char *>(scode));
	
	if (!ka) {
		if (iCode >= MAX_CODES - WARN_LEVEL) {
			if (iCode < MAX_CODES)
				ShowWarningError("Running out of memory in CodeStoreClass::codeArray", "Program will continue",shError10); 
			else ShowFatalError("Allocated memory exceeded in CodeStoreClass::codeArray",shError11);
		}
		codeArray[iCode].pcode =  CharStore.putCharStr(const_cast<char *>(scode));
		if (stext[0]) codeArray[iCode].ptext = CharStore.putCharStr(stext);
		else codeArray[iCode].ptext = CharStore.charNullString;
		++iCode;
		return (iCode-1);
	}
	else return ka;
} // addCode 

toktype CodeStoreClass::addCode (char *scode, char *stext)
// Finds code index in Code_Buf if it exists or else adds it; also stores text 
// ### should we switch this so that iCode is the *last* code added, rather than the 
// *next* code? -- that is how most everything else works
{	
	toktype	ka = findCode(scode);
	
	if (!ka) {
		if (iCode >= MAX_CODES - WARN_LEVEL) {
			if (iCode < MAX_CODES)
				ShowWarningError("Running out of memory in CodeStoreClass::codeArray", "Program will continue",shError10); 
			else ShowFatalError("Allocated memory exceeded in CodeStoreClass::codeArray",shError11);
		}
		codeArray[iCode].pcode =  CharStore.putCharStr(scode);
		if (stext[0]) codeArray[iCode].ptext = CharStore.putCharStr(stext);
		else codeArray[iCode].ptext = CharStore.charNullString;
		++iCode;
		return (iCode-1);
	}
	else return ka;
} // addCode 

char* CodeStoreClass::getCodePtr (toktype itok)
// Returns a pointer to the text of code for Code_Buf[itok]; warn and set to 0 if doesn't exist 
{	
	if (itok < iCode) return codeArray[itok].pcode;
	else {
		ShowWarningError ("Out of range code index in CodeStoreClass::getCodePtr","Program will continue but may crash soon",shError00 );
	  return 0;
	}
} // Find_Code 

char* CodeStoreClass::getCodeLabel (toktype itok)
// Returns a pointer to label for Code_Buf[itok]; warn and set to 0 if doesn't exist 
{	
	if (itok < iCode) return codeArray[itok].ptext;
	else {
		ShowWarningError ("Out of range code index in CodeStoreClass::getCodeLabel","Program will continue but may crash soon",shError00 );
	  return 0;
	}
} // getCodeLabel 

toktype CodeStoreClass::storeCode (char *scode, toktype mask)
// cleans blanks from scode, get the codeindex, |'s the index with mask, 
// stores in TokBuf, returns index of that TokBuf location
// ### needs to handle extra storage of high codes
{	
toktype index;

	TrimBlanks(scode);
	index = (toktype) addCode(scode);
	index |= mask;
	return 	TokenStore.putToken(index);
} // storeCode

toktype CodeStoreClass::makeCode (char *scode, toktype mask)
// same cleaning as storeCode, except it doesn't store 
// ### needs to handle extra storage of high codes
{	
toktype index;

	TrimBlanks(scode);
	index = (toktype) addCode(scode);
	index |=mask;
	return index;
} // makeCode


//___________________________________________________________________________________
// 																																		Actor functions

void CodeStoreClass::addNullString (char *s)
// checks for a solitary dated code; adds null code as default
// ### <09.11.18> This looks problematic on multiple dimensions...
//  a. first statement would get confused with a null code
//  b. why just look for two blocks? -- what about other incomplete combinations?
//  c. can't the null default be handled by the coder? 
{
char * pst;

	pst = strpbrk(s,"<>-"); 			// check for the date operators
	if ((!pst) || (!isdigit(*(pst + 1)))) return;    		// none there
	pst = strchr(s,']');   // find terminator of code block
	if (strchr(pst,'[')) return;  // we've got more than one, so okay
	strcat(s," [---]");			// add a null code  

}  // addNullString


void CodeStoreClass::parseActorString (char * sparse, char *s)
// parse the actor string in s; return in sparse
// parser does the following:
// 1. converts ][ to | and removes initial [ and final ]
// 2. converts <date>-<date> to ~date~date  (so it won't be confused with --- null code)
// 		this is a bit messy but the notation is sufficiently intuitive that there is no point in changing it
// 3. blank-terminates string 
//
// Throws: EMPTY_CODE, MISS_RBRAC
{	
instring sa;
char * pst;
char * pivot;

	if (!strchr(s,'[')) {   // handle a simple code entered from the editor
		TrimBlanks(s);  
		strcpy(sparse,s);
		strcat(sparse," ");
		return;
	}
	strcpy(sparse,"");
	
	addNullString(s);   // check whether we need to add a null

												// dated string
	pst = strchr(s,'[');
	while (pst) { 				// go through the [code <time>] units
		pst++;							// move past the [
		if (strchr(pst,']')) {
			Copy2Chr(sa, pst,']');  // copy the string inside [...] 
			TrimBlanks(sa);
			if('\0' == sa[0]) throw EMPTY_CODE;
		}	
		else throw MISS_RBRAC;		

		pivot = strchr(sa,'-');			// search for a <date>-<date> structure
		if (pivot && (*(pivot+1) == '-') && (*(pivot+2) == '-')) { // context is null code, so skip it
			pivot += 3;
			pivot = strchr(pivot,'-');
		}
		if (pivot) {
			*pivot = '~';			// replace '-' with '~'
			--pivot;
			while (*pivot == ' ') --pivot; // allow blanks
			while (isdigit(*pivot)) --pivot; // back down past the date
			if ((pivot >= sa) && (*pivot == ' ')) *pivot = '~';
			else {
				throw EMPTY_CODE;  // not always, but must likely cause of incorrect formating here
			}
		}
		strcat(sparse,sa);
		pst = strchr(pst,'[');
		if (pst) strcat(sparse, "|"); // separate units with "|"	
		else		 strcat(sparse, " "); // blank-terminate string		
	}

} // parseActorString

void CodeStoreClass::getActorCode (char *s, tokptr ptok)
// Gets the string version of the undated actor code sequence pointed to by ptok
{	
	toktype token;

	token = *ptok;
	if (!(token & maskActrFlag)) {   // simple code
		strcpy(s,getCodePtr(token));
		return;
	}
	strcpy(s,"");
	do {											// loop through the compound
		strcat(s,getCodePtr(token & maskActrCode));
		if (token & maskActrPair) {  // get the agent
			strcat(s,":");
			token = *(++ptok);  		// get next token
			strcat(s,getCodePtr(token & maskActrCode));
		}
		if (token & maskContinue) {	// addition compound element
			strcat(s,"/");
			token = *(++ptok);  		// get next token
		}
		else token = 0;			// we're done
	} while (token);	

} // getActorCode

char * CodeStoreClass::getAgentCodePtr (toktype tokindex)
// Gets the pointer to string of agent code at index tokloc
{	
	return getCodePtr(TokenStore.getTokenValue(tokindex));
} // getAgentCodePtr

void CodeStoreClass::getActorString (char *s, toktype itok)
// Gets the string version of the optionally dated actor code sequence pointed to by itok
{	
	instring sa;
	toktype token;
	tokptr ptok = TokenStore.getTokPtr(itok);

	strcpy(s,"[");
	token = *ptok;
	if (!(token & maskActrFlag)) {   // simple code
		strcat(s,getCodePtr(token));
		strcat(s,"]");
		return;
	}

	if (!(token & maskDateHead)) {   // undated compound code
		getActorCode(sa, ptok);
		strcat(s,sa);
		strcat(s,"]");
		return;
	}

	// we've got a dated sequence
	do {
		if (!((token & maskConstrnt) ^ maskINCntsrt)) { // - date constraint
			getActorCode(sa, (ptok+3));
			strcat(s,sa);
			strcat(s," ");
			JulianString(sa,*(ptok+1));
			strcat(s,sa);
			strcat(s,"-");
			JulianString(sa,*(ptok+2));
		}
		else if (token & maskLTCntsrt) { // < date constraint
			getActorCode(sa, (ptok+2));
			strcat(s,sa);
			strcat(s," <");
			JulianString(sa,*(ptok+1));
		}
		else if (token & maskGTCntsrt) { // > date constraint
			getActorCode(sa, (ptok+2));
			strcat(s,sa);
			strcat(s," >");
			JulianString(sa, *(ptok+1));
		}
		else getActorCode(sa, (ptok+1));// default constraint

		strcat(s,sa);
		strcat(s,"]");
		if (token & maskContinue) {	// additional date element
			strcat(s," [");
			ptok += (token & maskDateSkip);
			token = *ptok;  		// skip to the next date header
		}
		else token = 0;			// we're done
	} while (token);	
			
} // getActorString

void CodeStoreClass::putDate(char *s)
// transfers a date beginning at s into TokenBuf
{
//litstring sdate;		// date segment of s; allows quite a bit of slack for a string that is too long
//char *pdate;
toktype tjulian;  							 // value of date

//	Processor.fprob << s << endl;
	Processor.validateDate(s);  // this can throw errors that will be caught in ReadActors
//	pdate = sdate; // ### <09.11.18> These 3 lines were some earlier error checking, after a fashion...not sure we need it now...
//	while (isdigit(*s)) *pdate++ = *s++;
//	*pdate = '\0';						// terminate string
//	tjulian = MakeJulian(sdate); 
	tjulian = MakeJulian(s); 
	TokenStore.putToken(tjulian);
} // putDate

void CodeStoreClass::setDateSkips(toktype datehead)
// goes through a sequence of date headers and sets the skip fields
{
tokptr dateptr = TokenStore.getTokPtr(datehead);   // next date header
tokptr nextptr = dateptr;   // next date header

	while (*dateptr & maskContinue) { // loop until continuation bit off
		if (!((*nextptr & maskConstrnt) ^ maskINCntsrt)) nextptr += 2;	// skip other - date		
		else if ((*nextptr & maskLTCntsrt) || (*nextptr & maskGTCntsrt)) ++nextptr; // skip over <, > date
		++nextptr;
		while (!(*nextptr & maskDateHead)) ++nextptr;   // skip over the codes
		*dateptr |= (nextptr - dateptr);  // set the skip field
		dateptr = nextptr;
	}
} // setDateSkips

toktype CodeStoreClass::storeActorCode (char * s)
// parses and stores actor codes
{	
	char *pst = s;
	char *pair;
  instring sa;
  toktype	itok = TokenStore.iToken;

	while (pst) {
			pst = strchr(s,'/');		// get the next segment of a compound list
			if (pst) {
				Copy2Chr(sa,s,'/');   // ### could substitute code here; we already know pst
				s = ++pst;
			}
			else strcpy(sa,s);			// no more compound segments

			pair = strchr(sa,':');
			if (pair) {  					// we've got a code pair
					*pair = '\0';			// split the code into two strings
					storeCode(sa,maskActrPair);
					if (pst) storeCode(++pair, maskContinue);
					else 	 storeCode(++pair, maskActrStop);
			}
			else {
				if (pst) storeCode(sa, maskContinue);
				else 		 storeCode(sa, maskActrStop);
			}
	}
	return itok;
} // storeActorCode

toktype CodeStoreClass::storeAgentCode (char * s)
// stores agent codes
{	
	char *pst = s;
  instring sa;
  toktype	itok = TokenStore.iToken;

	pst = strchr(s,'[')+1;
	if (strchr(s,']')) {
		Copy2Chr(sa, pst,']');  // copy the string inside [...] 
		TrimBlanks(sa); // ### not needed, right? storeCode does this...
		if('\0' == sa[0]) throw EMPTY_CODE;
	}	
	itok = storeCode(sa, maskActrStop);
	return itok;
} // storeAgentCode

toktype CodeStoreClass::storeDatedString (char * s)
// creates a date header, then stores codes.  Returns index of header
{	
toktype idate;   // index of date header
toktype tokhead = maskDateInit;  // header record
instring scode;		// code segment of s
char *pst;
																	// extract dates from string 
	if ((strchr(s,'<')) || (strchr(s,'>'))) {  // deal with <... and >... constraints
		if (strchr(s,'<')) {
			tokhead |= maskLTCntsrt;		// set constraint bit
			pst = strchr(s,'<');
			Copy2Chr(scode,s,'<');
		}
		else {
			tokhead |= maskGTCntsrt;
			pst = strchr(s,'>');
			Copy2Chr(scode,s,'>');
		}
		if (!*scode) throw EMPTY_CODE;  //no code is present
		idate = TokenStore.putToken(tokhead);
		while(*pst && !isdigit(*pst)) ++pst;	// move past < or > to first digit of date
		putDate(pst);
		storeActorCode(scode);
	}
	else if (strchr(s,'~')) {   					// deal with - constraint, which is now  ~...~...
		tokhead |= maskINCntsrt;		// set constraint bit
		idate = TokenStore.putToken(tokhead);
		Copy2Chr(scode,s,'~');
		if (!*scode) throw EMPTY_CODE;  //no code is present
		pst = strchr(s,'~');
		while(*pst && !isdigit(*pst)) ++pst;	 // move past first ~
		putDate(pst);
		pst = strchr(pst,'~');	 // go to the second ~
		if (!pst) throw EMPTY_CODE;  // [apprently?] can occur if no code is present in [dddddd-dddddd]
		while(*pst && !isdigit(*pst)) ++pst;	 // move past second ~
		putDate(pst);
		storeActorCode(scode);
	} 
	else {
		idate = TokenStore.putToken(tokhead);
		storeActorCode(s); // default code
	}

	return idate;
} // storeDatedString

toktype CodeStoreClass::storeActorString (char *st)
// Processes the s code string; returns index of the header
{
toktype	itok = TokenStore.iToken;
toktype ia;
tokptr pa;
char 	*pst;
instring s,sitem;			// stores intermediate strings

	parseActorString(s,st);
																					// ### if (!pst) then we've got an error
	if (strchr(s,'|')) {      	// parse date restricted codes
		pst = s;
		while (pst) {
			Copy2Chr(sitem,pst,'|');
			ia = storeDatedString(sitem);
			pst = strchr(pst,'|');			// go to start of next code
			if (pst) pst++;					// skip past |
		}
		pa = TokenStore.getTokPtr(ia);
		*pa &= maskContnOff;     // set the continuation bit of last header to zero
		setDateSkips(itok);      // go back and set the skip fields
	}
	else storeActorCode(s); 
		
	return itok;
} // storeActorString 


void CodeStoreClass:: nextActorCode (toktype &idxactor, toktype &idxagent, tokptr *pnext, toktype istart)
//	Gets the next actor and agent codes from the optionally dated actor code sequence pointed 
//	to by *pnext.  If *pnext == NULL, then the date restrictions are resolved, and the
//	first code-pair is returned.  Subsequent calls work through any coded-compound. 
//	Returns false when there are no additional codes.
{	
toktype token;
tokptr	ptok;
bool 		found = false;
	if (!(*pnext)) { 												//initialize sequence 
		ptok = TokenStore.getTokPtr(istart);
		token = *ptok;

		if (!(token & maskActrFlag)) {   // simple code
			idxactor = token;
			idxagent = 0;
			*pnext = NULL;
			return;
		}

		if (token & maskDateHead) { 		// we've got a dated sequence
			while (!found) {
				if (!((token & maskConstrnt) ^ maskINCntsrt)) { // - date constraint
					if ((Processor.julianDate >= *(ptok+1)) && (Processor.julianDate <= *(ptok+2))) {
						*pnext = ptok + 3;
						found = true;
					}
				}
				else if (token & maskLTCntsrt) { // < date constraint
					if (Processor.julianDate < *(ptok+1)) {
						*pnext = ptok + 2;
						found = true;
					}
				}
				else if (token & maskGTCntsrt) { // > date constraint
					if (Processor.julianDate > *(ptok+1)) {
						*pnext = ptok + 2;
						found = true;
					}
				}
				else {			// default constraint
					*pnext = ptok + 1;
					found = true;
				}

				if (!found) {
					if (token & maskContinue) {	// there are additional date elements
						ptok += (token & maskDateSkip);
						token = *ptok;  		// skip to the next date header
					}
					else {				// no valid codes for this date
						idxactor = 0;
						idxagent = 0;
						*pnext = NULL;
						return;
					}
				}
			} // while
		} // if maskDateHead
		else *pnext = ptok;		   // undated compound code
	} // if !*pnext
	
														// ptok now points to the first codes
	token = **pnext;
	idxactor = (token & maskActrCode);	// get actor code
	if (token & maskActrPair) {  				// get the agent
		token = *(++(*pnext));  					// get next token
		idxagent = (token & maskActrCode);
	}
	else idxagent = 0;

	if (token & maskContinue) {	// addition compound element
		++(*pnext);  								// return pointer to next token
		return;
	}
	else {
		*pnext = NULL;
		return;
	}	
} // nextActorCode


//___________________________________________________________________________________
// 																																	Time functions

toktype CodeStoreClass::storeTimeCode (char * s)
// Stores time codes.  Returns the location where the code is stored
{	
	int     ival = 0;
	toktype	ka = 0;
  toktype	itok = TokenStore.iToken; // save this so it can be returned

	if (strstr(s,sNullCode)) ka = indexNull;  // check for the standard codes
	else if (strstr(s,sDiscardCode))  ka = indexDiscard;
	else if (strstr(s,sComplexCode))  ka = indexComplex;

	if (ka) ka |= maskTimeCode; // store a standard code index
	else {
		ival = atoi(&s[1]);   // copy integer following [ 
		if (ival>0) ka = ival;
		else {	// store decrement
			ival = -ival;
			ka  = ival;
			ka |= maskTimeDecr;	 // set decrement bit
		}
	}	
	if (ival > MAX_TIME_VALUE) {  // do bounds checking
		ShowWarningError("Time increment/decrement is too large", s,"Value has been set to zero",shError52); 
		ka = 0;
	}		
	TokenStore.putToken(ka);  
	return itok;
} // storeTimeCode

int CodeStoreClass::getTimeCode (toktype itok, bool &isIndex)
// Get time code.  If isIndex is true, value is a code index
{	
	toktype	ka = TokenStore.getTokenValue (itok);
  int			ival = ka & maskTimeClr;
  
  isIndex = (bool)(ka & maskTimeCode);
  if (ka & maskTimeDecr) ival = -ival;
	return ival;
} // getTimeCode

void CodeStoreClass::getTimeString (char *s, toktype itok)
// Get the string version of the time code pointed to by itok
{	
	char scr[8];  // holds value
	toktype	ka = TokenStore.getTokenValue (itok);
  toktype	ival = ka & maskTimeClr;
  
	s[0] = '[';
  if (ka & maskTimeCode)  {  //  code
		strcpy(&s[1],getCodePtr(ival));
		strcat(s,"]");
		return;
	}

  if (ka & maskTimeDecr) 	s[1] = '-';
	else s[1] = '+';
	sprintf(scr,"%d",ival);
	strcpy(&s[2],scr);
	strcat(s,"]");

} // getTimeString

//___________________________________________________________________________________
// 																																	Event functions

void CodeStoreClass:: appendEventActor(char * sparse, char *s, const char *sc)
// handles an actor $(...) substring, appends to sparse.  c is the initial marker
{
instring sa, sb;

	strcat(sparse,sc);
	Copy2Chr(sa,(s+2),')');  // ### error check here?
	parseActorString(sb,sa);
	strcat(sparse,sb);
	strcat(sparse,") ");	
} // appendEventActor

void CodeStoreClass::parseEventString (char * sparse, char *s)
// parse the event string in s; return in sparse
// parser does the following:
// 1. converts $(...) to $...)
// 2. converts +(...) to &...)
// 3. converts @(...) to @...)
// 4. converts actor strings using parseActorString
// 5. converts issue strings using parseIssueString
// 6. gets rid of outermost []
//
// Throws MISS_RBRAC
//
// Note 1. ### once this is debugged, switch the parse delimiters to \01,\02, etc.
// Note 2. The strpbrk(s,"/@&$") is a partially-implemented facility that would allow
//         an event string to also designate the source, target, attribute (STA) and 
//         issues.  However, these have not been integrated into the coder as of 
//         2002.01 since we aren't using them.
{	
instring sa, sb;
char * pst;

	pst = strstr(s,"+(");    // change + to & to avoid problems with +++
	if (pst) *pst = '&';  

	pst = strstr(s,"[");    // move past [ if it exists;
	if (pst) s = (pst+1);

	pst = s;
	while (*pst) pst++;		// go to the end of the string
	while ((pst > s) && (*pst != ']')) {
		--pst;   // search backwards for last ']'
	}
	if (']' == *pst) *pst = '\0';			// so terminate it here
	else throw MISS_RBRAC;
//  Processor.fprob << "CSC:pES Mk1 \"" << s << "\"" << endl; // *** debug
	
	pst = strpbrk(s,"/@&$");
	if (!pst) {
		strcpy(sparse,s);  // nothing more to process
		return;
	}
	else Copy2Chr(sparse,s,*pst);
	
	while ((pst) && ('/' == *pst)) {			// process the issue strings
		s = pst + 1;
  	if (!(*s)) break;
  	pst = strpbrk(s,"/ ");
		if (pst) Copy2Chr(sa,s,*pst);
		else strcpy(sa,s);
		parseIssueString(sb,sa);
		strcat(sparse,"/");
		strcat(sparse,sb);
	}
														// process the STA string
	pst = strstr(s,"$(");
	if (pst) appendEventActor (sparse, pst, "$");
	pst = strstr(s,"&(");
	if (pst) appendEventActor (sparse, pst, "&"); 
	pst = strstr(s,"@(");
	if (pst) appendEventActor (sparse, pst, "@");

} // parseEventString

void CodeStoreClass::getEventString (char *s, toktype itok)
// Gets the string version of the event code sequence pointed to by itok  }
{	
	instring scode;
	toktype token;
	tokptr ptok = TokenStore.getTokPtr(itok);

	strcpy(s,"[");
	token = *ptok;
	if (!(token & maskEvntFlag)) {   // simple code
		strcat(s,getCodePtr(token));
		strcat(s,"]");
		return;
	}
	// get initial code 
	strcat(s,getCodePtr(token & maskEvntCode));
	if (token & maskEvtSubor) strcat(s,"?");
	else if (token & maskEvtDomin) strcat(s,"!");
	
	if (((token & maskSTANext) ^  maskSTANext) && (token & maskPairNext)) { // process paired code
		strcat(s,":");
		token = *(++ptok);  // get next token
		strcat(s,getCodePtr(token & maskEvntCode));
		if (token & maskEvtSubor) strcat(s,"?");
		else if (token & maskEvtDomin) strcat(s,"!");
	}

	while (((token & maskSTANext) ^  maskSTANext) && (token & maskIssuNext)) { // process issues code
		strcat(s,"/");
		token = *(++ptok);  // get next token
		getIssueString(scode,token & maskEvntCode);
		strcat(s,scode);
	}	
	
	while (token & maskContinue)	{  // process STA cases
		token = *(++ptok);  // get next token
		getActorString(scode,token & maskEvntCode);
		if (token & maskEvntTar) strcat(s," +(");  // target code
		else if (token & maskEvntAtt) strcat(s," @("); // attribution code
		else strcat(s," $("); // source
		strcat(s,scode);
		strcat(s,")");
	}
	strcat(s,"]");

} // getEventString 

void CodeStoreClass:: decodeEventCode (toktype &evtcode, bool &fissub, bool &fisdom, toktype icode)
// Gets the code and status out of icode
{	
	evtcode = (icode & maskEvntCode);
	fissub = (bool) (icode & maskEvtSubor);
	fisdom = (bool) (icode & maskEvtDomin);
}

void CodeStoreClass:: getEventCode (toktype &icode, toktype &ipair, toktype itok)
// Gets the primary and optional paired code of the event code sequence pointed to by itok
// Calling routine needs to call decodeEventCode() is get status
{	
	toktype token;
	tokptr ptok = TokenStore.getTokPtr(itok);

	token = *ptok;
	if (!(token & maskEvntFlag)) {   // simple code
		icode = token;
		ipair = 0;
		return;
	}

	icode = token;	// get initial code 
	
	if (((token & maskSTANext) ^  maskSTANext) && (token & maskPairNext))  
		ipair = *(++ptok);  // get paired code token
	else ipair = 0;
	
} // getEventCode 

toktype CodeStoreClass::storeEventString (char * st)
// stores the event code sequence (duh...)
// This is more complicated than the actor code storage because issue and
// actor subcode sequences.  These are stored first, with the indices stored
// in an intermediate array.  Then that array is stored after the "next"
// fields have been set.
// Note: there are an assortment of "magic numbers" involved in the use of the
//       header array, so be very careful (or modify these to constants) if 
//       any are changed.
{	
	char *pst;
	char *psa;
	char *pair;
	int last = 0;		// keeps track of last header filled
	int ka;
  instring s, sa;
  toktype itok;
  toktype mask;
  toktype header[15];   // intermediate storage of headers: 
  											// 15 = evt + pair + 10 issues + STA
  												
//  Processor.fprob << "CSC:sES Mk1 " << st <<  endl; // *** debug
	parseEventString(s,st);

  memset(header,'\0',15*sizeof(toktype));   // zero out the header array
  												
	pst = strchr(s,'/');			// start of issues list
	pair = strchr(s,':');			// paired code
	if (pair) {
		Copy2Chr(sa,s,':');
		if ((psa = strchr(sa,'?')))      mask = maskEvtSubor;	// set subordinant flag : the "=' is deliberate
		else if ((psa = strchr(sa,'!'))) mask = maskEvtDomin;	// set dominant flag
		else mask = 0;
		if (psa) *psa = '\0';																// eliminate precedence
		header[0] = makeCode(sa,mask);									// save primary code

		if (pst) Copy2Chr(sa,++pair,'/');							  // get paired code
		else 	   Copy2Chr(sa,++pair,' ');							
		if ((psa = strchr(sa,'?')))      mask = maskEvtSubor;	// process precedence codes as above
		else if ((psa = strchr(sa,'!'))) mask = maskEvtDomin;	
		else mask = 0;
		if (psa) *psa = '\0';																
		header[1] = makeCode(sa,mask);									// save paired code : continuation is not set yet
		}
	else {
		if (pst) Copy2Chr(sa,s,'/');							  // copy the code 
		else 	   Copy2Chr(sa,s,' ');							
		if ((psa = strchr(sa,'?')))      mask = maskEvtSubor;	// process precedence codes as above
		else if ((psa = strchr(sa,'!'))) mask = maskEvtDomin;
		else mask = 0;
		if (psa) *psa = '\0';																// eliminate precedence
		header[0] = makeCode(sa,mask);									// save primary code : continuation is not set yet
	}
	
	if (pst) {					// we've got issues... (is this a relationship or what?)
		ka = 2;					// index for start of issues
		psa = ++pst;			// move start of s past '/'
		while (*pst) {
			pst = strchr(psa,'/');		// get the next segment of issue list
			if (pst) {
				Copy2Chr(sa,psa,'/');   // ### could substitute code here; we already know pst
				psa = ++pst;
			}
			else {
				Copy2Chr(sa,s,' ');	 // no more issues
				*pst = '\0';		// signal we're done
			}
			TrimBlanks(sa);
			if (sa) {							// if false, code was just a placeholder
				if (*pst) mask = maskIssuNext;
				else 			mask = 0;
				header[ka] = mask | storeIssueString(sa);
			}
			++ka;       
		}	// end of issue processing loop
	}  // if (pst) 

	if (strpbrk(s,"$&@")) {			// check for STA codes
		
		pst = strchr(s,'$');
		if (pst) {  // process source
			Copy2Chr(sa,(pst+1),')');
			header[12] = storeActorString(sa);
			header[12] |= maskEvntSrc;
		}
			
		pst = strchr(s,'&');
		if (pst) {  // process target
			Copy2Chr(sa,(pst+1),')');
			header[13] = storeActorString(sa);
			header[13] |= maskEvntTar;
		}

		pst = strchr(s,'@');
		if (pst) {  // process attribution
			Copy2Chr(sa,(pst+1),')');
			header[14] = storeActorString(sa);
			header[14] |= maskEvntAtt;
		}
	}  // if (strpbrk(s,"$&@"))
													    // finish by storing active headers in TokenStore
	last = 0;
	while (last < 15) {
		if (header[last]) {
			ka = last + 1;																				// figure out what comes next, set field
			while ((ka < 15) && (!header[ka])) ++ka;
			if (ka >= 15) header[last] &= maskContnOff;  // no more active items
			else if (1 == ka) header[last] |= maskPairNext;
			else if (ka > 11) header[last] |= maskSTANext;
			else header[last] |= maskIssuNext;
			
			if (last) TokenStore.putToken(header[last]);		// store the header
			else itok = TokenStore.putToken(header[0]);		// save the index of the initial header
		}
		++last;
	}
	return itok;
	
} // storeEventString

//___________________________________________________________________________________
// 																																	Issues functions

void CodeStoreClass::parseIssueString (char * sparse, char *s)
// parse the issue string in s; return in sparse
// currently just a placeholder : simply does strcpy(sparse,s)
{
	strcpy(sparse,s);
} // parseIssueString

void CodeStoreClass::getIssueString (char *s, toktype itok)
// Gets the string version of the issue code sequence pointed to by ptok  }
// ### currently just a placeholder
{	
	strcpy(s,"<issue>");
} // getIssueString

toktype CodeStoreClass::storeIssueString (char *s)
// Processes the s string; returns index of the header
// ###at the moment this doesn't do anything 
{
//	parseIssueString(s);
  return 0xCACA;					// return something we'll recognize
} // storeIssueString
