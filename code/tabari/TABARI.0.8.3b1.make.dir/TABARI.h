// 																		TABARI Project
//__________________________________________________________________________________
// 																			TABARI.h
//  This file is the global header file for the program
 
//__________________________________________________________________________________

//  Release Number: 0.01		29 Feb 2000		First release; only "mergefiles()" is functional
//  Release Number: 0.05		09 Mar 2000		Most event coding functional
//  Release Number: 0.10		24 Mar 2000		First Linux release
//  Release Number: 0.20		31 Mar 2000		All core KEDS coding features functional
//	Release Number: 02.02		07 April 2000
//	Release Number: 03.01		28 January 2002
//	Release Number: 03.07		12 February 2002
//	Release Number: 04.01		04 March 2002
//	Release Number: 04.05		26 January 2003
//	Release Number: 04.07		28 April 2003
//	Release Number: 04.08		12 July 2003
//	Release Number: 04.09		25 March 2005 (OS-X)
//	Release Number: 05.00		10 April 2005 (ncurses)
//	Release Number: 05.01		 9 May 2005 (checkPat refactoring)
//	Release Number: 05.02		19 May 2005 (freqs)
//	Release Number: 06.01		19 August 2007 (interface mods)
//	Release Number: 06.02		13 June 2008 (seg marking bug corrections)
//	Release Number: 06.03		30 June 2008 (additional bugs found in ICEWS 25-Gb coding exercise)
//	Release Number: 07.2		31 Jan 2009 (agents, multiple dictionaries, root endings)
//	Release Number: 07.3		03 July 2009 (ICEWS high volume coding bug fixes)
//	Release Number: 07.4		30 Sept 2009 (32-bit token storage)
//	Release Number: 07.5		30 Jan 2010 (regular verb, noun endings)
//	Release Number: 07.6		24 Apr 2011  (Bug fixes from JABARI)
//	Release Number: 08.1		24 Jan 2012  (synsets, DEFAULT)
//	Release Number: 08.2		14 Feb 2012  (COMMA, bug fixes to TIME, FORWARD)
//	Release Number: 08.3		 3 Jue 2012  (agent conversion, word loc output)
//
//__________________________________________________________________________________
//
//	 Copyright (c) 2000 - 2012  Philip A. Schrodt.  All rights reserved.
//
// 	Redistribution and use in source and binary forms, with or without modification,
// 	are permitted under the terms of the GNU General Public License 3.0:
// 	http://www.opensource.org/licenses/gpl-license.html
//
//	Report bugs to: schrodt@psu.edu

//	The most recent version of this code is available from the KEDS Web site:
//		http://web.ku.edu/keds

//	For plausible indenting of this source code, set the tab size in your editor to "2"

//___________________________________________________________________________________
// 																															Constant declarations

#define TRUE  1
#define FALSE 0

#define OFF_3WAY    0    // 3-way switch values
#define FALSE_3WAY  1
#define TRUE_3WAY   2

#define TAB_DEBUG FALSE		// controls the primary debugging mode
#define DEBUG_PAT FALSE		// controls the secondary debugging in checkPattern and support functions (coder.c) <12.01.16> Uses Processor.fprob now
#define DEBUG_VERB FALSE	// secondary debugging in doVerb and checkPattern 
#define DEBUG_PAT FALSE		// secondary debugging in checkPattern and support functions


//___________________________________________________________________________________
// 																																 #include libraries

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>
#include <cstdlib>
#include <fstream>

#include <curses.h> // requires -lncurses compiler option

using namespace std;

//___________________________________________________________________________________
// 																																   

const char sRelease[]     = "0.8.3b1a";   	// Release number
const char sReleaseDate[] = "June 2012";   // Release date

const int WARN_LEVEL = 32;	// controls when to issue warning in IncrementIndex

const int MAX_TAB_INPUT =  256;  // max length of an input line + 1 
const int MAX_XML_INPUT =  512;  // max length of an XML input line + 1 
const int MAX_SENTENCE  = 2048;  // max length of source text + 1 				
const int MAX_DATE_LEN  =   10;  // max length of date string (includes room 8-digit date and time-shift marker) 				
const int MAX_ENGDATE_LEN = 16;	 // max length of English version of date string 				
const int MAX_RECID_LEN =   64;  // max length of record ID string 				
const int MAX_DOCID_LEN =   64;  // max length of document ID string 				
const int LIT_SIZE 		= 	64;	 // length of litstring[]
const int SFILE_SIZE 	= 	64;	 // length of filestring[]
const int MAX_REC_BACK 	= 	32;	 // maximum number of records one can go in reverse
const int MAX_ERROR			= 	 8;	 // maximum type of errors reported by ProcessorClass:: writeError

const int MAX_LEX =       128;  	// max literals in sentence; size of lexArray			
const int MAX_SYNT = MAX_LEX;
const int MAX_TAGS =     1024;  	// size of tagArray[]			
const int MAX_TAG_TYPES =  32;		// size of tagText[]			
const int MAX_TEMP =      128;  	// size of tempArray[]			
const int MAX_LITRLIST =  512;  	// size of literaList[]			
const int MAX_FREQ =       64;  	// size of kountFreq[]			

// IMPORTANT NOTE:
// As noted in Chapter 3.4 of the manual, TABARI uses *fixed* blocks of memory for storing the dictionaries.
// Consequently, as dictionaries increase in size -- prior to the ICEWS project, we considered 1000
// actors, 10,000 verb patterns, and 500 codes to be large dictionaries -- the various array sizes 
// need to be increased as well -- the program provides warnings when this needs to be done, and you 
// will find that when one dictionary-related array size needs to be increased, pretty much all of  
// them will. Most modern programs, in contrast, would handle this automatically, and in fact it might
// not be that difficult to incorporate that here, but it has never been a priority. So just beware.
//
const int MAX_CODES  = 8192;     // max distinct codes [update  08.06.19 for SAE]
const int MAX_CHAR  = 524288;    // size of CharBuf array  [update 09.09.17: 2 ^ 19  <12.09.12>]					
const int MAX_TOKENS = 196608;	// size of TokenArray   [update 09.09.16: 65536 * 1.5]	[update 09.09.16: 65536 * 3]	

const int MAX_PHRASES =  402432;		// size of phraseBuf array  [update 08.12.10]	
const int WARN_PHRASES = 400000;	// warning level for same

const int	MAX_ROOTS = 32768;		// size of rootArray  [update 08.03.01 for UGA]
const int	MAX_PATTERN = 49152;	// size of PatternArray [update 08.04.17 for UGA]

const int MAX_LITERALS =  49152;	// size of litArray [update 06.08.10, 12.01.09]
const int MAX_LIT_HASH =  1369; // size of litIndex array
const int MAX_LITBUF   = 65353; // size litBuf array (start, freq and synonym lists)  [update 09.09.17]
const int COMMA_LIT    =  1;  	// litArray index for comma
const int NUMBER_LIT   =  2;    // litArray index for numbers will be 2
const int VERB_LIT     =  3;  	// litArray index for *
const int SOURCE_LIT   =  4;    // litArray index for $
const int TARGET_LIT   =  5;  	// litArray index for +
const int ATTRIB_LIT   =  6;  	// litArray index for @
const int COMPOUND_LIT =  7;    // litArray index for %
const int SKIP_LIT     =  8;    // litArray index for ^
const int OPBRACK_LIT  =  9;    // litArray index for {
const int OR_LIT 			 = 10;  	// litArray index for |
const int CLBRACK_LIT  = 11;    // litArray index for }
const int REPLACE_LIT  = 15;    // litArray index for XML replacement text
const int SYMBOL_LIMIT = 16;  	// litArray indices involving symbols are < SYMBOL_LIMIT

const int MAX_EVENTS = 64;       // max basic events found in a single text 		
const int MAX_EVENT_STRINGS = 128;  // max events strings, following expansion of complex codes 		
const int MAX_TIME_VALUE = 8191;	// maximum value of time increment/decrement

																// CodeWarrior SIOUX window settings
const int WINDOW_WIDTH 	=  80;	// width of output window
const int WINDOW_HEIGHT =  48;	// height of output window
const int WINDOW_LEFT 	=	 10;	// top pixel of output window
const int WINDOW_TOP		=  40;	// left pixel of output window

const char sNullCode[]    = "---";
const char sBlankCode[]   = "***";
const char sComplexCode[] = "+++";
const char sDiscardCode[] = "###";
const char sEliteCode[]   = "~ELI";  // used on agents when 'FORMER' is found

const char sCodeNull[]    = "[---]";  // inserts null code into sCode on phrase parsing errors
								 
// SET: parameter values
const int MIN_LEX= 8;					// minimum words for a valid sentence

// input error flags
const int INPUT_OK 	=	 0;	// last input operation was okay
const int INPUT_EOF =	 1;	// last input operation hit eof
const int INPUT_SKIP = 2;	// last input operation requires a context-specific skip
const int LINE_LONG  = 4;	// input was > MAX_TAB_INPUT
const int STORY_LONG = 5;	// story was > MAX_SENTENCE
const int EMPTY_CODE = 6;	// no code in [...]
const int MISS_LBRAC = 7;	// missing '['
const int MISS_RBRAC = 8;	// missing ']'
const int MISS_VERBM = 9;	// missing verb marker '*'
const int PAT_ORPHAN = 10;	// patterns found before verb in .verbs file
const int DUP_VERB   = 11;	// Verb  duplicated an existing root
const int DUP_VFORM  = 12;	// Verb form duplicated an existing root
const int NO_VFORM   = 13;	// No verb forms inside {...} 
const int DATES_LONG = 14;	// Date restrictions are too long 
const int DATE_INVLD = 15;	// Invalid date string 
const int YEAR_OOR   = 16;	// Year is out of range in date 
const int MONTH_OOR  = 17;	// Month is out of range in date 
const int DAY_OOR    = 18;	// Day is out of range in date 
const int DUP_ROOT   = 19;	// General duplicate of root 
const int MISS_SYN   = 20;	// Undeclared &synset designator

// parser error flags
const int PARSE_OK 			=	0;	// parsing okay
const int GENERIC_ERROR =	1;	// unspecified error
const int NO_TEXT_ERROR = 2;	// no text in record
const int TOO_LONG_ERROR= 3;	// too many words in sentence
const int TOO_SHORT_ERROR= 4;	// too few words in sentence
const int HEAD_TAG_ERROR= 5;	// <not used>
const int TAIL_TAG_ERROR= 6;	// <not used>
const int MARK_NOUN_ERROR= 7;	// problem in noun marking
const int COMPOUND_TAG_ERROR= 8;	// problem in tag balancing in compound marking
const int SUBORD_TAG_ERROR= 9;	// problem in tag balancing in subordinant clause marking
const int PRONOUN_REF_ERROR= 10;	// Missing pronoun reference tag in ParserClass:: getReference
const int ACTOR_CODE_ERROR= 11;	// empty actor code in CoderClass:: getActorStructCodes
const int CONVERT_AGENT_ERROR= 12;	// empty agent tail in ParserClass:: convertAgents

// coder error flags
const int CODER_OK 			   =	0;		// coding okay
const int CODER_ERROR      =	21;		// unspecified coder error
const int CLAUSE_END_ERROR =  22;		// can't find end of clause
const int HEAD_ATTRIB_ERROR=  23;		// No head tag on attribution phrase
const int EVT_OVRFLOW_ERROR=  24;		// No additional storage in CoderClass::putEvent
const int CMP_OVRFLOW_ERROR=  25;		// Overflow in expandCompoundActors
const int EVT_STRG_ERROR   =  26;		// Invalid index in getEventString
const int MAX_PHRASE_ERROR =	27;		// Excessive words between nonconsecutive elements of a phrase (getActorHead)

