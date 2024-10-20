CC = g++
CFLAGS = -c -Wall
FILES_CPP = main.cpp Compiler.cpp Processor.cpp ConsoleCmd.cpp Stack.cpp Hash.cpp GlobalInclude.cpp
OBJECTS = $(FILES_CPP:.cpp=.o)
OBJECTS_H = $(FILES_CPP:.cpp=.h)

all: $(SOURCES) main


main: $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@

.cpp.o: $(OBJECTS_H)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o hello

clean.exe:
	rm -rf *.exe hello

