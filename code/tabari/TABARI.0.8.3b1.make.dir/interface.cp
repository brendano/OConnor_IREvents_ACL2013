// 																		TABARI Project
//____________________________________________________________________________________
// 																		interface.cp

// 	This file contains an assortment of general-purpose utilities that involve the
// 	screen interface.  This contains all of the screen input except for the functions 
//  found in modify.cp.  Unlike the GUI	interface of KEDS, TABARI  uses a very simple 
//  keyboard-driven "dumb terminal" interface via the ncurses package. The program is 
//  designed to work on a screen that is at least 48 lines high by 128 characters wide.
//
//	Screen output is  generally are isolated in a limited number of functions in 
//	the various classes -- these usually have a "show" prefix and only use   
//	addstr() for output.
//
//  Note that most of the *file* I/O is in the class ProcessorClass.
//
//	Operating System Options
//	As of Version 0.8, the only option available is that formerly known as NCURSES;  
//  the code for the older LINUX and WINDOWS has been eliminated, as has the NCURSES 
//  macro. Mac and Linux versions are now identical, and no one has ever shown any 
//  interest in keeping the Windows version up to date.
//
//	Input is isolated in the functions getUpper, ReadLine and GetInput: these are
//	configured so that the program responds immediately to keystrokes in menus
//	("noncanonical", in Unix terminology) but waits for a return (and recognizes
//	the backscape/delete key) when entering codes and phrases.  How this (and the
//	echoing of keystrokes) is handled depends on the operating system.
//
//	Output is done through a number of basic output macros and functions, plus
//  some interface-specific code for more elaborate structures such as menus
//  and list displays.
//
//__________________________________________________________________________________
//
//	Copyright (c) 2002-2012  Philip A. Schrodt.  All rights reserved.
//
// 	Redistribution and use in source and binary forms, with or without modification,
// 	are permitted under the terms of the GNU General Public License:
// 	http://www.opensource.org/licenses/gpl-license.html
//
//	Report bugs to: schrodt@psu.edu

//	The most recent version of this code is available from the KEDS Web site:
//		http://eventdata.psu.edu

//	For plausible indenting of this source code, set the tab size in your editor to "2"

//___________________________________________________________________________________
// 																	Headers

#include "TABARI.h"

//___________________________________________________________________________________
// 											Function prototypes not in Tabari.h

void InitTABARI(void);				// Do introduction
void initTerminal(void);			// Set Linux terminal parameters
void firstHaiku(void);				// selects and displays introductory haiku
void writeIntro(void);				// Writes the introductory screen

extern TabariFlagsClass 	TabariFlags;

//___________________________________________________________________________________
// 																																			


void initTerminal(void)
// initializes the ncurses environment
{
	initscr(); 
	cbreak(); 
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
} // initTerminal

char getUpper(void)
// returns uppercase keystroke without echo; converts \n to \r
{
char	c = toupper(getch());
	if ('\n' == c) c = '\r';  // convert Linux \n to a Mac \r
	return c;
}

void ClearScreen(void)
// clears screen but does not refresh
{ 
	erase();
	TabariFlags.cursc = 0;
	TabariFlags.cursr = 0;
 }
 
void InitTABARI(void)		
// Do introduction
{
	initTerminal();
	writeIntro();
}

void ResetCurs(void)
// refresh screen, then get cursr, cursc. Used following series of screen
// writes where final location of cursor is indeterminant
{
	refresh();
	getyx(stdscr,TabariFlags.cursr,TabariFlags.cursc);
}

void ReadLine(char *s)
// reads s from keyboard.
// ncurses: toggles to the canonical status, reads the string,
// then back to non-canonical
{ 
	nocbreak();
	echo();
	getstr(s);
	refresh(); // is this needed? ###
	getyx(stdscr,TabariFlags.cursr,TabariFlags.cursc);
	cbreak(); 
	noecho();
	} // ReadLine/ncurses

void GetInput(char *s, const char *sprompt)
// write sprompt on current line, then read s from keyboard 
// ncurses: toggles to the canonical status, reads the string,
// then back to non-canonical
{ 
	nocbreak();
	echo();
	addstr(sprompt);
	addstr(": ");
	refresh();
	getstr(s);
	getyx(stdscr,TabariFlags.cursr,TabariFlags.cursc);
	cbreak(); 
	noecho();
} //GetInput/ncurses

bool safeGetLine(char *s)
// gets a line of text from keyboard, checking that buffer isn't exceeded.
// Note:This function gets around an infinite loop that occurred when a user typed
//      a string that exceeded the length of instring s.  I would think that there
//      would be a straightforward way around this using standard stdio functions, 
//      but I couldn't figure it out, and it might be necessary because I'm using 
//      <return>-free keyboard input in the MetroWerks SIOUX environment.  If you 
//      know a more elegant solution, please implement it... [02.02.07]

