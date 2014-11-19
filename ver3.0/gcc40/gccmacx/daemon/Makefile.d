# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/debug/daemon
Bin	=	../../output_gccmacx/debug
libDirs	=
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/daemon

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O -g

SRCS	=\
	$(srcDirs)/daemon.cpp

EXOBJS	=\
	$(oDir)/daemon.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libdaemon.a
ALLTGT	=	$(Bin)/libdaemon.a

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

$(Bin)/libdaemon.a: $(EXOBJS)
	rm -f $(Bin)/libdaemon.a
	ar cr $(Bin)/libdaemon.a $(EXOBJS)
	ranlib $(Bin)/libdaemon.a

$(oDir)/daemon.o: $(srcDirs)/daemon.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
