# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gcc/release/fuzzy
Bin	=	../../output_gcc/release
libDirs	=
incDirs	=	-I../../src/os/linux -I../../src
srcDirs	=	../../src/fuzzy

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O

SRCS	=\
	$(srcDirs)/fuzzyphonetic.cpp\
	$(srcDirs)/fuzzyimpl.cpp\
	$(srcDirs)/fuzzywrapper.cpp

EXOBJS	=\
	$(oDir)/fuzzyphonetic.o\
	$(oDir)/fuzzyimpl.o\
	$(oDir)/fuzzywrapper.o


ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libfuzzy.a
ALLTGT	=	$(Bin)/libfuzzy.a

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

$(Bin)/libfuzzy.a: $(EXOBJS)
	rm -f $(Bin)/libfuzzy.a
	ar cr $(Bin)/libfuzzy.a $(EXOBJS)
	ranlib $(Bin)/libfuzzy.a

$(oDir)/fuzzyphonetic.o: $(srcDirs)/fuzzyphonetic.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/fuzzyimpl.o: $(srcDirs)/fuzzyimpl.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/fuzzywrapper.o: $(srcDirs)/fuzzywrapper.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

