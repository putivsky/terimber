# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gcc/debug/alg
Bin	=	../../output_gcc/debug
libDirs	=
incDirs	=	-I../../src/os/linux -I../../src
srcDirs	=	../../src/alg
osDirs	=	../../src/os/linux

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O -g

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
