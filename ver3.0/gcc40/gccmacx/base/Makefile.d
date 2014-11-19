# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/debug/base
Bin	=	../../output_gccmacx/debug
libDirs	=
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/base
osDirs	=	../../src/os/macos

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O -g

SRCS	=\
	$(osDirs)/ostypes.cpp\
	$(srcDirs)/memory.cpp\
	$(srcDirs)/common.cpp\
	$(srcDirs)/date.cpp\
	$(srcDirs)/except.cpp\
	$(srcDirs)/numeric.cpp\
	$(srcDirs)/primitives.cpp\
	$(srcDirs)/log.cpp


EXOBJS	=\
	$(oDir)/ostypes.o\
	$(oDir)/memory.o\
	$(oDir)/common.o\
	$(oDir)/date.o\
	$(oDir)/except.o\
	$(oDir)/numeric.o\
	$(oDir)/primitives.o\
	$(oDir)/log.o


ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libbase.a
ALLTGT	=	$(Bin)/libbase.a

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

$(Bin)/libbase.a: $(EXOBJS)
	rm -f $(Bin)/libbase.a
	ar cr $(Bin)/libbase.a $(EXOBJS)
	ranlib $(Bin)/libbase.a

$(oDir)/ostypes.o: $(osDirs)/ostypes.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/memory.o: $(srcDirs)/memory.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/common.o: $(srcDirs)/common.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/date.o: $(srcDirs)/date.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/except.o: $(srcDirs)/except.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/numeric.o: $(srcDirs)/numeric.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/primitives.o: $(srcDirs)/primitives.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/log.o: $(srcDirs)/log.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