// informative error strings
const char sErrMISS_RBRAC[] = "Missing right-bracket ]"; 
const char sErrMISS_LBRAC[] = "Missing left-bracket ["; 
const char sErrEMPTY_CODE[] = "No code specified between [...]"; 

// showHaiku error strings
const char shError00[] = ""; // shError default

const char shError01[] = "The file has ended/Your long work is now finished/The world is at rest"; // Normal text file EOF
const char shError02[] = "Where is the data?/The file has suddenly ended/I can work no more"; // unexpected EOF
const char shError03[] = "File will not open/Do you not trust me with it?/Is it too private?"; // cannot open file
const char shError04[] = "The file will not close/It remains open forever/The ages will marvel"; // cannot close file
const char shError05[] = "Frodo told Gandalf/\"I cannot read the letters\"/So I with this file"; // generic read error
const char shError06[] = "Writing is ancient/Sumerian scribes were adept/This program is not"; // generic output problem
const char shError07[] = "The file was closed/But it will not reopen now/Success is fleeting"; // cannot reopen file
const char shError08[] = "File will not open/It remains a mystery/Much like life itself"; // skipping a file in ProcessorClass:: openNextText(void)
const char shError09[] = "What is in a name?/This one must be dear to you/The Bard would wonder"; // attempting to rename file in ProcessorClass
const char shError0A[] = "No actors or verbs?/This is very much like Zen/But I can't code it"; // missing files in ProcessorClass:: readProject(void)

const char shError10[] = "Far too much to learn/My mind is stretched so thin/The end may be near"; // memory warning
const char shError11[] = "The world is finite/Am I not a part of it?/You demand too much"; // out of memory error
const char shError12[] = "In this one sentence/I find surprising detail!/Too much for poor me"; // out of memory error

const char shError21[] = "Please enter a code/You would not want me to guess/I know so little"; // Enter code
const char shError22[] = "One hand is clapping?/Text before the beginning?/You cannot go there"; // Reverse read before start of file

const char shError31[] = "Input was too long/You want far too much from me/I will die instead"; // Instring too long
const char shError32[] = "Input was too long/Why do you ask all this from me?/I will ignore it"; // Instring too long
const char shError33[] = "That is meaningless!/\"Indeterminant error???\"/I am not pleased"; 		// Undocumented error thrown to readNextRecord
const char shError34[] = "Your system command/Could not be implemented/Sorry about that"; 		// system() called returned -1 in ProcessorClass:: readProject

const char shError41[] = "FORWARD was too long/It could remember the past/But not in this case"; // FORWARD string too long
const char shError42[] = "The string I found here/Makes no sense at all to me/I will ignore it"; // string cannot be interpreted

const char shError51[] = "The date I found here/Is outside my time and space/It must be a dream"; // date cannot be interpreted
const char shError52[] = "More than twenty years?!/You see to a distant time!/Alas, I cannot"; // date cannot be interpreted

const char shError61[] = "The record marker/In this XML input/Is too long for me"; // XML ID too long (ReadXMLClass:: getXMLDate)
const char shError62[] = "Where is the date field?/I must know when things happen/This is important!"; // No XML date marker (ReadXMLClass:: openXMLFile(void))

const char shError91[] = "The Evil Empire/Is not an option for us now/Choose Mac or Linux"; // attempt to choose Windows OS
const char shError92[] = "You are too greedy/Or perhaps schizophrenic?/Just one system,please"; // multiple OS
const char shError93[] = "No instructions here?/I cannot decide alone/Please choose a system"; // no OS specified
const char shError98[] = "Nothing you can do/The fault is the programmer's/We are quite sorry"; // deep errors
const char shError99[] = "This is not haiku/Yet employing recursion/There is no problem"; // bailout in showHaiku()

#define MAX_SEASON 3
const char shWinter[MAX_SEASON][96] = {
													"Graceful white snowflakes/Blanket leafless sycamores/I can only code",
													"Outside it is cold/I have been ordered to code/I want an ice storm",
													"Bare-branched maples/Contrast with the green cedars/Me? I am coding"
													};
const char shSpring[MAX_SEASON][96] = {
													"Tulips, daffodils/In spring the world is reborn!/I code forever",
 													"The grass is now green/The air is fresh, the sky blue/All we do is code",
													"Green grass on black earth/Follows flames of prairie fire/We are still coding"
													};
const char shSummer[MAX_SEASON][96] = {
													"It is summer time/And the living is easy/Not for those who code",
													"Long summer evenings/Children run and play football/I just sit and code",
													"The beach beckons me!/Sand, surf, and a cooling breeze/Alas, I code instead"
													};
const char shAutumn[MAX_SEASON][96] = {
													"The forests ablaze/With the colors of autumn/I stare at a screen",
													"The full hunter's moon/Illuminates falling leaves/But here we just code",
													"Above in the sky/Geese are flying to the south/We stay here and code"
													};
#define MAX_INTRO 32
const char shIntro[MAX_INTRO][96] = {
													"My eyes are so sore!/Stop! I will code no longer!/\"You want fries with that?\"",
													"Forty verb patterns?/And then TABARI crashes?/Now I want to cry",
													"Another new day/Same stupid news reports/Will it never end?",
													"Why bother with this?/All we ever analyze/Are two digit codes!",
													"Always the same story/I eat, therefore I must code/Life is so unfair",
													"Each day produces/A thousand brand new stories/We will code them all",
													"Events keep coming/In an unbroken cycle/Forever we code",
													"This is getting old/These haiku are terrible/Must we continue?",
													"Days will come and go/I will continue coding/They call this a life??",
													"The same old story/TABARI is acting up/A new version, please",  // thanks to Lee McMullen, 2005
												  "Haikus are easy/But sometimes they don't make sense/Refrigerator",  // seen on a t-shirt, Dec-09
			// modified from  http://haiku-poems.50webs.com/computer-haiku-poems.htm
													"Stay the patient course/Of little worth is your ire/There is more to code", 
													"Chaos reigns within/Reflect, repent, and recode/Order shall return",
													"A crash reduces/Your expensive computer/To a simple stone",
													"Three things are certain:/Death, taxes and lost data/Guess which will soon occur",
													"You step in the stream/But the water has moved on/The codebook changed",
			// all of the following thanks to Sarah Stacy, 2007
												 	"Without my coffee/Coding is impossible/Another cup please",
													"It\'s commonly said/Only those bored are boring/They know not coding",
													"I code so often/I know all the codes by heart/No one is impressed",
													"I can no longer enjoy/Reading the daily paper/My mind keeps coding",
													"It is 4 o-clock/On a beautiful Friday/I want to go home",
													"The lead was gripping/The first twenty times I read it/Now it is old news",
													"The gentle breezes/Whisper outside the windows/\"Come-hither coder\"",
													"Every now and then/I get a craving to code/What is wrong with me?",
													"This is certain:/TABARI is not my friend/On days like today",
													"One more acronym/Is this a practical joke?/No letters remain",
													"How do these stories/Make it in your newswire?/Shame on you Reuters!",
													"The tick of the clock/Is the only sound I hear/As I code forever",
													"The room is empty/Only I remain coding/Each second dragging",
													"Another Monday/Another week in this place/Woe to us coders!",

													};

/* 

from: http://haiku-poems.50webs.com/computer-haiku-poems.htm

The Web site you seek/Cannot be located, but/Countless more exist.
Program aborting:/Close all that you have worked on./You ask far too much.
Windows NT crashed./I am the Blue Screen of Death./No one hears your screams.
Yesterday it worked./Today it is not working./Windows is like that.
Your file was so big./It might be very useful./But now it is gone.
Out of memory./We wish to hold the whole sky/But we never will.
Having been erased/The document you're seeking/Must now be retyped.
Serious error/All shortcuts have disappeared/Screen. Mind. Both are blank. 
*/
//___________________________________________________________________________________
// define types for various pointers

typedef int toktype;										// type of TokenBuf[] -- should be 32-bit
// ### debug note [06.08.10]: this is being used for *way* too many things -- in particular it is used
//     for both index counters and masks, and possibly some other things. Need to sort that out 
typedef const int ctoktype;
typedef	int *tokptr;                        // pointer to TokenBuf[] -- should be 32-bit words
typedef char *charptr;						// pointer to CharBuf[]
typedef	int tokindex;						// index for tokenBuf[]
typedef	int phrindex;						// index for phraseBuf[]
typedef int  rootindex;						// index of rootArray[]
typedef int  patindex;						// index of PatternArray[]
typedef int  textptr;						// used for stored info in a random access file
typedef char instring[MAX_TAB_INPUT];		// input string length; also used for most other strings  
typedef char errstring[81];					// error message string length  ### [08.07.05] : Magic number here; correct this 
typedef char litstring[LIT_SIZE];			// literal string length
typedef char filestring[SFILE_SIZE];		// file name string length; may need to be longer 
typedef	long commloc;						// type used for Comment file position indices

// distinctive initialization constants
const toktype INIT_TOKTYPE = 0xBABA;						// max number of files in AutoFile list	

// enum's -- see documentation in parser.cp
enum wordtype 
{	Null, Actor, Agent, Verb, Time, Attrib, Determ, Noun, 
  Adjctv, Auxil, Byword, Comma, Pronoun, Conj, Prep, Plural, Number, 
  Issue, Synym, 
  Clause, Compound, Reference, Subord, Replace, NullTag,
  Halt};  // Note: "Halt" should be the final type in order for asserts to work

//___________________________________________________________________________________
// 	                                                                   global structs																															   

struct litStruct {   	// directory for literals
	charptr		pchar;   	// pointer to text in charBuf				
	int			length;		// number of chars in text
	toktype 	istart;  	// index of litBuf list of phrases that begin with this literal 
	toktype 	ifreq;	 	// index of litBuf list of freq tags for the literal	
	litStruct *pnext; 		// pointer to next literal in alphabetic list	
}; 

struct patStruct {  	// primarily used in PatternsClass
	tokptr 			phrase;   	// index of phrase list in Literals.litArray			
	toktype 		icode;    	// location of codes: see codes.cp for details 
	int				length;		// length of phrase; used to determine order of evaluation
	int				maxalt;		// length of longest string inside any alternative pattern
	int				used;		// number of times used
	commloc 		icomment; 	// filepos for comments 
	patStruct*pnext;    // next pattern record; NULL if none	
}; 

struct issueListStruct { 	// issue list elements: primarily used in CoderClass
	toktype icode;					// codeArray index
	issueListStruct * pnext;// next list item
};

struct issueHeadStruct {  // start of issue lists
	int 			kategory;			// category number
	charptr			phrase;    		// pointer to category text in charBuf				
	toktype 		icode;    		// location of default codes
	bool 			doAll;       	// record all occurrences?
	bool 			doNumber;			// process as a NUMBER issue?
	bool 			found;				// has a code in this category been found?
	issueHeadStruct * pnext;// next header
	union {
		issueListStruct * plist;// first list item for coded issue
		int total;							// total for numeric issue
	};
};

struct freqStruct {  			// freq list elements
	int 			kategory;			// category number
	charptr		phrase;    		// pointer to category text in charBuf				
	charptr		abbrev;    		// pointer to abbrev text in charBuf				
	freqStruct * pnext;			// next element
};

//___________________________________________________________________________________
// 																													Global utility functions

void IncrementIndex(toktype &index, int max, const char *sloc);// increment index, check against max for overflow.
void CheckIndex(toktype index, int max, const char *sloc); // check against max for overflow without incrementing.
char * Copy2Chr(char *s1, char *s2, char c);  // copies s2 into s1 up to but not including c
void TrimEndBlanks(char *s);		// trim terminal white space from the string
void TrimLeadBlanks(char *s);		// trim initial white space from the string
void TrimBlanks(char *s);				// trim initial and terminal white space from s
char * Int2Str(char *s,int k);		// converts k to left-justified string
long MakeJulian (char *s);			// convert string to Julian-1904 date
void JulianString (char *s, long jdate);	// Julian-1904 date to string
void MakeTimeDateStr (char *stime, char *date);	// puts formatted date and time in string

