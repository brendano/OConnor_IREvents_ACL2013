// 																			TABARI Project
//__________________________________________________________________________________
// 																			TABARI.cp
//	TABARI: Text Analysis by Augmented Replacement Instructions
//
//	The Event Data Project at Penn State
//	Dept of Political Science
//	Pennsylvania State University
//	227 Pond Laboratory
//	University Park, PA, 16802 U.S.A.
//	http://eventdata.psu.edu
//	Email: schrodt@psu.edu
//
//__________________________________________________________________________________
//  														Notes on documentation
//
//	This program is, in theory, sufficiently well documented that it should be 
//	possible to modify the source code and still have it work.  This is, after all,
//	open source software.  While this may be possible, anyone trying to do so is
//	strongly advised to also consult the TABARI manual, since this provides 
//	considerably more information about the interpretation and objectives of various
//	components of the program.
//
//	TABARI makes *extensive* use of strings and character array manipulation.  The 
//	basic C++ idioms for working with character arrays are generally *not* commented,
//	and are usually written in a very abbreviated form because they are so pervasive 
//	that it is assumed anyone reading this code will quickly become familiar with
//	them.  The C <string.h> functions have been used rather than the C++ string 
//	class because most of the time the program is working with characters stored in
//	the general character arrays such as charBuf rather than with free-standing C++ 
//	strings.
//
//	>>> GUIDE TO VARIABLE NAMING CONVENTIONS <<<
//
//	Scope:
//			macros				ALL CAPS
//			global constants	 ALL CAPS
//			global variables   Title Case
//			local to class     first word lower case, then Title Case (e.g. storeDatedString)
//			local to function  all lower case
//			struct elements 	 all lower case
//	
//	Types: usually but not obsessively indicated by first letter
//			strings (char[])		s
//			chars					c
//			indices					i
//			boolean flags			f
//			pointers				p
//
//	Other variable naming conventions:
//
//	1. Loop variables are usually named ka, kb, kc, etc.  [Why? -- because "k" is an
//	   integer in FORTRAN and the mechanical keypunch machines I first started on
//	   required a slow mechanical shift operation to punch numbers, so I used ka, kb 
//	   rather than  k1, k2, etc.  Old habits die hard...].  This convention tends to 
//	   get extended to any other arbitrary variable, i.e. if 's' is one string, 'sa' 
//	   will likely be the next unless come other compelling name has suggested itself.
//
//	2. The ubiquitous char *pst, *psa found in many functions are pointers to characters 
//		in strings; they are usually used to go through a string processing it.
//
//	3. typedefs are all lower case, consistent with the C++ types.  Classes and 
//	   structures are Title Case

//	>>> FORMATTING CONVENTIONS <<<
//
//	Here are my conventions on the standard points of dispute
//
//	1. if ( ) {
//		}
//		else {
//		} 
//
//	2. if () <stmnt>;
//	   else <stmnt>;
//
//	3. All binary operators (e.g. ==, <=, ||, =, +=, etc) have spaces on each side.
//	   
//	4. When multiple logical conditions of any complexity are used, they are on
//	   separate lines.
//
//	5. The constant in an == test comes first! -- i.e. if (1 == ka), not if (ka == 1)
//
//	6. Parentheses are used anytime the precedence of operations is in doubt (even if
//	   in doubt by the programmer, not the compiler.  *Particularly* by the programmer...)
//
//	7. This was originally formatted with a tab size of 2
//	 
//	8. Assignments ("=") in if() statements are always commented, as are empty while () 
//		loops
//
//	9. Comments containing ### signal places where there are unresolved coding issues,
//	   for example placed where range or validity checks need to be made.
//
//	10. Comments containing *** signal debugging code.  All nontrivial debugging code 
//	   and test functions are being left in at the moment; a good linker will keep 
//	   this out of the compiled code.
 
