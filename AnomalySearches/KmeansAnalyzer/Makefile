CC=g++

# -Wno-sign-compare turns off warnings for comparisons between signed and unsigned variable
CFLAGS=-c -g -Wall $(shell root-config --cflags) -I ~/boost_1_79_0 -I ./ -Wno-sign-compare

LDFLAGS=$(shell root-config --glibs --libs)

all: main

main: main.cc KmeansAnalyzer.cc KmeansAnalyzer.h Makefile
	$(CC) -o KmeansAnalyzer.o KmeansAnalyzer.cc $(CFLAGS)
	$(CC) -o main.o main.cc $(CFLAGS)
	$(CC) -o main main.o KmeansAnalyzer.o $(LDFLAGS)

clean: 
	rm -rf *.o
	rm -rf main

pack: main.cc KmeansAnalyzer.cc KmeansAnalyzer.h Makefile clean
	rm -rf packed.zip
	zip -9 -r packed.zip ./ -x '*.zip' -x './input/Slink_20160427.093307.root' -x '*.o' -x 'main'

# debugging:
# https://stackoverflow.com/questions/58581500/how-to-fix-debugger-in-vscode-if-you-have-makefile-project-on-c