//___________________________________________________________________________________
// 																												Global interface functions

# define WRITECHAR(c)		addch(c)  // <12.01.19> these aren't a good idea and are left over from the transition to ncurses
# define WRITESTRING(s)		addstr(s)
# define WRITEEOL()		  	addch('\n')

// interface-dependent routines (NCURSES vs <iostream>)
void ClearScreen(void);
void ResetCurs(void); 			// refresh screen, then get cursr, cursc in ncurses mode
char getUpper(void);				// returns uppercase keystroke
void doKeyMode(void);				// swaps the Linux key mode; called from O)ther menu
void ReadLine(char *s); 		// reads s from keyboard
bool safeGetLine(char *s);	// gets a line of text, checking that buffer isn't exceeded
void WriteNewLine(const char *s);	// write s to display on new line
void WriteLine(char *s);  						// write s to display
void WriteLine(const char *s);  			// write s to display
void WriteLine(char *s1, char *s2);		// write s1 and s2 to display
void WriteLine(char *s1, const char *s2);	// write s1 and s2 to display
void WriteLine(const char *s1, char *s2);	// write s1 and s2 to display
void WriteAlert(const char *s1);			// beep, then write s1 to display
void WriteAlert(const char *s1, char *s2);	// beep, then write s1 and s2 to display
void WriteLong(const char *s1, long i);			// write s1, integer and \n without refresh
void WriteFloat(const char *s1, float r);		// write s1, float and \n without refreshy
void GetInput(char *s, const char *sprompt);			// write sprompt, then read s from keyboard
bool GetAnswer(char *sprompt, char cyes, char cno);		// writes sprompt, reads a character, returns true if this is upper or lower cyes
bool GetAnswer(char *s1, char * s2);	// two string version; 'Y' is affirmative
void Pause(void);											// wait for char; exit if Q
void showHaiku(const char * shaiku);				// Display a haiku string
void setupMenu(bool hasinit);					// move menu to a specific location on screen
void showMenu(const char *s);			// display menu prompt
void correctMenu(const char *s);	// prompts with correct response, returns to previous line

bool PhraseError (char *stin, char *sreason);
void ShowWarningError (const char *sreason, const char *sresponse, const char *shaiku);		// Announces an error, then returns
void ShowWarningError (const char *sreason, char *s2, const char *sresponse, const char *shaiku);// Same as ShowWarningError, but concatenates two strings
void ShowFatalError (const char *sreason, const char *shaiku);						// Announces an error, then calls exit(1)
void ShowFatalError (const char *sreason, char *sfile, const char *shaiku);

bool ReadArg(int argc, char *argv[], char *sfilename);		// Process command line arguments
int RecordMenu(void);				// Menu for the primary read-parse-code loop
int OtherMenu(bool hasProb);// Menu for the O)ther option in doProcess
void writeIntro(void);			// Writes the introductory screen
void ShowAutoHelp(void);		// Autocoding help screen
int  AutocodeSetup(void);	// defined in interface.cp
void ShowAutoHelp(void);	// Autocoding help screen (also in interface.cp)

//___________________________________________________________________________________
// 																																Class declarations

class TabariFlagsClass;		// just keeps all of the flags in one place
class ReadXMLClass;				// Read XML input
class ProcessorClass;			// Primary processing routines
class CharStoreClass;			// handles all character string storage
class TokenStoreClass;		// handles all numerical storage
class CodeStoreClass;			// handles code storage
class PhrasesClass;				// phrase list storage
class LiteralsClass;			// pattern storage
class RootsClass;					// Root storage
class CommentFileClass;		// handles the temporary storage of comments and file tails in a random-access file
class ReadFilesClass;			// Input routines
class WriteFilesClass;		// output routines
class MergeFilesClass;		// Dictionary comparison routines
class ParserClass;				// Parsing routines
class CoderClass;					// Coding routines
class ModifyClass;				// Dictionary modification routines

//___________________________________________________________________________________
// 																																External prototypes

class TabariFlagsClass		
// just keeps all of the flags in one place
// a.k.a. "the class formerly known as GlobalFlags", except that Windows
// already has a class called "GlobalFlags."
{
	public:
	bool f4DigitYear;   // dates have four-digit years
	bool fIgnoreConj;   // code across multiple clauses	
	bool fSameActors;   // allow source and target to be identical	
	bool fwriteEvents;  // write events to a file
	bool fShowHaiku;		// if false, do not show error haikus
	bool fWarnDups;			// issue warning when duplicate roots are found
	bool fhasIssues;		// Issues are being coded
	bool fhasFreqs;			// Freqs are being coded
	bool fhasAttribList;// <ATTRIB> list in verbs file  #### <09.01.07> these have not been adjusted for multiple files
	bool fhasTimeList;	// <TIME> list in verbs file
	bool fFBISMode;			// FBIS mode -- affects reading of headers, treatment of brackets, etc.
	int	 fAutoCodemode; // command line auto-coding mode information
	int	 fCheckSkip;    // ask whether to skip previously coded records
	int	 fConvAgent;        // convert isolated agents to actors
	int	 fActorUsage;		// 3-way switch for saving actor usage files
	int	 fAgentUsage;		// 3-way switch for saving agent usage files
	int	 fVerbUsage;		// 3-way switch for saving verb usage files
	int  fNonEvents;		// create records for non-events: 2 => output nonevents only
	int cursr; 				// current cursor row: only active in NCURSES mode
	int cursc; 				// current cursor column
	int nlinesread;			// number of lines read in current file
	
	TabariFlagsClass()
	// constructor
	{ 
		f4DigitYear 	= false;
		fIgnoreConj 	= false;
		fSameActors 	= false;
		fwriteEvents 	= false;
		fShowHaiku  	= true;
		fWarnDups     	= true;
		fhasIssues 		= false;
		fhasFreqs 		= false;
		fhasTimeList 	= false;
		fhasAttribList	= false;
		fConvAgent      = false;
		fFBISMode     	= false;
		fNonEvents   	= 0;
		fAutoCodemode 	= -1;
		fCheckSkip 		= true;
		fActorUsage   = OFF_3WAY;
		fAgentUsage   = OFF_3WAY;
		fVerbUsage    = OFF_3WAY;
		cursr = 0; // current cursor row
		cursc = 0; // current cursor column
		nlinesread = 0;		// number of lines read in current file
	}


	~TabariFlagsClass(void)
	// destructor
	{ 
		endwin(); // terminate curses
	}
	
}; // TabariFlagsClass

//___________________________________________________________________________________

class ReadXMLClass		// Read XML input
{
public:		
	int setXMLType(char * sLine); // evaluate the <text XML="..."> record
	void SetXMLTags (int index); // reset the tags to the appropriate XML format
	bool openXMLFile(char * sfilename);  // open XML file, get first record ID and date
	void closeXMLFile(void); // closes the XML text file
	void skipXMLRecords(void);	// skips nLast records.  ### still needs to be implemented
	void readXMLRecord(void);	// get next record in file
	void skipXMLRemainder(void);	// skip remainder of a long story
	bool getXMLDate(void);			// get the sequence and date fields, read to the sTextTag marker
	char * startSent(void); 			// find start of next sentence
		
ReadXMLClass ()
// constructor
{
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
	
	inReplace = false;
  indexCurrent = 0;	
  pLast = NULL;
  
	strcpy(sBBNTag,"BBN");
	indexBBN = 1;
	strcpy(sVeridTag,"Verid");
	indexVerid = 2;

} // constructor

 private:
	ifstream fin;			// text input file
	char sLine[MAX_XML_INPUT];		// input string
	bool inReplace;   // marking a replace field
	char * pSent;			// used to transfer info to Processor.sentText 
	char * pLast;				// holds point in sLine where previous sentence ended
//	int idxField;
	int idxText;
	
	int indexCurrent;  // XML type currently active  

	char sDateLine[10]; 	//"INFODATE:";
	char sRecIDLine[10]; 	//"SERIAL:";

	char sTextTag[7];	// "<TEXT>";
	char sTextEnd[8]; 	// = "</TEXT>";
	char sSentTag[4];	// = "<S ";
	char sSentEnd[5]; 	// = "</S>";
	char sSentFld[3]; 	// = "ID";
	char sNounTag[5]; 	// = "<NE ";
	char sNounEnd[6]; 	// = "</NE>";
	char sPronTag[10]; 	// = "<PRONOUN ";
	char sPronEnd[11]; 	// = "</PRONOUN>";
	char sNounFld[4]; 	// = "WDN";
	char sBracTag[10]; 	// = "<BRACKET>";
	char sBracEnd[11]; 	// = "</BRACKET>";
	char sIgnrTag[10]; 	// = "<IGNORE>";
	char sIgnrEnd[11]; 	// = "</IGNORE>";
	
	char sBBNTag[4];    // = "BBN"
	int  indexBBN;
	char sVeridTag[6];    // = "Verid"
	int  indexVerid;

	void checkLength(int incr, char * pst); // increment idxText; throw STORY_LONG if too long
	void insertReplace(char * pst); // inserts a replacement marker into pSent 
	char * gotoEnd(char * pst); // skip to the end of a tag
	char * doNE(char * pst);
	char * doPRONOUN(char * pst);
	char * doBRACKET(char * pst); // skip over everything in <BRACKET>...</BRACKET>
	char * doIGNORE(char * pst);	// skip over everything in <IGNORE>...</IGNORE>

	void writeXMLInfo(void); // ### debugging output
//	void writeVerbsMissing(void);

}; // ReadXMLClass

//___________________________________________________________________________________


class ProcessorClass		// Primary processing routines
{
public:		

	char sentText[MAX_SENTENCE]; 	// original text
	char sRecDate[MAX_DATE_LEN];	// record date
	char sRecID[MAX_RECID_LEN];		// record ID
	char sDocID[MAX_DOCID_LEN];		// document ID
	long julianDate;						// julian date
	int iPauseStatus;							// pause after coding?
	filestring scommNew;					// code + date string
	ofstream fprob;		// problems output file; ### currently needs to be public for ShowParsing
	bool fopenError;							// error file is open
	int curDict;                  // index of dictionary currently being read
		
	void doProcess(char *sprojectname); // driver function
	void changeDict(int iDict, wordtype wtype);	// sets curDict, increments changes count
	void showRecord(void);							// display the record on the screen with date and ID.
 	void validateDate(char *ps);				// checks that string beginning at ps is a valid date

	void writeProblem(void);		// write information to problems file
	void writeEvents(void);		// write coded events to file
	void showEvents(void);		// display coded events and write to event file
	void showMemory(void); 		// shows the current memory use
	void readRecord(bool forward);		// read text record
	void writeError(const char *s, char *s1, int error); 		// writes to the error file

	friend bool ReadXMLClass:: getXMLDate(void);
	friend int ReadXMLClass:: setXMLType(char * sLine);
	friend char * ReadXMLClass:: startSent(void);
	friend class WriteFilesClass; 
	friend class ReadFilesClass; 
	friend class CommentFileClass; 
	