//__________________________________________________________________________________
//  														Status of the source code
//
//	This version of the program has been compiled and tested under both gcc 4.2.1
//  (which may have some Apple modifications) and Apple/s XCode 4.2. It is 
//  periodically compiled using 'make' on various Linux systems (typically clusters)
//  and no problems have been encountered. It should also compile without warnings.
//
//	On all systems, it coded 26,000 Reuters leads dealing with the Middle East for
//	1987 to 1990 using the "APSA.96" dictionaries from the KEDS project (these are
//	available at  http://eventdata.psu.edu/data.hmtl).  While this not preclude
//	the possibility that the program will crash when coding record number 26,001,
//	it does indicate that the program is generally working. Released code has also
//  been run on the Validation.project validation suite.
//
//	The source code makes extensive use of the ISO extensions to C++, notably true,
//	false, bool, and a few of the new C++ I/O functions.  Most of the C++ is very 
//  straightforward --	there is no overriding of operators, inheritance, or STL,
//	and there are fewer instances of try/catch than there should be. I.e. this is
//  more C than C++. Finally, there are still a few places in the source code where 
//  features have been "roughed in" but not completed; at this point they will 
//  almost certain never be completed so I've tried to remove such code.  
//
//__________________________________________________________________________________
//  																Debugging tools
//
//	There is an assortment of residule debugging code and procedures scattered 
//	throughout, mostly just simple flags to indicate where the program is working.
//	Most of the serious debugging was done in the gdb environment.
//  A few additional helpful tricks:
//
//  1. 	Parser.writeParsing("Comment string");  // *** debug
//      will send the parse output and tags to the <problemfile>, where it can
//      be read at leisure.
//
//  2.  Parser.showParsing(); Pause(); // *** debug 									
//      will display the parse output on the screen, then pause  
//	
//  3.  #define TAB_DEBUG = TRUE flag (TABARI.h)   
//	When this is set, the program automatically goes to the C)oding option, reads a file 
//	specified in the command line if that is present, otherwise sets this in TABARI.cp,
//  and sets sCoder to "debug".  At termination in the
//	function ProcessorClass::endSession, it deletes the new event file and and skips 
//	the over-writing of the .actors and .verbs files even if they have been changed.  
//  There are a large number of additional debugging options that are commented-out
//	or bypassed using #if FALSE directives; these can be turned on as needed.
//
//  There are three additional options that provide detailed traces of the parsing
//  and matching:
//
//	#define DEBUG_LEX 		// secondary debugging in matchLex and makeSynt 
//                           (parser.c)
//	#define DEBUG_VERB		// secondary debugging in doVerb and checkPattern 
//                           (coder.c)
//	#define DEBUG_PAT		// secondary debugging in checkPattern and support functions
//                           (coder.c)
//
//  These use <iostream>: DEBUG_PAT has been shifted to writing the problem file Processor.fprob
//  [see below]; DEBUG_VERB and DEBUG_LEX still use cout (based on pre-ncurses versions) and
//  probably won't work now; a global replace s/cout/Processor.fprob should correct this.
//
//  Also see: "Debugging Guides" in the initial comments in memory.cp -- this provides
//            code snippets for generating an assortment of useful output.
//
//  <09.01.05>
//  Using the tag <problemfile!> -- note the addition of '!' --  causes a ".debug"  
//  suffix rather than the session number (so the file name remains constant and can
//  be monitored in BBEdit...), to be added to the problem file name, and the file 
//  is always saved, even if the O)ther/P)roblem menu option is not used (under default 
//  circumstances, an empty <problemfile> is deleted at the end of a session. This allows
//  the Processor.fprob file to be used for debugging output with iostream formatting. 
//  fprob is opened in Processor.initSession before any of the dictionaries are read, 
//  so it is available for almost all of the program.
//
//  ParserClass:: writeParsing(char *sinfo) can also be used to write the current 
//  status of the parsing -- including the tag structure -- at critical points.
//
//  Also: include an <errorfile> when debugging -- sometimes the existing error alerts
//        reported therein will be useful.
//
//__________________________________________________________________________________
//  															Notes on programming style
//
//	This is the first (and still really the only) large C++ program I've written
//	and some of the structure is probably quite idiosyncratic. More generally,
//	I'm guessing this probably reads like a C program with C++ storage and control 
//  structures; it makes relatively little use of most elements of C++. That said,
//  it should be pretty decent C, and shouldn't read like either a Fortran or
//  Pascal program. To say nothing of a Java program... ("Beware! Here be pointers,
//  me hearties...")
//
//	Variable declarations are typically done at the beginning of functions (okay,
//  that *does* look like Pascal...) rather than proximate to blocks, which may
//  lead to sub-optimal register allocation. I'm gradually changing this but
//  only gradually.
//
//__________________________________________________________________________________
//
//	Required files:
//	TABARI.cp 			main()
//	TABARI.h			headers
//	mergefiles.cp		dictionary comparison routines
//	utilities.cp		global utilities, mostly string functions
//	memory.cp			get/put/find routines for various data structures
//	codes.cp			get/put for codes
//	process.cp			primary record processing loop
//	parser.cp			parser
//	coder.cp			coder
//	input.cp			file input routines
//	output.cp			file output routines
//	interface.cp		all user-interface routines except dictionary modification
//	modify.cp			dictionary modification
//  xmlinput.cp         experimental and only partial developed XML input system
//
//__________________________________________________________________________________
//
//	Copyright (c) 2000 - 2012  Philip A. Schrodt.  All rights reserved.
//
// 	Redistribution and use in source and binary forms, with or without modification,
// 	are permitted under the terms of the GNU General Public License, version 3.0:
// 	http://www.opensource.org/licenses/gpl-license.html
//
//	Report bugs to: schrodt@psu.edu
//
//	The most recent version of this code is available from the KEDS Web site:
//		http://eventdata.psu.edu
//
//  Last update: 24 January 2012
//__________________________________________________________________________________
//  																	Release history
//
// See TABARI.h file
//
//___________________________________________________________________________________
// 																																			Headers

