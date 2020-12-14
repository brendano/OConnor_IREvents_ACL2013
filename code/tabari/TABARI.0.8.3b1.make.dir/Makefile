GCC=g++

all::	TABARI
	@ echo "All done in `pwd`"

coder.o : coder.cp TABARI.h memory.o parser.cp process.cp codes.cp 
	$(GCC) -c coder.cp
	
memory.o: memory.cp TABARI.h parser.cp input.cp 
	$(GCC) -c memory.cp

input.o : input.cp TABARI.h parser.o coder.o
	$(GCC) -c input.cp
	
utilities.o : utilities.cp TABARI.h
	$(GCC) -c utilities.cp

codes.o : codes.cp TABARI.h memory.o parser.cp process.cp 
	$(GCC) -c codes.cp

output.o : output.cp TABARI.h memory.o input.o codes.o
	$(GCC) -c output.cp

parser.o : parser.cp TABARI.h  memory.o process.o
	$(GCC) -c parser.cp

process.o : process.cp TABARI.h memory.o parser.cp coder.o input.cp modify.o interface.o xmlinput.o
	$(GCC) -c process.cp

interface.o : interface.cp TABARI.h
	$(GCC) -c interface.cp

modify.o : modify.cp TABARI.h memory.o codes.o
	$(GCC) -c modify.cp
	
xmlinput.o : xmlinput.cp TABARI.h
	$(GCC) -c xmlinput.cp
	
TABARI.o : TABARI.cp TABARI.h interface.o process.o 
	$(GCC) -c TABARI.cp
	
TABARI: TABARI.o input.o memory.o utilities.o codes.o output.o coder.o parser.o\
 process.o interface.o modify.o xmlinput.o
	$(GCC) -lncurses -o TABARI.0.8.3b1 -g TABARI.o input.o memory.o utilities.o codes.o output.o coder.o parser.o\
 process.o interface.o modify.o xmlinput.o

clean:
	rm -f *.o 
	echo "Done..."
