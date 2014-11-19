# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gcc/debug/pcre
Bin	=	../../output_gcc/debug
libDirs	=
incDirs	=	-I../../src/os/linux -I../../src
srcDirs	=	../../src/pcre
osDirs	=	../../src/os/linux

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O -g

SRCS	=\
	$(srcDirs)/study.c\
	$(srcDirs)/pcre.c


EXOBJS	=\
	$(oDir)/study.o\
	$(oDir)/pcre.o


ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libpcre.a
ALLTGT	=	$(Bin)/libpcre.a

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

$(Bin)/libpcre.a: $(EXOBJS)
	rm -f $(Bin)/libpcre.a
	ar cr $(Bin)/libpcre.a $(EXOBJS)
	ranlib $(Bin)/libpcre.a

$(oDir)/study.o: $(srcDirs)/study.c
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/pcre.o: $(srcDirs)/pcre.c
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