{
char *ps = s;
int   ka = 0;

	while (ka < MAX_TAB_INPUT) {
		*ps = getch();
		addch(*ps);
		if (('\r' == *ps) || ('\n' == *ps)) { // handle Mac and Linux <returns>
			break;
		}
		++ka;
		++ps;
	} 
	if (ka >= MAX_TAB_INPUT) {
		WriteLine("Input line length exceeded; please hit <return> at the end of each line");
		s[MAX_TAB_INPUT-1] = '\0';
		return false;
	}
	*ps = '\0';
	return true;
} // safeGetLine

// NOTE: All of the Write*() functions require that TabariFlags.cursr is set
//       correctly; if mixing these with the WRITE*() macros, use ResetCurs() 
//       first      

void WriteNewLine(const char *s)
// write s to display on new line
{
	getyx(stdscr,TabariFlags.cursr,TabariFlags.cursc);
	move(++TabariFlags.cursr,0);
	addstr(s);
	move(++TabariFlags.cursr,0);
	refresh();
}

void WriteLine(char *s)
// write s to display
{
	addstr(s);
	move(++TabariFlags.cursr,0);
	refresh();
}

void WriteLine(const char *s)
// write s to display
{
	addstr(s);
	move(++TabariFlags.cursr,0);
	refresh();
}

void WriteLine(char *s1, char *s2)
// write s1 and s2 to display
{
	addstr(s1);
	addstr(s2);
	move(++TabariFlags.cursr,0);
	refresh();
}

void WriteLine(char *s1, const char *s2)
// write s1 and s2 to display
{
	addstr(s1);
	addstr(s2);
	move(++TabariFlags.cursr,0);
	refresh();
}

void WriteLine(const char *s1, char *s2)
// write s1 and s2 to display
{
	addstr(s1);
	addstr(s2);
	move(++TabariFlags.cursr,0);
	refresh();
}

void WriteAlert(const char *s1)	
// beep, then write s1  to display
{
	beep();
	getyx(stdscr,TabariFlags.cursr,TabariFlags.cursc);
	move(++TabariFlags.cursr,0);
	addstr(s1);
	move(++TabariFlags.cursr,0);	
	refresh();
}

void WriteAlert(const char *s1, char *s2)	
// beep, then write s1 and s2 to display
{
	beep();
	getyx(stdscr,TabariFlags.cursr,TabariFlags.cursc);
	move(++TabariFlags.cursr,0);
	addstr(s1);
	addstr(s2);
	move(++TabariFlags.cursr,0);	
	refresh();
}

void WriteLong(const char *s1, long i)
// write s1, integer and \n without refresh
{
char sout[16];
	addstr(s1);
	sprintf(sout,"%ld",i);
	addstr(sout);
	move(++TabariFlags.cursr,0);
}

void WriteFloat(const char *s1, float r)
// write s1, float and \n without refresh
{
char sout[16];
	addstr(s1);
	sprintf(sout,"%10.2f",r);
	addstr(sout);
	move(++TabariFlags.cursr,0);
}

bool GetAnswer(char *sprompt, char cyes, char cno)
// writes sprompt, reads a character, returns true if this is upper or lower cyes;
// response must be cyes or cno
{
char c;

	while (true) {
		beep();
		addstr(sprompt);
		addstr(" ->");
		refresh();
		c = toupper(getch());
		addch(c);
		refresh();
		if ((toupper(cno) == c) || (toupper(cyes) == c)) break;
		beep();
  	mvaddstr(++TabariFlags.cursr,0,"Please enter ");
  	addch(cyes);
  	addstr(" or ");
  	addch(cno);
  	move(--TabariFlags.cursr,0);	
  	clrtoeol();	
		--TabariFlags.cursr;
	}	
	WRITEEOL();
	if (c == toupper(cyes)) return true;
	else return false;
} // GetAnswer

bool GetAnswer(char *s1, char * s2)
// two strings, "Y" is the affirmative response; response must be 'N' or 'Y'
{
char c;

	while (true) {
		beep();
		addstr(s1);
		addstr(s2);
		addstr("? (Y/N) ->");
		refresh();
		c = toupper(getch());
		addch(c);
		refresh();
		if (('N' == c) || ('Y' == c)) break;
		beep();
  	mvaddstr(++TabariFlags.cursr,0,"Please enter Y or N");
  	move(--TabariFlags.cursr,0);	
  	clrtoeol();	
		--TabariFlags.cursr;
	}	
	WRITEEOL();
	if (c == 'Y') return true;
	else return false;
} // GetAnswer