	ProcessorClass()
	// constructor
	{ 
	
		projectFile[0] 	= '\0';
		optionsFile[0] 	= '\0';
		eventFile[0]  	= '\0';
		probFile[0]	  	= '\0';
		errorFile[0]	= '\0';
		issueFile[0]	= '\0';
		freqFile[0]		= '\0';

		textFile[0]    = '\0';
		textFileList.fileName[0] = '\0';

		strcpy(sXMLField,"XML");
		strcpy(sCoder,"");
		strcpy(sRecEngDate,"           ");
		strcpy(sMonths,"???JanFebMarAprMayJunJulAugSepOctNovDec");

 		nextFile = NULL;
 		nextDict = NULL;
		curDict  = 0;
 		nLast = 0;
 		nSession = 0;	
 		nRecords = 0;	
 		nActorChanges = 0;	
 		nVerbChanges = 0;	
		totalEvents = 0;
		irecback = 0;
		for (int ka = 0; ka < MAX_REC_BACK; ++ka) backrec[ka] = -1;
		
		lastSeq = -1;  // indicates new story
		nSeq    =  0;
		slastRecID[0] = '\0';	
 		sRecDate[0]   = '\0';	
 		sRecID[0] 	  = '\0';	
 
		fusingXML = false;
		fhasDocs  = false;
		fopenError= false;
		fhasRun   = false; 

		iPauseStatus = false;			// pause after coding?

//		cTimeShift = '*';			// time-shift marker
//		pTimeModList = NULL;  // start of time shift list

		fwroteProb = false;   // wrote to the problems file; don't delete it
		
		fchangeDict   = false;
		firstActorDict = 0;  
		firstAgentDict = 0;  
		firstVerbDict  = 0; 
	}
	
	~ProcessorClass()
	// destructor
	{
//		delete pTimeModList;
	}

private:
	struct textListStruct { // linked list for storing multiple text file names
		filestring fileName;
		int indexXML;				// type of XML formatting
		textListStruct * next;
	} textFileList, *nextFile;

	struct dictListStruct { // linked list for storing multiple dictionary file names
		filestring fileName;
		int index;				// index used to keep track of roots
		wordtype dictType;				// type of dictionary
		bool isFlat;      // flat dictionary?
		bool hasNounList;      
		bool hasAdjctList;  // ### <09.01.07> also need the equivalent -- union? -- for time and attribut
		commloc tailLoc;  // location and number of comments at end of file
		int     tailLen;
		int nChanges;  
		dictListStruct * next;
	} dictFileList, *nextDict;
	int iDict;         // index of dictionary         

	ofstream fout;		// event output file
	ifstream fin;			// text input file
	ofstream ferror;	// error output file
	
	filestring projectFile;	
	filestring optionsFile;
	filestring eventFile;
	filestring probFile;
	filestring issueFile;
	filestring freqFile;
	filestring errorFile;
	
	tokindex tokText;		// start of list of text files
	filestring textFile;   // textfile currently being processed

	char sRecEngDate[MAX_ENGDATE_LEN];	// English version of record date
	char slastRecID[MAX_RECID_LEN];	// previous record ID
	char sXMLField[6];	// ="\"XML\"" ; possible field in <textfile>

	instring 	sLine;		// input string
	litstring 	sCoder;	// coder id
	litstring 	sStart;	// starting time
	litstring 	sDoc;	  // documentation header
	int 		nSession;			// number of coding sessions
	long 		nRecords;		// number of records coded this session
	int 		nActorChanges;			// number of changes made this session
	int 		nVerbChanges;			// number of changes made this session
	long		nLast;				// last record coded
	
	int didError[MAX_ERROR];  // record of errors already reported by  writeError
	
	bool	fchangeDict;	// change was made in at least one dictionary
	int firstActorDict; // first flat actor dictionary: new entries stored here
	int firstAgentDict; // first flat actor dictionary: new entries stored here
	int firstVerbDict;  // first flat verb dictionary: new entries stored here
	
	commloc backrec[MAX_REC_BACK]; // circular array of previous records
	int			irecback;							// index of same
	long		totalEvents;		// total events coded in session
	bool		fwroteProb;   	// wrote to the problems file; don't delete it
	
	bool fusingXML;		// using XML file
	bool fhasDocs;    // project file has <dochead> and/or <doctail> tags
	bool fhasRun;			// project file has a <run> command

	int nSeq;						// sequence number	
	int lastSeq;				// of previous sentence sequence number; set to -1 on new story	

	char sMonths[40]; // month abbreviations

/*	struct timeModListStruct { // linked list for storing out of sequence records
		toktype julian;		// Julian date of records
		commloc floc;				// location in file
		int			nrec;				// number of records
		timeModListStruct * next;
	} *pTimeModList;
*/	
//	char cTimeShift;	// marks record as time-shifted

	void getsLine(void); 			// safe version of getline: throws LINE_LONG exception if line is too long
	void getsLineError(int err); // processes errors returned from getsLine
	void addDictionary(char *sfilename, wordtype type); // add dictionary to the dictList
	void readProject(char *sfilename);	// read project file
	bool openNextText(void); 	// opens the next text file; returns false when list is finished
	void closeText(void); 		// closes the text file
	void openError(void); 		// opens the error file
	void closeError(void); 		// closes the error file
	void writeDoc(void); // writes the documentation header to the event file
	void initSession(void);	 	// initialize coding session
	void writeProject(void);	// write project file
	void writeUsage(void); 	// writes .use files
	void backFiles(void);			// writes .save files
	void endSession(void); 		// end the coding session
	void reverseRecord(int irev); // reverses the read
	void skipRecords(void);		// skips nLast records
	void skipRemainder(void);	// skip remainder of a long story
	void makeEnglishDate(void);	// Converts sRecDate to an English form
	void doOther(void);				// handles the O)ther option
	void writeIDs(int loc, int len);	// write ID strings to event file fout
	void writeProbIDs(int loc, int len);
	void writeXMLIDs(void);		// write ID strings to event file fout for XML files
	void showFreqs(void); // display the freqs for the record. 
	void autoCode(void); 			// autocoding option
	void nextTextFile(char *s); // gets the next string containing the next text file name
	void showDate(void); 			// display only date, ID and records coded
	bool readNextRecord(void);	// read next record
	void doFinalSysCalls(void);	// handles the <system> calls following <run>

// deactivated in version 0.8+
//	void addTimeModList(void);	// adds next record to the TimeModList at the appropriate point based on ijulian
//	void showTimeModList(void); // displays current TimeModList ### DEBUGGING
//	void writeTimeShift(timeModListStruct * ptime); // writes a set of time-shifted records
//	void sortTimeOutput(void);	// pulls out the time-shifted records, then merges them into the correct order

	void doValidation(void); 	// validation option
	void checkEvents(void); 	// validation check

}; // ProcessorClass

//___________________________________________________________________________________

class CharStoreClass		// handles all character string storage
{
	public:
	charptr charNull;		// used to initialize pointers to something meaningful
	charptr charNullString;		// used to initialize pointers to something meaningful

	charptr putCharStr (char* str);
	
	friend class LiteralsClass;
	friend void ProcessorClass:: showMemory(void);

	CharStoreClass()
	{ 
		charBuf[0] = '\0';  // first cell is unused
		pCharLast = charBuf;
		charNull = putCharStr((char *)"*null*");
		charNullString = &charBuf[0];
	}

	private:
	charptr pCharLast; // last active cell in charBuf
	char charBuf[MAX_CHAR];

}; // CharStoreClass

//___________________________________________________________________________________

class TokenStoreClass		// handles all numerical storage
{
	public:
	toktype iToken;    // ### this is needed in CodeStoreClass::storeActorCode; just make it friend?
	TokenStoreClass()
	{ 
		tokenBuf[0] = 0;  // first cell is unused
		iToken = 1;
	}

	tokptr getTokPtr(tokindex index); // gets the pointer to TokBuf[index]
	toktype getToken (tokptr loc);
	tokindex putToken (toktype tokitem);
	toktype getTokenValue (tokindex index);	// returns the value of tokenBuf[index]

	friend class RootsClass;

	private:
	toktype  tokenBuf[MAX_TOKENS];

}; // TokenStoreClass

//___________________________________________________________________________________

class CodeStoreClass		// handles code storage
{
public:
	int indexNull;
	int indexComplex;
	int indexDiscard;
	int indexBlank;
	
	int codeElite;
	
											// function prototypes
	char* getCodePtr (toktype itok);
	char* getCodeLabel (toktype itok);
	char* getAgentCodePtr (toktype tokindex); // Gets the pointer to the string of agent code at index tokindex
	toktype findCode (char *scode);
	toktype addCode (char *scode);
	toktype addCode (const char *scode, char *stext);
	toktype addCode (char *scode, char *stext);
	toktype storeCodeString (char *sCode); // processes the sCode string
	void parseCodeInput (char * scode, char *s); // convert s into the sCode format, handling date restrictions 
	void addNullString (char *s); // checks for a solitary dated code; adds null code as default
	toktype storeAgentCode (char * s); // stores agent codes
	toktype storeActorString (char *sCode);
	void nextActorCode (toktype &idxactor, toktype &idxagent, tokptr *pnext, toktype istart); //	Gets the next actor and agent codes from the optionally dated actor code sequence pointed 
	toktype storeTimeCode (char * s); // Stores time codes
	int getTimeCode (toktype itok, bool &isIndex);	// Get time code. 
	void getTimeString (char *s, toktype itok); // Get the string version of the time code pointed to by itok
	toktype storeEventString (char *sCode);
	toktype storeIssueString (char *sCode);
	void getActorString (char *s, toktype itok);
	void getEventString (char *s, toktype itok);
	void getIssueString (char *s, toktype itok);
	void decodeEventCode (toktype &evtcode, bool &fissub, bool &fisdom, toktype icode); // Gets the code and status out of icode
	void getEventCode (toktype &icode, toktype &ipair, toktype itok);  // Gets the primary and optional paired code of the event code sequence pointed to by itok
	void codeTest(void); 

	friend void ProcessorClass:: showMemory(void);

	CodeStoreClass()
	{ 
		extern CharStoreClass CharStore;  

		codeArray[0].pcode = CharStore.charNullString; ;  // first cell is unused
		codeArray[0].ptext = CharStore.charNullString; ;
		iCode = 1;

	// masks for checking code flags
		maskContinue  = 0x20000000; // continuation bit
		maskContnOff  = 0xDFFFFFFF; // turn off continuation bit
		maskType      = 0xD0000000; // 2-bit header
		maskConstrnt  = 0x18000000; // isolates the date constraint
		maskLTCntsrt  = 0x08000000; // < date constraint
		maskGTCntsrt  = 0x10000000; // > date constraint
		maskINCntsrt  = 0x18000000; // - date constraint
		maskDateInit  = 0xA0000000; // date header with continuation on
		maskDateHead  = 0x80000000; // date header field
		maskDateSkip  = 0x01FFFFFF; // skip words in date header
		maskSpecHead  = 0x40000000; // special header
		maskActrPair  = 0x40000000; // actor pair
		maskActrStop  = 0x00000000; // single actor 
		maskEvntCode  = 0x0FFFFFFF; // event code
		maskEvntFlag  = ~maskEvntCode; // event flags
		maskActrCode  = 0x1FFFFFFF; // actor, agent code
		maskActrFlag  = ~maskActrCode; // actor, agent flags
		maskEvtSubor  = 0x40000000; // subordinant code
		maskEvtDomin  = 0x80000000; // dominant code
		maskPairNext  = 0x10000000; // paired code follows
		maskIssuNext  = 0x20000000; // issue code follows
		maskSTANext   = 0x30000000; // STA code follows
		maskEvntSrc   = 0x00000000; // STA source code 
		maskEvntTar   = 0x40000000; // STA target code 
		maskEvntAtt   = 0x80000000; // STA attribution code
		maskTimeCode  = 0x40000000; // Time code field
		maskTimeDecr  = 0x80000000; // Decrement time
		maskTimeClr   = 0x3FFFFFFF; // Clear time fields

															//  set the core codes
		indexNull 	 = addCode(sNullCode,(char *)"Null code");
		indexComplex = addCode(sComplexCode,(char *)"Complexity code");
		indexDiscard = addCode(sDiscardCode,(char *)"Discard code");
		indexBlank   = addCode(sBlankCode,(char *)"Blank code");
		
		codeElite   = storeAgentCode((char *)"[~ELI]");  // handles "former <agent>" conversion

	} // CodeStoreClass constructor


private:
	// masks for checking code flags : see constructor for use
	toktype maskContinue, maskContnOff;
	toktype maskType;
	toktype maskConstrnt, maskLTCntsrt, maskGTCntsrt, maskINCntsrt;
	toktype maskDateHead, maskDateInit, maskDateSkip;
	toktype maskSpecHead;
	toktype maskEvntCode, maskEvntFlag;
	toktype maskActrCode, maskActrFlag;
	toktype maskActrPair, maskActrStop;
	toktype maskDate;
	toktype maskEvtSubor, maskEvtDomin;
	toktype maskEvtPair;
	toktype maskPairNext, maskIssuNext, maskSTANext;
	toktype maskEvntSrc, maskEvntTar, maskEvntAtt;
	toktype maskTimeCode, maskTimeDecr, maskTimeClr;

