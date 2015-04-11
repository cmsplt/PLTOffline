CC = g++
LD = g++
CFLAGS = -O3 -Wall `root-config --cflags`
LIBS = `root-config --libs` -Llib -lMinuit -lSpectrum -lEve -lGeom
INCLUDE = -Iinterface -Idict
OBJS  = $(patsubst src/%.cc,lib/%.o,$(wildcard src/*.cc))
EXECS = $(patsubst bin/%.cc,%,$(wildcard bin/*.cc))
SCRIPTS = $(patsubst scripts/%.cc,%,$(wildcard scripts/*.cc))
EXEOBJS  = $(patsubst bin/%.cc,lib/%.o,$(wildcard bin/*.cc))
LINKDEFCC  = $(patsubst dict/%_linkdef.h,dict/%_dict.cc,$(wildcard dict/*_linkdef.h))
LINKDEFOBJ  = $(patsubst dict/%_linkdef.h,lib/%_dict.o,$(wildcard dict/*_linkdef.h))

print-%:
	@echo $*=$($*)

all: $(LINKDEFCC) $(LINKDEFOBJ) $(OBJS) $(EXEOBJS) $(SCRIPTS) $(EXECS)

lib/%.o : src/%.cc
	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@

lib/%.o : bin/%.cc
	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@

# build test only
#lib/%.o : test/%.cc
#	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@

lib/%.o : scripts/%.cc
	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@

lib/%.o : dict/%.cc
	$(CC) -Wall $(CFLAGS) $(INCLUDE) -c $< -o $@

dict/%_dict.cc : include/%.h dict/%_linkdef.h
	rootcint -f $@ -c $<

% : $(OBJS) lib/%.o
	$(LD) $(LIBS) $(OBJS) $(LINKDEFOBJ) lib/$@.o -o $@





clean:
	rm -f $(EXECS) $(SCRIPTS) lib/*.o dict/*_dict.cc dict/*_dict.h
cleanimg:
	rm -f *.eps *.gif *.jpg plots/*.*
