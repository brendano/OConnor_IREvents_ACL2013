// 																																		TABARI Project
//__________________________________________________________________________________
// 																																			utilities.cp

// This files contains an assortment of general-purpose utilities that involve the
// interface; most of this are error-handling routines
//
//__________________________________________________________________________________
//
//	 Copyright (c) 2000 - 2007  Philip A. Schrodt.  All rights reserved.
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
// 																																						Headers

#include "TABARI.h"

// Note: the "TABARI.h" header contains the function prototypes for this file

//___________________________________________________________________________________
// 																																   external globals

extern TabariFlagsClass TabariFlags;

//___________________________________________________________________________________
// 																																			

const int	Month[] = {0,31,59,90,120,151,181,212,243,273,304,334,366};		// used for Julian computation: # days prior to Month[n] JAN=1

//___________________________________________________________________________________
// 																																			

void IncrementIndex(toktype &index, int max, const char *sloc)
// increment index, check against max for overflow.
// ### If a true overflow is reached, program dies via ShowFatalError -- probably should do this
//     more elegantly...
{
litstring s;
	++index;
	if (index >= max - WARN_LEVEL) {
		strcpy(s,sloc);
		if (index < max)
			ShowWarningError("Running out of memory in ",s, "Program will continue but may crash soon",shError10); 
		else ShowFatalError("Allocated memory exceeded in ",s,shError11);
	} 
} // IncrementIndex

void CheckIndex(toktype index, int max, const char *sloc)
// check against max for overflow without incrementing.
// the first level check will be redundant in some of the calls
// ### If a true overflow is reached, program dies via ShowFatalError -- probably should do this
//     more elegantly...
{
litstring s;
	if (index >= max - WARN_LEVEL) {
		strcpy(s,sloc);
		if (index < max)
			ShowWarningError("Running out of memory in ",s, "Program will continue but may crash soon",shError10); 
		else ShowFatalError("Allocated memory exceeded in ",s,shError11);
	} 
} // IncrementIndex

char * Copy2Chr(char *s1, char *s2, char c)  
// copies s2 into s1 up to but not including c, or else to end of s2
// returns pointer to start of s1
{
char *pst = s1;

	while ((*s2) && (*s2 != c)) *s1++ = *s2++;
	*s1 = '\0';
	return pst;
}

void TrimEndBlanks(char *s)
// trim terminal white space from s
{
char *pst = s;

	if (!pst) return;     // do nothing if the string is null
	while (*pst) pst++;  // move to the end of the string
	pst--;
	while ((pst>s) && isspace(*pst)) pst--;
	if (pst >= s) pst++;   // move past non-blank char unless string was entirely blank
	*pst = '\0';					// terminate the string
}

void TrimLeadBlanks(char *s)
// trim initial white space from s
{
char *pst = s;

	while ((*pst) && isspace(*pst)) pst++;  // move to the first non-blank
	if (pst == s) return;     							// do nothing if no initial blanks
	while ((*s++ = *pst++)) ;									// transfer the string
}

void TrimBlanks(char *s)
// trim initial and terminal white space from s
{
char *pst = s;

	if (!pst) return;     // do nothing if the string is null
	
	while (*pst) pst++;  // move to the end of the string
	pst--;
	while ((pst>s) && isspace(*pst)) pst--;
	if (pst >= s) pst++;   // move past non-blank char unless string was entirely blank
	*pst = '\0';					// terminate the string

	pst = s;
	while ((*pst) && isspace(*pst)) pst++;  // move to the first non-blank
	if (pst == s) return;     							// do nothing if no initial blanks
	while ((*s++ = *pst++)) ;									// transfer the string
}

char * Int2Str(char *s,int k)
// converts k to left-justified string
// s must be initialized to a blank string long enough to hold k
// returns pointer to start of s
{
char *pc = s;

	while (*pc) ++pc; // move to end of s
	--pc;
	while (k) {				// pick off the digits
		*pc = (char) (48 + (k % 10));	// get the ASCII for a digit
		k /= 10;
		--pc;
	}
	TrimLeadBlanks(s);		// left-justify
	return s;
} // Int2Str

