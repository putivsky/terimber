# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/release/tokenizer
Bin	=	../../output_gccmacx/release
libDirs	=
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/tokenizer

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O

SRCS	=\
	$(srcDirs)/tokenizer.cpp

EXOBJS	=\
	$(oDir)/tokenizer.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libtokenizer.a
ALLTGT	=	$(Bin)/libtokenizer.a

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

$(Bin)/libtokenizer.a: $(EXOBJS)
	rm -f $(Bin)/libtokenizer.a
	ar cr $(Bin)/libtokenizer.a $(EXOBJS)
	ranlib $(Bin)/libtokenizer.a

$(oDir)/tokenizer.o: $(srcDirs)/tokenizer.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
