# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/release/base
Bin	=	../../output_gccmacx/release
libDirs	=
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/alg
osDirs	=	../../src/os/macos

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O

SRCS	=\
	$(srcDirs)/mstaccess.cpp


EXOBJS	=\
	$(oDir)/mstaccess.o


ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libalg.a
ALLTGT	=	$(Bin)/libalg.a

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

$(Bin)/libalg.a: $(EXOBJS)
	rm -f $(Bin)/libalg.a
	ar cr $(Bin)/libalg.a $(EXOBJS)
	ranlib $(Bin)/libalg.a

$(oDir)/mstaccess.o: $(srcDirs)/mstaccess.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

