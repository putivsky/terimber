# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/debug/aiogate
Bin	=	../../output_gccmacx/debug
libDirs	=
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/aiogate

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O -g

SRCS	=\
	$(srcDirs)/aiogate.cpp

EXOBJS	=\
	$(oDir)/aiogate.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libaiogate.a
ALLTGT	=	$(Bin)/libaiogate.a

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

$(Bin)/libaiogate.a: $(EXOBJS)
	rm -f $(Bin)/libaiogate.a
	ar cr $(Bin)/libaiogate.a $(EXOBJS)
	ranlib $(Bin)/libaiogate.a

$(oDir)/aiogate.o: $(srcDirs)/aiogate.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