void Pause(void)
// wait for char; exit if Q
{
	WriteNewLine("Pause: press any key to continue; Q to exit program without saving changes");
	char c = getUpper();
	if ('Q' == c) {
		addstr("Exiting...");
		refresh();
		exit(0);
	}
} // Pause


void showHaiku(const char * shaiku)
// Display a haiku string
{
const char * pst = shaiku;

	if (!*pst) return;
	addstr("\n--  ");
	while ((*pst) && ('/' != *pst)) addch(*pst++);
	if (!*pst) {
		addch('\n');
		showHaiku(shError99);
		return;
	}
	addstr("\n--   ");
	pst++;
	while ((*pst) && ('/' != *pst)) addch(*pst++);;
	if (!*pst) {
		addch('\n');
		showHaiku(shError99);
		return;
	}
	addstr("\n--    ");
	addstr(++pst);
	addch('\n');
	ResetCurs();
		 
} // showHaiku

void showMenu(const char *s)
// display menu prompt
{
		mvaddstr(++TabariFlags.cursr,0,s);
		refresh();
} // showMenu

void correctMenu(const char *s)
// prompts with correct response, returns to previous line
{
	beep();
	mvaddstr(++TabariFlags.cursr,0,s);
	move(--TabariFlags.cursr,0);
	clrtoeol();
	--TabariFlags.cursr;
} // correctMenu

void setupMenu(bool hasinit)
// move menu to a specific location on screen; call with false to set location
{
static int promptrow;

	if (hasinit) {
		move(promptrow+1,0);
		clrtoeol();
		TabariFlags.cursr = promptrow; // note that showMenu will increment this
//		mvaddstr(TabariFlags.cursr,0,">>>>>>>>>>>");
	}
	else {
		ResetCurs();
		promptrow = TabariFlags.cursr; 
		hline(0,WINDOW_WIDTH); // use default horz line char
//		mvaddstr(TabariFlags.cursr,0,"<<<<<<<<<<<<");
	}
} // setupMenu

int AutocodeSetup(void)	
// setup autocoding options
{
char c;
	ClearScreen();
	if (TabariFlags.fAutoCodemode > -1) return TabariFlags.fAutoCodemode; // level was set in command line
	while (true) {
		beep();
		addstr("-- Automatic coding mode --\n");
		addstr("\nFeedback level: N)one  D)ate  F)ull  H)elp  C)ancel ->");
		switch (getUpper()) {
			case '\r':;						// handles <enter>
			case 'N': return 0;
			case 'D': return 1;
			case 'F': return 2;
			case 'H': ShowAutoHelp();
			case 'X':;
			case 'Q':;
			case 'C': return 3;
		}
		if (c != 'H') {
			beep();
			addstr("\nPlease enter N, D, F, H or C\n");
		}
	}
// ### if a complex file is being written, confirm that user still wants it here	
} // autoSetup


bool PhraseError (char *stin, char *sreason)
// Announces error in .verbs and .actors input
// Called from: parsePhrase
// Returns false if quitting
{
int ka;
int startrow = TabariFlags.cursr;  // save where we've started

	WriteLong("Error in the input line ",TabariFlags.nlinesread);
	WriteLine(stin);
	WriteLine("Problem: ",sreason);
	ka = GetAnswer((char *)"Enter C to continue, Q to quit",'C','Q');
// ### need to work around this at some point...
	move(startrow,0);
	clrtobot(); // clear the message
	TabariFlags.cursr = startrow;
	return ka;	
} // phraseError

void ShowWarningError (const char *sreason, const char *sresponse, const char *shaiku)
// Announces an error, waits for <return>, then erases itself and returns.
// Entering q+<return> will exit the program
{
int startrow = TabariFlags.cursr; // save where we've started
  
	WriteAlert("Warning! Program is encountering a potential problem");
	WriteLine("Problem : ",(char *)sreason);
	WriteLine("Response: ",(char *)sresponse);
	showHaiku(shaiku); 
	WriteLine("Press <Return> to continue processing or Q to quit.");
	if ('Q' == toupper(getch())) exit(1);  // do-what-I-mean programming...
  else {
		move(startrow,0);
		clrtobot(); // clear the message
	}
} // ShowWarningError

