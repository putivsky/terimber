# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/debug/aiosock
Bin	=	../../output_gccmacx/debug
libDirs	=
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/aiosock

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O -g

SRCS	=\
	$(srcDirs)/aiosock.cpp

EXOBJS	=\
	$(oDir)/aiosock.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libaiosock.a
ALLTGT	=	$(Bin)/libaiosock.a

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

$(Bin)/libaiosock.a: $(EXOBJS)
	rm -f $(Bin)/libaiosock.a
	ar cr $(Bin)/libaiosock.a $(EXOBJS)
	ranlib $(Bin)/libaiosock.a

$(oDir)/aiosock.o: $(srcDirs)/aiosock.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