long MakeJulian (char *s)
// Converts dates of form YYMMDD or YYYYMMDD to Julian serial beginning 0 = 1 Jan 1904
// (which matches the Excel convention). Returns 0 if date is out of [0,65535] range 
// Notes:
// 1. 2-digit years are pivoted on <2030 
// 2. This does only minimal  error checking -- generally ProcessorClass:: validateDate()
//    should be called to weed out invalid dates before they are processed here.
// 3. This does NOT handle even-century leap years except for 2000  (recall that 1900 and
//    2100 -- which are out of range with this base -- are not leap years)
{
long 	mo,dy,yr;
long	ij;

// get parts of date  
  if (TabariFlags.f4DigitYear){
		yr = (short)s[2] * 10 + (short)s[3] - 532;   // use 532 to subtract 1904
		if ('2' == *s) yr +=100;
		mo = (short)s[4] * 10 + (short)s[5] - 528;
		dy = (short)s[6] * 10 + (short)s[7] - 528;
  } 
	else {
		yr = (short)s[0] * 10 + (short)s[1] - 532;   // use 532 to subtract 1904
		if (yr<26) yr +=100;				// pivot on 2030  (which is now 2026)
		mo = (short)s[2] * 10 + (short)s[3] - 528;	// 528 = 10*(int)'0' + (int)'0' -- converting chars to number
		dy = (short)s[4] * 10 + (short)s[5] - 528;
 	} 
  						
	ij =  yr * 365;
	ij += ((yr+3) / 4) + Month[mo-1] + (dy-1);				// ((yr+3)/4) is the leap-year adjustment
	if (((yr % 4) == 0) && (mo > 2)) ij++;	// adjust for month within a leap-year
	
	if ((ij < 0) || (ij > 65535)) return 0;  
	else return (toktype) ij;
} // MakeJulian

void JulianString (char *s, long jdate)	
// Converts Julian-1904 date to string
{
long yr, mo, dy, ic;
												// subtract leap units
	yr = (jdate / 1461) * 4;
	jdate = jdate % 1461;
	if (jdate >= 366) {
		++yr;
		jdate -= 366;
		while (jdate >= 365) {
			++yr;
			jdate -= 365;
		}
	}
	if ((yr % 4) > 0) { // non-leap year -- this isn't checking for 1900, 2100
		mo = 0;
		while (jdate >= Month[mo]) ++mo;
		dy = jdate - Month[mo-1] + 1;
	}
	else {									// leap year 
		if (jdate < 31) {    //jan 
			mo = 1;
			dy = jdate + 1;
		}
		else if (jdate < 60) { // feb 
			mo = 2;
			dy = jdate - 30;
		}
		else {
		mo = 3;
			while (jdate > Month[mo]) mo++;
			dy = jdate - Month[mo-1];
		}
	}
	if (TabariFlags.f4DigitYear) {   // create a 4-digit year
		yr += 1904;
		strcpy(s,"YYYYMMDD");   // put something there so we can replace it
		ic = 4;
		s[3] = 48 + yr % 10;		// 48 is ASCII (int) '0'
		yr /= 10;
		s[2] = 48 + yr % 10;
		yr /= 10;
		s[1] = 48 + yr % 10;
		yr /= 10;
		s[0] = 48 + yr;
	}	
	else {							// do the 2-digit year
		yr += 4;
		if (yr >=130) yr -=100; // pivot on 2030
		strcpy(s,"YYMMDD");  
		ic = 2;
		s[1] = 48 + yr % 10;
		yr /= 10;
		s[0] = 48 + yr % 10;
	}
		s[ic+1] = 48 + mo % 10;   // set month
		if (mo >= 10) s[ic] = '1';
		else  s[ic] = '0';
		
		s[ic+3] = 48 + dy % 10;  // set day
		dy /= 10;
		s[ic+2] = 48 + dy;

} // JulianString

void MakeTimeDateStr (char *stime, char *sdate)
// puts formatted date and time in string.  Strings are assumed to be litstring
{
	time_t thetime;
	struct tm * datetime;
	
	time(&thetime);
	datetime = localtime(&thetime);
	strftime(stime,LIT_SIZE,"%H:%M %Z ",datetime);		
	strftime(sdate,LIT_SIZE,"%a %d %b %Y",datetime);		
} // MakeTimeDateStr


//___________________________________________________________________________________
// 																																		Old test code

#if FALSE // test for GetAnswer
	if (GetAnswer((char *)"This is a test",'Y')) cout << "true";
	else cout << "False";
	cout << "\nDone";
#endif

#if FALSE		// test for MakeTimeDate
litstring stime,sdate;

	MakeTimeDateStr(stime,sdate);
	cout << stime << "  " << sdate << endl;
#endif

#if FALSE		// test for Int2Str
instring s = "     ";
int ka = 123;
//		Int2Str(s,ka);
//		cout << "'" << s << "'\n";
		cout << "'" << Int2Str(s,ka) << "'\n";
#endif

#if FALSE		// test for Trim functions
instring s;

	while ( 'Q' != s[0]) {
		cin.getline(s, MAX_TAB_INPUT);
//		TrimLeadBlanks(s);
//		TrimEndBlanks(s);
		TrimBlanks(s);
		cout << "'" << s << "'\n";
	}
	cout << "Done";
#endif