	toktype iCode;
	struct Code_Rec{
		charptr pcode;    // location of code string in CharBuf }
		charptr ptext;    // location of label in CharBuf }
	} codeArray[MAX_CODES];

	bool fParseError;				// true if there was a parsing error
	errstring sParseError;	// and this gives the reason

	toktype storeCode (char *scode, toktype mask);	// cleans blanks, get the codeindex, &'s index with mask
	toktype makeCode (char *scode, toktype mask);		// same cleaning as storeCode, except it doesn't store 
	void appendEventActor (char * sparse, char *s, const char *sc); // handles an actor $(...) substring, appends to sparse.
	void parseActorString (char * sparse, char *s);	// parsing routines
	void parseIssueString (char * sparse, char *s);
	void parseEventString (char * sparse, char *s);
	toktype storeActorCode (instring s); // parses and stores actor codes
	void putDate(char *s); 										// transfers a date beginning at s into TokenBuf
	void setDateSkips(toktype datehead); 			// set skip fields in a sequence of date headers 
	toktype storeDatedString (char * s); 		// creates a date header, then stores codes
	void getActorCode (char *s, tokptr ptok); // gets undated actor code sequence

}; // CodeStoreClass						

//___________________________________________________________________________________

class PhrasesClass   // phrase list storage
{
public:

//	toktype connSpace, connWild;
	toktype connFullNext, connPartNext, connFullSkip, connPartSkip;
	toktype connEqual;
	toktype flagSynm,flagSynmFS,flagSynmPN;

	tokptr store(char *sphrase);
	void make(toktype tokar[], char *s); // store s in tokar but not in phraseBuf
	char * get(char *sphrase, tokptr phrase);

	friend void ProcessorClass:: showMemory(void);

	PhrasesClass()
	{
		// ### some harmless telltale needs to be put in the zero cell
		iPhrase = 0;
																	//	checkPattern connectors
		connFullNext  = 0x0000;	// exact match and consecutive
		connPartNext  = 0x4000;	// partial match and consecutive
		connFullSkip  = 0x0400;	// exact match and skip
		connPartSkip  = 0x4400;	// partial match and skip

		connEqual	  = 0x0010;   // do not add regular endings (version 0.7)
        
        flagSynm 	  = 0x0002;     // mask for prior token is a synset root index (version 0.8)
        flagSynmFS 	  = 0x0402;     // prior token is a synset root index with open match 
        flagSynmPN 	  = 0x4002;     // prior token is a synset root index with consecutive match 

		valEndList = 0;					// end of list		

	} // constructor
		
private:
	phrindex iPhrase;	
	toktype phraseBuf[MAX_PHRASES];
	toktype valEndList;
			
	void add2Phrase(toktype index,toktype marker);   

}; // PhrasesClass

//___________________________________________________________________________________

class LiteralsClass   // pattern storage
{
private:
	toktype iLit;
	toktype iLitBuf;

	litStruct litArray[MAX_LITERALS];
	
	litStruct *litIndex[MAX_LIT_HASH];  // hashed index to litArray
	toktype litBuf[MAX_LITBUF];  // storage of istart lists
	
	int iHash(char *s);  						// hash into litIndex
	toktype newLiteral(void);  			// create new literal entry
	toktype litSub(char *s1, char *s2); // look for substring
	
public:
	toktype kountFreq[MAX_FREQ];  // individual freq counts		

	void storeFreq(char *s, int kat);	// adds a freq index to literal in s, creating the literal if needed
	void countFreqs(toktype * ptok); // counts freq codes for literal list pointed to by toklit ( = Parser.lexArray[index].ilit)

	void alphaList(void); 	// *** debugging -- produces alphabetical listing
	void cellList(char *s); // *** debug function
	void cHash(char &ca, char &cb, int i);		//  *** debugging: returns characters that would generate hash i
	bool find(litStruct **plit, char *s); 		// looks for s in literals
	toktype add(litStruct *plit, char *s);		// add s to literals following a find
	void create(char *s);											// do find, then add s to literals
	bool check(char *s); // looks for s or part of s in literals; puts the successful matches in Parser.literaList
	void addStart(tokptr ptok, toktype iRoot);// adds a root to the start list of the first literal in phrase
	toktype getIndex(litStruct *plit);    		// gets the index for a pointer
	bool duplicateRoot(char *s);		// checks whether s already exists as a root
    bool duplicateSyn(char *s);    // checks whether s already exists as a synset designator
	friend class PhrasesClass;
	friend class ParserClass;
	friend class CoderClass;   // *** debug
	friend class WriteFilesClass;
	friend class MergeFilesClass;
	friend class ModifyClass;			
	friend void ProcessorClass:: showMemory(void); 
//	friend void MemTest(void);  // *** debug

	LiteralsClass()
	{ // this needs to be called before Roots constructor
		int ka;
		extern CharStoreClass CharStore;  

												// initialize zero cells
		litArray[0].pchar   = CharStore.charNullString; 
		litArray[0].istart  = INIT_TOKTYPE; 
		litArray[0].ifreq   = INIT_TOKTYPE; 
		litArray[0].pnext   = NULL; 
		
		litBuf[0] = 0xDADA;  // initialize to something we might recognize
		litBuf[1] = 0xDADB;
	
		iLit = 0;
		for (ka=0; ka < MAX_LIT_HASH; ka++) litIndex[ka] = NULL;
		iLitBuf = 1;
		
		// store the basic pattern literals : DO NOT CHANGE THE ORDER HERE!! (unless the *_LIT constants are also changed.  But don't...)
		create((char *)",");		// forces the comma to have index COMMA_LIT   = 1
		create((char *)"#");		// index 2 will be numbers NUM_LIT   = 2
		create((char *)"*");
		create((char *)"$");
		create((char *)"+");
		create((char *)"@");
		create((char *)"%");
		create((char *)"^");
		create((char *)"{");
		create((char *)"|");
		create((char *)"}");
		
		for (++iLit; iLit<SYMBOL_LIMIT; ++iLit) { // store remaining literals at indices above LIT_SYMB
			litArray[iLit].pchar   = CharStore.charNull;
			litArray[iLit].istart  = INIT_TOKTYPE; 
			litArray[iLit].ifreq  = INIT_TOKTYPE; 
			litArray[iLit].pnext   = NULL; 
		}
		iLit--;  // first non-symbol will be stored at SYMB_LIT				

	} // constructor

}; // LiteralsClass

//___________________________________________________________________________________

class RootsClass		// Root storage
{
public:

	void storeActor(void);
	void storeAgent(void);
	toktype storeVerb(void); // store the current contents of InPhrase as a Verb in rootArray
	void storeVerbForm(toktype idxroot); // store alternative form of verb
	toktype storeTime(void);
	toktype storeAttrib(void);
	void storeIssue(int kat, bool isNumeric);
	void storeWord(char *sphrase, wordtype wtype); // store sphrase as an wtype in rootArray
	void storePattern(toktype idxroot, wordtype wtype);
	void storeSyns(toktype idxroot, wordtype wtype);

	toktype litAND; // literal indices for various words
	toktype litNOR;
	toktype litHE;
	toktype litTHE;
	toktype litIT;
	toktype rootFORMER;
	toktype rootDefActor;
	bool isNullVerb(rootindex iroot); 		// determines if rootArray[iroot] has a null primary code
	bool isNullActor(rootindex iroot); 		// determines if rootArray[iroot] has a null primary code after date restrictions
	bool isDiscardCode(rootindex iroot);	// determines if rootArray[iroot] has a discard primary code

//	friend void MemTest(void); // *** debugging function
	friend class LiteralsClass;
	friend class MergeFilesClass;			
	friend class ParserClass;			
	friend class CoderClass;			
	friend class ModifyClass;			
	friend class ProcessorClass;
	friend class WriteFilesClass; 
	friend class ReadFilesClass; 
		
	RootsClass()
	{ 
		iRoot = 0;  // note that verb root index of 0 signals a non-event
		iPattern = 0;
		// #### this doesn't look like a good idea...point it to the patArray[0] and put something there
		patArray[0].phrase 	 = (tokptr) 0xEAEAEAEA;				
		patArray[0].icode	 	 = 0;  // this can be used to generate non-events
		patArray[0].icomment = 0;
		patArray[0].pnext 	 = NULL;
		
		storeType("A_",Determ); 
		storeType("AN_",Determ);
		storeType("THE_",Determ);
		litTHE = *(rootArray[iRoot].phrase);  // shortcut for plural pronouns
		storeType("AND_",Conj);
		litAND = *(rootArray[iRoot].phrase);
		storeType("NOR_",Conj);
		litNOR = *(rootArray[iRoot].phrase);
		storeType("BUT_",Conj);
		storeType("OR_",Conj);
		storeType("IN_",Prep);
		storeType("OF_",Prep);
		storeType("HE_",Pronoun);
		litHE = *(rootArray[iRoot].phrase);
		storeType("HIM_",Pronoun);
		storeType("SHE_",Pronoun);
		storeType("HER_",Pronoun);
		storeType("IT_",Pronoun);
		litIT = *(rootArray[iRoot].phrase);
		storeType("THEM_",Pronoun);
		storeType("THEIR_",Pronoun);  // ### is this right? -- it's the possessive...
		storeType("THEY_",Pronoun);
		storeType("WERE_",Auxil);  // auxilary verbs for passive voice
		storeType("WAS_",Auxil);
		storeType("BEEN_",Auxil);
		storeType("BY_",Byword);

		storeType("FORMER_",Adjctv);  // handles "former"<agent> conversion
		rootFORMER = iRoot;
		storeType("DEFAULT_ACTOR",Actor);  // stores a string for the default actors
		rootDefActor = iRoot;
        
        needcodes = false;
		
	} // constructor

private:

	struct rootStruct {
		tokptr	 	phrase;   // pointer to phrase list
		wordtype 	wtype;  	// type 
		patStruct* 	ppat;	// pointer to pattern list; first pattern carries root code 
		int			used;			// number of times used
		int        	iDict;    // source dictionary index
		tokindex 	iclass;  	// for verbs: 0 = no additional forms; + => highest index of forms; - => verb form
												// for actors: 0 = parent root; + = index of parent of synonym
												// for issues: category
	} rootArray[MAX_ROOTS];
	toktype iRoot;
			
	patStruct patArray[MAX_PATTERN];
	toktype iPattern;
	
	instring sdaterest; // date restrictions
	bool needcodes;     // need date restrictions on previous entry
	tokindex actParentIdx; // index of current actor parent root

	patStruct * newPattern(void);		// initializes new patArray cell   
	void initRoot(void);						// Handles the standard rootArray[iRoot] initializations
	void storeType(const char *sphrase, wordtype wtype);			// store sphrase as an wtype in rootArray
	void lengthPattern(int &len, int &alt);							// computes the length of the pattern in sPhrase

}; // RootsClass

//___________________________________________________________________________________