#include "TABARI.h"

#if FALSE 	// *** debugging
extern LiteralsClass	 	Literals;
extern ParserClass 		 	Parser;
extern CoderClass 		 	Coder;
extern CodeStoreClass		CodeStore;
extern ReadXMLClass 		ReadXML;
#endif

extern CodeStoreClass		CodeStore; // ###
extern PhrasesClass 		Phrases;  // ###

extern ReadFilesClass 	ReadFiles;	
extern WriteFilesClass 	WriteFiles;
extern MergeFilesClass MergeFiles; 
extern ProcessorClass  Processor;

//___________________________________________________________________________________
// 																																		Declare globals


void MemTest(void);
void InitTABARI(void);		
int MenuMain(void);
int RecordMenu(void); // Menu for the primary read-parse-code loop
void HelpMain(void);

//___________________________________________________________________________________
// 																																			

int main(int argc, char *argv[])
{
instring sfilename;
#if !(TAB_DEBUG)				// working version of program

	InitTABARI();
	if (argc > 1 && ReadArg(argc,argv,sfilename)) { 
		WriteLine(" ");
		Processor.doProcess(sfilename);
	}
	else {
		GetInput(sfilename,"\nEnter project file name, or <return> for the demonstration file");
		if (!sfilename[0] || (' ' == sfilename[0])) strcpy(sfilename,"TABARI.Demo.project");
		if (!strcmp(sfilename,"~v")) strcpy(sfilename,"Validate.project");
		Processor.doProcess(sfilename);
	}
#else  // ***  debugging version
	InitTABARI();
	WriteLine("DEBUGGING MODE");
	if (argc > 1) { Processor.doProcess(argv[1]);}  // get the file name from command line
	else {  // specify a file name
	//  strcpy(sfilename,"synset.devel.project");
	//	strcpy(sfilename,"TABARI.Demo.project");
	 strcpy(sfilename,"Validate.project");
	//	strcpy(sfilename,"LV.DEBUG.PROJECT");    // and for the rest, a walk down memory lane...
	//	strcpy(sfilename,"Attrib.test.project"); // of hours of slogging through debugging...
	//	strcpy(sfilename,"Issues.project");      // memories...memories...
	//	strcpy(sfilename,"Freqs.project");       // there needs to be a haiku for that...
	//	strcpy(sfilename,"LEVANT.CAMEO.PROJECT");//    The names of these files
	//	strcpy(sfilename,"COMPOUND.DEBUG.PROJECT");//  Seared upon my memory
	//  strcpy(sfilename,"merge.b4.project");    //    Long hours debugging
	//	strcpy(sfilename,"TimeShift.test.project");
	//	strcpy(sfilename,"TABARI.APSA96.project");
	//	strcpy(sfilename,"XML.Demo.project");
		Processor.doProcess(sfilename);
	}
#endif  // # else debug

} // main()

//___________________________________________________________________________________
// 																																		Old test code

//  CodeStore.codeTest();
// exit(0);

//	showHaiku(shError91);
//	Pause();

//  CodeStore.codeTest();
//  exit(0);

//	Processor.filterText();
//	MemTest();

//ReadFiles.testOptions();
//Parser.testCplx();
//imain = 1;
//exit(0);

#if FALSE
	cout << "XML DEBUGGING MODE\n";
	ReadXML.openXMLFile("XML.DemoText.XML");
	while (true) ReadXML.readXMLRecord();
#endif

#if FALSE // read/write dictionaries test
	ReadFiles.readActors("Kv3.actors");
//	ReadFiles.readVerbs("Kv3.verbs");
//	ReadFiles.readVerbs("TimeShift.test.verbs");
	cout << "\n----------------------------\n\n";
	Literals.alphaList();
	cout << "\n----------------------------\n\n";
	WriteFiles.writeActors("scr.actors");
//	WriteFiles.writeVerbs("scr.verbs");
#endif

#if FALSE 
// test Phrases store/get
tokptr pt; 
instring sz;
pt = Phrases.store("SUGGESTED_+ * DEMAND_{ GENEVA_ | INTERNATIONAL }_CONVENTIONS ");
cout << Phrases.get(sz,pt) << endl; Pause();
pt = Phrases.store("SUGGESTED_+ * DEMAND_{GENEVA_|INTERNATIONAL}_CONVENTIONS ");
cout << Phrases.get(sz,pt) << endl; Pause();
#endif
