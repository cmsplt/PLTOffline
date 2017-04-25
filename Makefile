CC = g++
LD = g++
CFLAGS = -O3 -Wall `root-config --cflags`
LIBS = `root-config --libs` -Llib -lMinuit -lSpectrum -lEve -lGeom
INCLUDE = -Iinterface
OBJS  = $(patsubst src/%.cc,lib/%.o,$(wildcard src/*.cc))
EXECS = $(patsubst bin/%.cc,%,$(wildcard bin/*.cc))
SCRIPTS = $(patsubst scripts/%.cc,%,$(wildcard scripts/*.cc))
EXEOBJS  = $(patsubst bin/%.cc,lib/%.o,$(wildcard bin/*.cc))

# Exclude TestBufferReader from compilation since most systems that
# you'll compile this on don't have zmq installed.
EXEOBJS := $(filter-out lib/TestBufferReader.o,$(EXEOBJS))
EXECS := $(filter-out TestBufferReader,$(EXECS))

print-%:
	@echo $*=$($*)

all: $(OBJS) $(EXEOBJS) $(SCRIPTS) $(EXECS)

lib/%.o : src/%.cc
	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@

lib/%.o : bin/%.cc
	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@

lib/%.o : scripts/%.cc
	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@

% : $(OBJS) lib/%.o
	$(LD) $(LIBS) $(OBJS) $(LINKDEFOBJ) lib/$@.o -o $@





clean:
	rm -f $(EXECS) $(SCRIPTS) lib/*.o
cleanimg:
	rm -f *.eps *.gif *.jpg plots/*.*