class CommentFileClass		
// handles the temporary storage of comments and file tails in a random-access file
{
public:

	CommentFileClass()
	// constructor
	{ 
		strcpy(sfcommname,"Comments.temp");		
		fcomm.open(sfcommname,ios::out); 
		if (fcomm.fail()) ShowFatalError("Unable to open file for storing comments; disk may be full or locked",shError03);
	}

	~CommentFileClass()
	{ // close the file and erase it
		fcomm.close();
		remove(sfcommname);
	}	// destructor


	void reopenComment(ProcessorClass::dictListStruct *dictFile); // close fcomm; re-opens as an ios:in file; sets theTail to appropriate file type
	commloc putComment(char *sphrase); // stores comment; returns pointer
	void getComment(commloc loc, char *sphrase);   // retrieves comment, puts in sphrase
	void putTail(char *sphrase);  // stores tail information
	bool getTail(char *s);  // retrieves tail information

private:
	fstream fcomm;
	char 		sfcommname[14];
	int 		curLen; 
	commloc curLoc;		// tailLen and tailLoc for current file; used for output

}; // CommentFileClass						

//___________________________________________________________________________________

class ReadFilesClass		// Input routines
{
public:
	instring sPhrase;  		// phrase found in input line
	instring sCode;				// code found inside [] in input line
	instring sComment;		// comment found after ; in input line
	commloc locComment;		// location of comment in comment file					
	bool fPhraseOK;				// any problems in the initial parse?
	bool isSynRoot;                         // storing a synset root, so treat it as a literal. See Phrases.store()
		
	void readActors(void);
	void readAgents(char *sfilename);
	void readVerbs(char *sfilename);
    void doOptionCommand(char *sCmd);    // processes on .options command. 
	void readOptions(char *sfilename);
	void readIssues(char *sfilename);
	void readFreqs(char *sfilename);

	int getISSUE(void); // process the ISSUES tag: public since it makes a friend reference to Coder
	int getFREQ(void);	// process the FREQ tag; returns current value of kategory
	void getSET(void);			// process the SET command 

//	void set(char *sphrase, char *scode, char *scomment, commloc loc);   // *** debugging: sets the local variables
//	void testOptions(void); // *** debugging : tests assorted .options functions
//	void test(void); // ### debugging function 

	friend class ProcessorClass;
	friend class CommentFileClass;			
	friend class MergeFilesClass;			
	friend class ModifyClass;			
	friend class CoderClass;
		
	ReadFilesClass()
	// constructor
	{ 
		iIDInfoLoc =-1; // if this remains < 0, there was no FORWARD: specification
		iIDInfoLen = 0;
		iStoryLoc  = 0;
		iStoryLen  = 0;
		iSentLoc   = 0;
		iSentLen	= 0;		
		fhasDoc 	= false;
   		
		fOutputID       = false;
		fOutputStory    = false;
		fOutputSeq      = false;
		fOutputDoc      = false;
		fOutputText     = false;
		fOutputPats     = false;
		fOutputShift    = false;
		fOutputLabels	= true;
		fOutputMatch	= false;
		fOutputActRoots	= true;  // note that these two variable names are *way* too similar, if logical
		fOutputAgtRoots = true;
		fOutputParent   = true;
        fOutputLocs     = false;
		
		fisNumeric = false;
		fcodeOptional = false;
		fdoingFreq	= false;
		
        isSynRoot = false;

	} // constructor
	
private:
	ifstream fin;			// input file
	instring sLine;			// buffer for input line
	filestring curFilename; // name of file currently being read

	// record ID and sequence info
	int iIDInfoLoc;
	int iIDInfoLen;		
	int iStoryLoc;
	int iStoryLen;
	int iSentLoc;
	int iSentLen;
	bool fhasDoc;

	// output options
	bool fOutputID;
	bool fOutputStory;
	bool fOutputSeq;
	bool fOutputDoc;
	bool fOutputText;
	bool fOutputPats;			// output matched patterns
	bool fOutputShift;			// output time shifting information
	bool fOutputLabels;			// output event code labels in CoderClass:: getEventString
	bool fOutputMatch;			// output matched text to event records in CoderClass:: getEventString
	bool fOutputActRoots;		// output root phrase for source and target actors to event records in CoderClass:: getEventString
	bool fOutputAgtRoots;		// output root phrase for source and target agents to event records in CoderClass:: getEventString
	bool fOutputParent;   		// for fOutputActRoots, output the parent in an actor synonym set rather than the actual phrases 
	bool fOutputLocs;   		// output lexArray locations for the source, target, verb 

	void doPhraseError(const char *sreason);  // announces error; decides whether to quit;
	void doDupError(const char *sreason); // duplicate error with line information decides whether to quit; 

	bool fisNumeric;  	// processing a numeric issue
	bool fcodeOptional;  // parseline allows no code
	bool fdoingFreq;		// processing frequency file

	void getsLine(void);  		// safe version of getline: throws LINE_LONG exception if line is too long.
	void doLINE_LONG(void); 	// handles LINE_LONG exception
	void openFile(char *sfilename);   // opens "sfilename" for reading
	bool readLine(void); 		// reads a phrase; returns false when internal EOF is hit
	void storeTail(void); 		// stores the tail of dictionary file; closes fin

	void insertStr(char *s1, char *s2); // used to filter input phrase  
	void parseLine (void); 			// Generic phrase parsing procedure that handles all phrase [code] strings
	void addBlanks (char *pst, char c, int &len); // adds blanks around c; increments len; does nothing if len>=MAX_LENGTH

	void optionsError (const char *s);
	void trimCommand (char *s); // trim command from s and  deletes leading blanks
	void getField (char *s, const char *substr, char *sout); // Gets the ="..." in s following substr, assigns blank-trimmed value to sout. 
	void getInteger (char *s, const char *substr, int &iout);	// Gets the first number in s following substr, assigns value to iout, or defaults to zero. 
    void resetInteger (char *s, const char *substr, int &iout); // getInteger except does not default to zero
	bool getBoolean (void); // looks for a T/F after an =
	int  get3Way(void);     // looks for a OFF/T/F after an =; 
	void getCOMPLEX(void); 	// process the COMPLEX command 
	void getDEFAULT(void);    // process the DEFAULT command 
	void getFORWARD(void); 	// process the FORWARD command 
	void getOUTPUT(void);		// process the OUTPUT command 
	void getCOMMA(void); 		// process the COMMA command  #### <09.01.12> current not implemented
	void getLABEL(void); 		// process the LABEL command 
	void getNONEVENTS(void); // process the NONEVENTS command 

	void readTime(void);
	void readAttrib(void);
	void readWord(const char *sendtag, wordtype wtype);
    void readSynsets(void);

}; // ReadFilesClass

//___________________________________________________________________________________

class WriteFilesClass		// output routines
{
public:
	void writeActors(ProcessorClass::dictListStruct *dictFile);
	void writeVerbs(ProcessorClass::dictListStruct *dictFile);

	friend class ProcessorClass;
		
private:
	ofstream fout;			// output file

	void openFile(char *sfilename);   // opens "sfilename" for writing
	void makeLine(char *s, char *scode, patStruct *pPat);	// adds scode and comments from *pPat pattern to s[]
	void writeActorUsage(bool doActors); // writes the actors or agents with usage statistics to fileName.use
	void writeVerbUsage(void); // writes the verbs dictionaries with usage statistics to fileName.use
			
}; // WriteFilesClass

//___________________________________________________________________________________

class ParserClass		// Parsing routines
{
public:		
	int iparseError;
	
	char filtext[MAX_SENTENCE]; 	// filtered text

	void doParsing(void); 				// calls the various parsing routines in the correct order
	void showParsing(void);				// displays sentence with syntactic markup
	void htmlParsing(void);				// writes parse information in html format
	void writeParsing(const char *sinfo);   // writes to Processor.fprob file
	void showTags(void);          // display tag lists
	void addLitr (toktype ilit, toktype icomp); 	// add the literal index ilit and comparision result icomp to litArray
	int  hasLitr (int index, toktype litr); // checks for literal in the list for syntArray[index]; returns status
	bool hasHeadTag(int loc, toktype tag); // returns true if snytArray[loc].pheadtag contains tag
	bool hasHeadTag(int loc, toktype tag, toktype &index); // returns true if snytArray[loc].pheadtag contains indexed tag of type "tag:index"
	bool checkHeadTag(int loc, toktype tag, toktype index); // returns true if snytArray[loc].pheadtag contains tag of type tag and value of index.
	bool hasTailTag(int loc, toktype tag);	// returns true if snytArray[loc].ptailtag contains tag; sets value of index
	bool hasTailTag(int loc, toktype tag, toktype index); // returns true if snytArray[loc].pheadtag contains indexed tag of type "tag:index"
	toktype getTailTag(int loc, toktype tag); // returns index if snytArray[loc].ptailtag contains tag of type tag and returns value of index or zero if tag not found
	bool checkTailTag(int loc, toktype tag); // confirms that a corresponding tail exists for an indexed head tag at loc

	void testCplx(void); // *** debugging : tests to check/boolCplx functions
//	void showLitr(int idx); // *** debugging : shows the contents of literaList list starting at idx
//	int getLitr(void); // *** debugging : returns current value of iLitr
//	void initLitr(void); // *** debugging : sets iLitr = 0

	friend class CoderClass;
	friend class ReadFilesClass;
//	friend int main(void); // *** debugging
	friend void ReadFilesClass:: getSET(void);
	friend void ProcessorClass:: showMemory(void);
	friend void ProcessorClass:: readRecord(bool forward);
		