void ShowWarningError (const char *sreason, char *s2, const char *sresponse, const char *shaiku)
// Version of function that concatenates two strings
// (this is typically used to add a phrase or file name to the message) 
// Entering q+<return> will exit the program
{
int startrow = TabariFlags.cursr; // save where we've started

	WriteAlert("Warning! Program is encountering a potential problem");
	WRITESTRING("Problem : ");
	WRITESTRING(sreason);
	WriteLine(s2);
	WriteLine("Response: ", (char *)sresponse);
	showHaiku(shaiku); 
	WriteLine("Press <Return> to continue processing or Q to quit.");
	if ('Q' == toupper(getch())) exit(1);  
  else {
		move(startrow,0);
		clrtobot(); // clear the message
	}
} // ShowWarningError

void ShowFatalError (const char *sreason, const char *shaiku)
// Announces an error, waits for <return>, then calls exit(1)
{
	WriteAlert("Fatal error encountered in program");
	WriteLine("Problem : ",(char *)sreason);
	showHaiku(shaiku); 
	WriteLine("Press <Return> to end processing.");
	getch();
	exit(1);		// ### we don't actually *use* the '1' at the moment...
} // ShowFatalError

void ShowFatalError (const char *sreason, char *sfile, const char *shaiku)
// Same as above but with two strings; usually the second is a file name
{
	WriteAlert("Fatal error encountered in program");
	WriteLine("Problem : ",(char *)sreason);
	WRITESTRING(sreason);  // ### <09.01.02> This probably is redundant; check on how it is used
	WriteLine(sfile);
	showHaiku(shaiku); 
	WriteLine("Press <Return> to end processing.");
	getch();
	exit(1);		// ### we don't actually *use* the '1' at the moment...
} // ShowFatalError

int RecordMenu(void)
// Menu for the primary read-parse-code loop
{
	while (true) {
		setupMenu(true);
		showMenu("Select: N)ext  P)arsing  M)odify  R)ecode  A)utocode  O)ther  Q)uit ->");
		switch (getUpper()) {
		case '\r':;						// handles <enter>
		case ' ' :;						// because my thumb rests on this key...
		case 'N': return 0;		// what the documentation actually says
		case 'P': return 1;
		case 'M': return 2;
		case 'R': return 3;
		case 'A': return 4;
		case '0': ;						// zero
		case 'O': return 5;
		case 'X': ;
		case 'Q': return 6;
		case '<': return 7;  // reverse
		case ',': return 7;
		}
		correctMenu("Please enter N, P, M, R, A, or Q");
	}	
} // RecordMenu

int OtherMenu(bool hasProb)
// Menu for the O)ther option in doProcess
{
	while (true) {
		if (hasProb) showMenu("Select: B)ackup  P)roblem  M)emory  U)sage  eX)it ->");
		else  showMenu("Select: B)ackup  M)emory  U)seage  eX)it ->");
		switch (getUpper()) {
		case '\r':;						// handles <enter>
		case 'E': ;
		case 'X': ;
		case 'Q':
					move(TabariFlags.cursr,0);  // erase the menu
					clrtoeol();
					return 0;
		case 'B': return 1;
		case 'P': if (hasProb) return 2;
							else break;
		case 'M': return 3;
		case 'U': return 4;
		case 'V': return 5; // doValidation: invisible to the user
		case 'T': return 6; // showTags: invisible to the user
		}
		if (hasProb) correctMenu("Please enter B, P, M, U, or X");
		else correctMenu("Please enter B, M, U, or X"); 
	}	
} // OtherMenu

bool ReadArg(int argc, char *argv[],  char *sfilename)		// Process command line
	// Process command line arguments
{
	if (argv[1][0] != '-') {
		strcpy(sfilename, argv[1]);
		return 1;
	}
	else {
		if (argv[1][1] == 'v') {
			WriteLine("\n\nPress any key to return to the operating system.");
			getch();
			exit(1);		// ### we don't actually *use* the '1' at the moment...
		}
		if (argv[1][1] == 'c') {
			strcpy(sfilename, "Validate.project");
//	    if (TabariFlags.fAutoCodemode < 0) Pause();  // debug
			return 1;	
		}
		if (argv[1][1] != 'a') {
			ShowWarningError("Unrecognized argument in command line","Command line input will be ignored",shError00);
			return 0;
		}
			switch (argv[1][2]) {  // currently only handles -a* option   
				case 'n' :
				case '\0':  TabariFlags.fAutoCodemode = 0; break;  // default and 'None'  ### replace the magic numbers here; also let 0 indicate OFF...
				case 'd' :  TabariFlags.fAutoCodemode = 1; break;  // 'Date'
				case 'f' :  TabariFlags.fAutoCodemode = 2; break;  // 'Full'
				case 'q' :  TabariFlags.fAutoCodemode = 4; break;  // quiet mode: does not ask to write usage stats
				default  :  ShowWarningError("Unrecognized argument in command line","Command line input will be ignored",shError00);
										return 0;
			} // ### error check other options
			strcpy(sfilename, argv[2]);
			return 1;
	}

} // ReadArg

