# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/debug/memdb
Bin	=	../../output_gccmacx/debug
incDirs	=	-I../../src/os/macos -I../../src/db -I../../src
srcDirs	=	../../src/memdb

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O -g

SRCS	=\
	$(srcDirs)/memdb.cpp\
	$(srcDirs)/memtable.cpp\
	$(srcDirs)/memindex.cpp\
	$(srcDirs)/memlookup.cpp

EXOBJS	=\
	$(oDir)/memdb.o\
	$(oDir)/memtable.o\
	$(oDir)/memindex.o\
	$(oDir)/memlookup.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libmemdb.a
ALLTGT	=	$(Bin)/libmemdb.a

# User defines:

#@# Targets follow ---------------------------------

all:	pre\
	$(ALLTGT)

pre: 	
	mkdir -p $(oDir)
	mkdir -p $(Bin)

clean:
	rm -f $(ALLOBJS)
	rm -f $(ALLBIN)

#@# User Targets follow ---------------------------------


#@# Dependency rules follow -----------------------------

$(Bin)/libmemdb.a: $(EXOBJS)
	rm -f $(Bin)/libmemdb.a
	ar cr $(Bin)/libmemdb.a $(EXOBJS)
	ranlib $(Bin)/libmemdb.a

$(oDir)/memdb.o: $(srcDirs)/memdb.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/memtable.o: $(srcDirs)/memtable.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/memindex.o: $(srcDirs)/memindex.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/memlookup.o: $(srcDirs)/memlookup.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