	ParserClass()
	// constructor
	{ 
		iTag = 0;
		
	// constants for setting lex flags
		setMidCap  = 0x0001; // capitalized in mid-sentence
		setInitCap = 0x0002; // capitalized, beginning of sentence
		setNumber  = 0x0004; // number
		setComma	 = 0x0008; // comma
		setReplace = 0x0010; // substituted text (XML)
		
	// constants for setting synt flags
		setCompd  = 0x0001; // word is part of a compound
		setRefer  = 0x8000; // signal index of a pronoun reference
		maskRefer = 0x7FFF; // get index of a pronoun reference

	// text for tags
//	 	assert(MAX_TAG_TYPES > Halt);  // make sure we've got room here
 		strcpy(tagText[Null],"null");		tagTab[Null]  = 2;
		strcpy(tagText[Actor],"actor"); tagTab[Actor] = 1;
		strcpy(tagText[Agent],"agent"); tagTab[Agent] = 1;
		strcpy(tagText[Verb],"verb");   tagTab[Verb]  = 2;
		strcpy(tagText[Time],"time"); 	tagTab[Time] = 2;
		strcpy(tagText[Attrib],"attrib"); tagTab[Attrib] = 0;
		strcpy(tagText[Determ],"determ"); tagTab[Determ] = 0;
		strcpy(tagText[Noun],"noun");		tagTab[Noun]  = 2;
		strcpy(tagText[Pronoun],"prnoun");tagTab[Pronoun] = 0;
		strcpy(tagText[Conj],"conj");		tagTab[Conj]  = 2;
		strcpy(tagText[Prep],"prep");		tagTab[Prep]  = 2;
		strcpy(tagText[Plural],"plural");tagTab[Plural] = 0;
		strcpy(tagText[Adjctv],"adjctv");tagTab[Adjctv] = 0;
		strcpy(tagText[Auxil],"auxil"); tagTab[Auxil] = 1;
		strcpy(tagText[Byword],"by_");	tagTab[Byword] = 3;
		strcpy(tagText[Comma],"comma");	tagTab[Comma] = 1;
		strcpy(tagText[Number],"number");tagTab[Number] = 0;
		strcpy(tagText[Issue],"issue");	tagTab[Issue] = 1;
		strcpy(tagText[Synym],"synym");	tagTab[Synym] = 1;
		strcpy(tagText[Clause],"clause");tagTab[Clause] = 0;
		strcpy(tagText[Compound],"compnd");tagTab[Compound] = 0;
		strcpy(tagText[Reference],"refrnc");tagTab[Reference] = 0;
		strcpy(tagText[Subord],"subord");	tagTab[Subord] = 0;
		strcpy(tagText[Replace],"replac");tagTab[Replace] = 0;
		strcpy(tagText[NullTag],"nultag");tagTab[NullTag] = 0;
		strcpy(tagText[Halt],"-halt-");		tagTab[Halt] = 0;// (should never see this...)

 		strcpy(tagColor[Null],"GRAY");		tagTab[Null]  = 2;
		strcpy(tagColor[Actor],"RED"); tagTab[Actor] = 1;
		strcpy(tagColor[Agent],"BLACK"); tagTab[Agent] = 1;
		strcpy(tagColor[Verb],"BLUE");   tagTab[Verb]  = 2;
		strcpy(tagColor[Time],"BLACK"); 	tagTab[Time] = 2;
		strcpy(tagColor[Attrib],"BLACK"); tagTab[Attrib] = 0;
		strcpy(tagColor[Determ],"MAROON"); tagTab[Determ] = 0;
		strcpy(tagColor[Noun],"FUCHSIA");	// aka MAGENTA	tagTab[Noun]  = 2;
		strcpy(tagColor[Pronoun],"LIME");tagTab[Pronoun] = 0;
		strcpy(tagColor[Conj],"GREEN");		tagTab[Conj]  = 2;
		strcpy(tagColor[Prep],"BLACK");		tagTab[Prep]  = 2;
		strcpy(tagColor[Plural],"BLACK");tagTab[Plural] = 0;
		strcpy(tagColor[Adjctv],"BLACK");tagTab[Adjctv] = 0;
		strcpy(tagColor[Auxil],"BLACK"); tagTab[Auxil] = 1;
		strcpy(tagColor[Byword],"BLACK");	tagTab[Byword] = 3;
		strcpy(tagColor[Comma],"BLACK");	tagTab[Comma] = 1;
		strcpy(tagColor[Number],"BLACK");tagTab[Number] = 0;
		strcpy(tagColor[Issue],"BLACK");	tagTab[Issue] = 1;
		strcpy(tagColor[Synym],"BLACK");	tagTab[Synym] = 1;
		strcpy(tagColor[Clause],"BLACK");tagTab[Clause] = 0;
		strcpy(tagColor[Compound],"BLACK");tagTab[Compound] = 0;
		strcpy(tagColor[Reference],"BLACK");tagTab[Reference] = 0;
		strcpy(tagColor[Subord],"BLACK");	tagTab[Subord] = 0;
		strcpy(tagColor[Replace],"BLACK");tagTab[Replace] = 0;
		strcpy(tagColor[NullTag],"BLACK");tagTab[NullTag] = 0;
		strcpy(tagColor[Halt],"BLACK");		tagTab[Halt] = 0;// (should never see this...)

// set subordinate limits to KEDS levels
		minSubord	=  2;
		maxSubord	= 10;
		minBegSubord =  MAX_LEX;  // effectively deactivates this check
		maxBegSubord =  0;
		minEndSubord =  2;
		maxEndSubord = 10;
		
		minLex = MIN_LEX - 1;
		
// variables for forwarding
		fuseForward = false;	//	forwarding has been activated
		fcanForward = false;	//	forwarding can be used on this record	

// variables for complexity evaluation
		fCheckCplx = false;		// check complexity conditions?
		fCplxStrg  = false; 	// write complexity string?
		fIsComplex = false;		// are complexity conditions met?
		cplxLimit.nVerb 	= 0;		// set default limits
		cplxLimit.nActor	= 0;
		cplxLimit.nConj		= 0;
		cplxLimit.nPronoun 	= 0;
		cplxLimit.nLateVerb = 0;
		cplxLimit.nNonRes 	= 0;
		cplxLimit.fPreAct 	= false;
		cplxLimit.fPostAct 	= false;
		cplxLimit.fNoVerb 	= false;
		cplxLimit.fNoSrc 	= false;
		cplxLimit.fNoTar 	= false;
		cplxLimit.fNoEvt 	= false;

		fHasDiscard = false;	// is there a primary discard code?
		fRecCAPS = true;      // all caps record?

// this is a placeholder for the SOURCE/TARGET default codes
		syntArray[MAX_SYNT-1].index 	= MAX_SYNT-1;		// index of array
		syntArray[MAX_SYNT-1].ilit 		= 0;		// start of literaList list
		syntArray[MAX_SYNT-1].iroot 	= 0;		// index of rootArray entry: this is not used; actor.rootidx is set to rootDefActor 
		syntArray[MAX_SYNT-1].wtype 	= Null;     // word type
		syntArray[MAX_SYNT-1].pheadtag 	= 0;		// tag list prior to word
		syntArray[MAX_SYNT-1].ptailtag 	= 0;		// tag list following word
		syntArray[MAX_SYNT-1].flags 	= 0;		// indicator flags

	} // constructor
	
private:
	struct lexStruct {
		charptr ptext;		// start text in senfilt
		charptr pend;		// end of match, if any  (### no longer used?)
		int 	ilit;		// index to start of literal list in literaList; -1 if none
		toktype flags;		// indicator flags
	} lexArray[MAX_LEX];
	int iLex;						// final active cell in lexArray
	
	struct syntStruct {
		toktype index;		// array index (used to convert pointers)
		int 		ilit;			// index to start of literal list in literaList; -1 if none
		rootindex iroot;	// index of rootArray entry
		wordtype wtype;		// word type
		toktype pheadtag;	// tag list prior to word
		toktype ptailtag;	// tag list following word
		toktype flags;		// indicator flags
	} syntArray[MAX_SYNT];
	int iSynt;					// final active cell in syntArray (iLex + 1)
	
	toktype tagArray[MAX_TAGS];			// storage for head and tail tags
	char tagText[MAX_TAG_TYPES][9]; // labels for the showParse tags
	char tagColor[MAX_TAG_TYPES][8]; // colors for the showParse tags
	int tagTab[MAX_TAG_TYPES];			// number of blanks to use to fill out tags
	int iTag;    // this has value of the *next* tag to be added
	
	int tempArray[MAX_TEMP];  // temporary storage/stack
	int iTemp; 

	int matchArray[MAX_TEMP];  // storage for the longest root matched
	int iMatch; 

	toktype literaList[MAX_LITRLIST];  // lists of matched literals
	int iLitr; 

 	bool fRecCAPS; // inidividual record is all caps?
 
 	// constants for setting lex flags : see constructor for use
	toktype setMidCap, setInitCap, setNumber, setComma;
	toktype setReplace;
	bool fhasClauses;		// are there multiple clauses? 

	// constants for setting synt flags : see constructor for use
	toktype setCompd;
	toktype setRefer;
	toktype maskRefer;

	int indexSource, indexTarget, indexCompound;
	
	int minSubord, maxSubord;   // min and max size for a subordinate (comma-delimited) clause
	int minBegSubord, maxBegSubord;   // min and max size for an initial subordinate clause
	int minEndSubord, maxEndSubord;   // min and max size for a terminal subordinate clause
	
// variables for forwarding
	bool fuseForward;	// ### should this be in TABARIFlags??
	bool fcanForward;
	int  idxForwActor;  	// index of forwarded actor
	int  idxForwCompd;		// index of forwarded compound
	int  lastSynt;				// iSynt from previous record

// variables for complexity conditions
	struct cplxStruct {
		int nVerb, nActor, nConj, nPronoun, nLateVerb, nNonRes;
		bool fPreAct, fPostAct, fNoVerb, fNoSrc, fNoTar, fNoEvt;
	} cplxLimit, cplxValue;

	bool fCheckCplx, fCplxStrg;  // check complexity conditions; write complexity string
	instring sComplex;
	bool fIsComplex;		// are complexity conditions met
	bool fHasDiscard;		// is there a primary discard code?

	int minLex;			// minimum size for valid sentence

	void parseError(const char *s, int ierror);	// set fhasError, get out noisily but without crashing
	void getParseError(char *s);				// set text of problems string based on iparseError
	bool checkDisplay(bool Initialize); // scroll parse display

	void addSegHeadTag(int loc, toktype tag, toktype index);	// adds indexed tag to snytArray[loc].pheadtag for segments
	void addHeadTag(int loc, toktype tag);	// adds tag to snytArray[loc].pheadtag
	void addHeadTag(int loc, toktype tag, toktype &index); // adds next indexed tag to snytArray[loc].pheadtag; returns value of index
	void addSegTailTag(int loc, toktype tag, toktype index);	// adds indexed tag to snytArray[loc].ptailtag for segments
	void addTailTag(int loc, toktype tag);	// adds tag to snytArray[loc].ptailtag
	void addTailTag(int loc, toktype tag, toktype index); // adds next indexed tag to snytArray[loc].ptailtag; sets value of index
	void changeHeadTag(int loc, toktype oldtag, toktype newtag); // changes oldtag to newtag
	void changeTailTag(int loc, toktype oldtag, toktype newtag); // changes oldtag to newtag
	toktype getReference(int loc); // gets a pronoun reference from the headtag of syntArray[loc]  
	bool skipSegment(syntStruct * &psyn, wordtype wtype); // 	skip across a segment (i.e. clause, compound or subord)
	bool skipSegment(int &index, wordtype wtype);	// 	this variant works directly with the index rather than a pointer.

	void filterText(void); 							// transfers sentext to filtext
	bool matchLex(int &nmatch, tokptr phrase, wordtype wtype, int ibegin); // attempts to match phrase beginning at lexArray[ibegin]

	void makeLex(void);					// convert senfilt to lexArray
	void makeSynt(void);				// fill in syntArray from lexArray; assign roots
	void setForward(void);			// check if forwarding can be done; transfer first actor and compound from
	void makeFBISCompound(void); //	in FBIS_Mode, converts <Actor><Comma><Actor> to <Actor><Conj><Actor>
	void markSubordinate(void); //	find the subordinate clause boundaries
	void nullSubordinate(void); // 	null-out everything within the subordinate clause boundaries. 
	void markNouns(void);				// change any capitalized verbs and verbs preceded by determiners to nouns
    void convertAgents(void);        // change any agents that are not associated with an actor to actors; used only if fConvAgents == TRUE
	void markCompound(void); 		// Checks for a syntactic compound actor.
	void dereferencePronouns(void); //  Dereference pronouns.  The following rules are used:
	void markClauses(void); 		//	find the compound clause boundaries.
	
	void checkCplxValue (int check, int value, const char *s); // checks whether value >= check
	void checkCplxBool (bool check, bool value, const char * s); // checks whether value and check are true
	int countType (wordtype thetype); // returns the count of words of type wtype in syntArray 
	int countTags (wordtype thetype); // returns the count of head tags of type wtype in syntArray, skipping over 
	void evaluateComplexity(void); 		// evaluate and check cplxValue indicators for the complexity detector 
	void checkDiscards(void); // check for discard codes 

}; // ParserClass

//___________________________________________________________________________________

class CoderClass		// Coding routines
{
public:		
	bool fhasProblem;		// no events coded due to discard, complex, etc.
	instring sProblem;	// description of reason no events coded
	int  nEvents;				// number of events found - 1  (i.e. 0 => 1 event found)

	freqStruct  freqHead; 			// start of freq lists
	int iFreq;					// freq categories + 1

	void codeEvents(void); // driver routine
	char * getEventString(char *s, int ievt); // converts eventStrg[ievent] to a formatted output string in s
	char * getPatterns(char *s, int ievt); // puts pattern strings into s
	void setcodeMode(char *s);  // sets the coding mode	

	friend void ReadFilesClass:: readOptions(char *sfilename);
	friend void ReadFilesClass:: getDEFAULT(void);
	friend int  ReadFilesClass:: getISSUE(void);
	friend void ProcessorClass:: showMemory(void);
	friend void ProcessorClass:: showEvents(void);
	friend void ProcessorClass:: writeEvents(void);
	friend void ProcessorClass:: writeProblem(void);
	