void firstHaiku(void)
// selects and displays introductory haiku
{
	char str[8];
	time_t cur_time;
	long ka;
	int nseas;
	
	time(&cur_time);
	ka = cur_time % (MAX_SEASON + MAX_INTRO);
	if (ka < MAX_INTRO) showHaiku(shIntro[ka]);
	else {
		strncpy(str,ctime(&cur_time),8);
		switch (str[4]) {
			case 'J': if ('u' == str[5]) nseas = 2; else nseas = 0;
				break;
			case 'A': if ('p' == str[5]) nseas = 1; else nseas = 2;
				break;
			case 'F':
			case 'D': nseas = 0;
				break;
			case 'M': nseas = 1;
				break;
			default: nseas = 3; // S, O, N
		}
		switch (nseas) {
			case 0:  showHaiku(shWinter[ka - MAX_INTRO]);
				break;
			case 1:  showHaiku(shSpring[ka - MAX_INTRO]);
				break;
			case 2:  showHaiku(shSummer[ka - MAX_INTRO]);
				break;
			case 3:  showHaiku(shAutumn[ka - MAX_INTRO]);
		}
	}
} // firstHaiku

void writeIntro(void)
// Writes the introductory screen
{
	WRITESTRING("\n\n");
	WRITESTRING("TTTTTTTT     A      BBBBB       A      RRRRR    II\n");
	WRITESTRING("   TT       A A     B    B     A A     R    R   II\n");
	WRITESTRING("   TT      A   A    B    B    A   A    R    R   II\n");
	WRITESTRING("   TT     AAAAAAA   BBBBB    AAAAAAA   RRRRR    II\n");
	WRITESTRING("   TT     A     A   B    B   A     A   R  R     II\n");
	WRITESTRING("   TT     A     A   B    B   A     A   R   R    II\n");
	WRITESTRING("   TT     A     A   BBBBB    A     A   R    R   II\n\n");
	WRITESTRING("Text Analysis By Augmented Replacement Instructions\n");
	WRITESTRING("Version ");
	WRITESTRING(sRelease);
	WRITESTRING(" (");
	WRITESTRING(sReleaseDate);
	WRITESTRING(")\n\n");
	WRITESTRING("The Event Data Project\n");
	WRITESTRING("Department of Political Science\n");
	WRITESTRING("Pennsylvania State University\nState College, PA 16802, U.S.A.\n");
	WRITESTRING("http://eventdata.psu.edu\n\n");
	WRITESTRING("Redistribution and use of TABARI in source and binary forms, with or without \n");
	WRITESTRING("modification, are permitted under the terms of the GNU General Public License:\n");
	WRITESTRING("http://www.opensource.org/licenses/gpl-license.html\n\n");
	WRITESTRING("Please set your terminal window to display at least 120 characters by 48 lines\n");
	firstHaiku();
	ResetCurs();
} // writeIntro

void ShowAutoHelp(void)	
// Autocoding help screen
{
	ClearScreen();
	WRITESTRING("Autocoding Help\n\n");
	WRITESTRING("Autocoding codes the entire sets of files specified in the project without\n");
	WRITESTRING("pausing.  Once the process has been initiated, it cannot be stopped except\n");
	WRITESTRING("by terminating the program using Cmd-. (period) or Cmd-Q.\n\n");
	WRITESTRING("The speed of autocoding is dramatically affected by the amount of screen \n");
	WRITESTRING("feedback -- the less that is written to the screen, the faster the coding.\n");
	WRITESTRING("There are three levels\n\n");
	WRITESTRING("N)one.  No feedback on individual records; text file names are displayed as\n");
	WRITESTRING("        they are processed. [default] \n\n");
	WRITESTRING("D)ate.  The date and record ID of each record is displayed as the record is\n");
	WRITESTRING("        processed.  This is useful in identifying a record that is causing\n");
	WRITESTRING("        the system to crash. \n\n");
	WRITESTRING("F)ull.  Display the text and the coded events as in standard coding.\n\n");
	WRITESTRING("H)elp.  Shows this screen\n\n");
	WRITESTRING("C)ancel.Exit the autocoding mode.\n\n");
	WRITESTRING("At the menu prompt, type the appropriate key -- N, D, F, H or C -- to select \n");
	WRITESTRING("an option.  <Return> selects the default option, N)one.\n\n");
	Pause();
	ClearScreen();
} // ShowAutoHelp

