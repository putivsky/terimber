# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/release/aiofile
Bin	=	../../output_gccmacx/release
libDirs	=
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/aiofile

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O

SRCS	=\
	$(srcDirs)/aiofile.cpp

EXOBJS	=\
	$(oDir)/aiofile.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libaiofile.a
ALLTGT	=	$(Bin)/libaiofile.a

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

$(Bin)/libaiofile.a: $(EXOBJS)
	rm -f $(Bin)/libaiofile.a
	ar cr $(Bin)/libaiofile.a $(EXOBJS)
	ranlib $(Bin)/libaiofile.a

$(oDir)/aiofile.o: $(srcDirs)/aiofile.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