	CoderClass()
	// constructor
	{ 
		// flags set by VALID option
		codeMode = 0;					// default is code-by-clause
	
		issueHead.kategory = -1;
		issueHead.phrase	 = NULL;
		issueHead.icode		 = 0;
		issueHead.doAll		 = false;
		issueHead.doNumber = false;
		issueHead.found		 = false;
		issueHead.pnext		 = NULL;
		issueHead.plist		 = NULL;

		freqHead.kategory = -1;
		freqHead.phrase	  = NULL;
		freqHead.abbrev	  = NULL;
		freqHead.pnext		= NULL;
		iFreq = 0;

		fhasDefltSrc = false;	// 	default source code is available
		fhasDefltTar = false;	//  same for target
		fDefPRIOR	 = false;   //  PRIOR option
		fDefAFTER	 = false;   //  AFTER option
		idxcodeDefSrc = 0;		//  indices for default codes
		idxcodeDefTar = 0;
        
        sTimeShift[0] = '\0';			// time-shift marker
        sShiftDate[0] = '\0';			// shifted date

	}

private:
	struct actorStruct {
		toktype actor;	// index of actor code
		toktype agent;	// index of agent code
		toktype index;  // index of start of complex code; zero if none
		rootindex rootidx; // index of actor root
		rootindex agtidx; // index of agent root
		int     where;  // where the actor was found
	};

	struct eventStruct {
		toktype source;			// codeArray index of source
		toktype srcagt;			// same for source agent
		toktype srcidx;			// codeArray index if source is complex
		toktype srcloc;			// lexArray index for start of source 
		toktype target;			// target
		toktype taragt;			// target agent
		toktype taridx;			// codeArray index if target is complex
		toktype tarloc;			// lexArray index for start of target
		toktype attrib;			// attribution
		toktype attidx;			// codeArray index if attribution is complex
		toktype event;			// codeArray index of event
		toktype evtloc;			// lexArray index for start of verb
		patStruct* ppat;            // matching pattern
		rootindex srcroot;      // root index for matched source
		rootindex tarroot;      // root index for matched target
		rootindex sagtroot;     // root index for matched source agent
		rootindex tagtroot;     // root index for matched target agent
		rootindex verbroot;     // root index for matched verb; 0 signals a non-event
		tokindex imatch;           // TokBuf index of lexArray indices for matched phrase
		bool 		fisevt;			// valid event?
		bool 		fissub;			// subordinant?
		bool 		fisdom;			// dominant?
		int 		nextEvt;		// eventArray index used to keep final strings in a plausible order
	} eventArray[MAX_EVENTS];

	struct evtStringsStruct {
		toktype sourceCode;		// codeArray index for source
		toktype srcagtCode;		// same for source agent
		toktype targetCode;		// target
		toktype taragtCode;		// target agent
		toktype attribCode;		// codeArray index for attribution
		toktype eventCode;		// event code
		tokindex imatch;			// TokBuf index of lexArray indices for matched phrase
		rootindex srcroot;    // root index for matched source
		rootindex tarroot;    // root index for matched target
		rootindex sagtroot;   // root index for matched source agent
		rootindex tagtroot;   // root index for matched target agent
		rootindex verbroot;   // root index for matched verb
		patStruct* ppat;		  // matching pattern
		toktype srcloc;			// lexArray index for start of source 
		toktype tarloc;			// lexArray index for start of target
		toktype evtloc;			// lexArray index for start of verb
		bool fisEvent;
	} eventStrgs[MAX_EVENT_STRINGS];
	

	int tempArray[MAX_TEMP];  // temporary storage/stack
	int iTemp;
	tokindex itempToken;			// index for matching index storage 
	int	lenalt;               // length of text matched in alternatives
	
	// checkPattern globals
	bool fskipMode;	// are skips allowed in text? (otherwise match required on next item)
	bool fullMode;	// is full match required on current target? (otherwise partial match allowed)
	bool isHigh;		// true if doing upper check
	tokptr ptPat;		// pointer to current pattern location
	tokptr ptThisPat;	// pointer to pattern being evaluated
	int locSynt;		// current text location

	// variables for saving state during evaluation of alternative sets
	bool  fcond;  		// doing alternative/conditional
	bool  condfskip;
	int   condloc;
	int   condtemp;
	int   condlen;

	actorStruct theSource;
	actorStruct theTarget;

	issueHeadStruct  issueHead; // start of issue lists
	instring sIssueCodes;				// issue code string

	instring sMatch;		// string that was matched
	int istartClause,iendClause;  // start and end of clause containing verb
	int indexSource;		// indices for actors located in patterns
	int indexTarget;
	int indexAttrib;
	int indexCompound;
	bool fRegularSource;// source was set using regular match, rather than through a pattern
	bool fRegularTarget;// same for target
	bool fisPassive;   // passive voice
	bool fPause;
	bool fhasDiscard, fhasComplex;
	int	 codeMode;					// clause, sentence or all coding mode
	
	bool 	fhasDefltSrc;	// 	default source code is available
	bool 	fhasDefltTar;	//  same for target
	bool	fDefPRIOR;   //  PRIOR option
	bool 	fDefAFTER;   //  AFTER option
	toktype idxcodeDefSrc;		//  indices for default codes
	toktype idxcodeDefTar;

    char sTimeShift[16];	// record is time-shifted
	char sShiftDate[MAX_DATE_LEN];	// shifted record date

	bool actorEqual(actorStruct a, actorStruct  b);
	bool actorEmpty (actorStruct a);
	void zeroActor (actorStruct &a);// set both elements of a to zero
	void setActor (actorStruct &a, actorStruct  b);// set a = b
	void coderError(const char *s, int ierror);
	bool setClauseBounds (void); 	// set the conjunction boundaries istartClause and iendClause around TLoc
	void getActorStructCodes(actorStruct &actFound, int loc); // attach codes to actFound based on the root at loc
	void findActor (int loc, int &locFound, actorStruct &actFound); // Find the beginning of next actor in syntArray with index >= loc; 
	void findAgent (int loc, actorStruct &actFound); // Find the beginning of an actor immediately adjacent to loc, checking after first, then prior 
	void backActor (int loc, int &locFound, actorStruct &actFound); // Find the beginning of next actor in syntArray with index <= loc; 
	int	 getActorHead(int loc, bool &fmatch);   // moves the entire actor root into temp; returns location of root head;
	int  getActorTail(int loc);                 // Returns location of actor root tail;
	int  getCompound(int loc, bool &fmatch); 	// moves the entire compound phrase into temp; returns location of phrase head;
	int  getCompound(int loc); 					// returns location of phrase head only
	void saveState(void);		// save prior to evaluating synset or conditional
	void restoreState(void);		// restore after fail in evaluating synset or conditional
	void initCondit(void); 		// save conditional state	
	void condCheck(int &state);	// see if we have alternatives in conditional if current option failed
    int  compSynset(void);      // Compares the synset patterns at Rootbuf[*ptPat] with syntArray location locSynt;
	int  checksubPattern(patStruct *pPat); // check for a subpattern in synset
	int  compTarget(void);		// Compares current pattern target *ptPat with syntArray location locSynt;
	int  continueCheck(void); 	// move in text without changing target and mode
	int  nextCheck(void);				// increment text and pattern: change target and mode
	int  stateCheck(int result); // implements response to result based on mode, then returns state of pattern check 
	bool checkPattern(patStruct *pPat, int loc, wordtype wtype); //	Core routine for checking verb phrases
	void setDefaultSource(void); 		// sets the source to the default
	void setDefaultTarget(void); 		// same for target
	void getRegularSource(int index); 	// Regular source finder; used if the source is not found by a pattern
	void getRegularTarget (int index);  // same for targets
	void checkTime(void); 					// Check for time-shift phrases.  
	bool shiftDate (patStruct * pPat);	// Shift date based on pPat
	bool checkPassive(int index); 	// check for passive voice using the following rules:
	void doVerb (int index); 				// Process verb phrase at index
	void checkVerbs(void); 					// Main verb processing loop 
	void codeIssues (void); 				// get any issues codes
	void makeIssueString (void); 		// assemble to issues codes string
	void zeroIssues (void); 				// zero-out the issues lists
	void codeFreqs (void); 					// get freq codes
	void checkNonEvents(void); // Create a record consisting of the first two distinct actors and a non-event code
	tokindex storeMatch(void); 			// store contents of tempItem
	void copyEvent (int ka, int kb);// set eventArray[ka] = eventArray[kb]
	void putEvent (patStruct * pPat, int index, actorStruct actSource, actorStruct actTarget ); // Load an event record into eventArray[nEvents] based on pPat, syntArray[index], source and target actors 
	void recoverCodes(void); 				// recovers initial codes in the case of eventArray overflow
	bool checkEventDups(int ievt);	// Check for duplicates
	bool checkSameActors(int ievt);	// Check if source and target codes are identical.
	void expandCompoundCodes(void);	// Expand the coded compounds using the compound lists in CodeStore
  	void addCompositeCode(char *s, toktype itokactor, toktype itokagent); // add the combined actor and agent codes to end of s 
	void makeEvent (patStruct * pPat, int index); // Create an event record from pPat, syntArray[index]TLoc; finds the agent 
	void evaluateEvents(void); 			// Evaluates the subordinant and dominant events, then compacts eventArray
	void makeEventStrings (void);		//	Constructs the event record strings using eventArray
	char * getNonEventString(char *s);	// generates the formatted non-event string based on eventStrg[0]
	void setProblemString(char *s); // sets the problem description
	void setProblemString(const char *s); // sets the problem description

	actorStruct theAttrib;  // attribution actor
	bool checkAttribCode (patStruct * pPat);	// Check for complexity, discard and null codes in attribution patterns.  
	void getAttribActor (int loc); // Find the attribution actor:
	void doAttrib(void); 					  // Check for attribution  


}; // CoderClass


//___________________________________________________________________________________

class ModifyClass		// Dictionary modification routines
{
public:		
	void doModify(void); // driver for dictionary modification
		
private:
	char 	start[2]; 		// holds the starting characters
	int 	iSelect;			// item selected in menu	
	int 	minItem, maxItem;  // minimum and maximum 
	toktype irootSelect[WINDOW_HEIGHT];  // rootArray indices by line; this is actually a little bigger than it needs to be
	patStruct* ppatSelect[WINDOW_HEIGHT];  // rootArray indices by line; this is actually a little bigger than it needs to be
	RootsClass::rootStruct theRoot;				// source of patterns

	bool enterCode(bool fnew);	// move code string into ReadFiles.sCode, adding [] if needed

	char getResponse(void); //	gets a character response from a menu
	int modifyMenu(void); // Menu for the primary modify selection
	int startDisplayMenu(bool deletePrompt); // Initial menu for the root modify display
	int pauseDisplayMenu(wordtype wty ); // Intermediate menu in the display
	int changeMenu(void);	// Menu for code/delete option
	int changeRootMenu(void); // Menu for code/delete option
	int changeVerbMenu(void); // Menu for pattern/code/delete option
	int pausePatternMenu(bool fend);// 	Menu for pattern modification

	void displayRoots(wordtype wty);	// writes part of the actors or verbs dictionary to the screen beginning at c1c2
	void displayPatterns(int iroot); // display the patterns for rootArray[iRoot]

	void setupChange(void);	     // set up a modify screen
	bool checkAgentCode(void);   // loop until we get a valid agent code or null entry
	void addRoot(wordtype wty);  // adds a new actor, verb or pattern
	void addPattern(int iroot);  // adds a new pattern
	void deleteRoot(int iroot);  // delete noun or adjective
	void changeActor(int iroot); // change code or delete actor
	void changeVerb(int iroot);  // display patterns, change code or delete verb
	void changePattern(patStruct *ppat); // change code or delete patterns,
	void deletePattern(patStruct *ppat); // delete pattern from list

}; // ModifyClass


