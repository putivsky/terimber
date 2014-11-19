# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
Terimber=	../../../../local2.0/AskSearch/3rdParty/c++/terimber
oDir	=	../../obj_gcc/debug/common
Bin	=	../../output_gcc/debug
incDirs	=	-I$(Terimber)/src/os/linux -I$(Terimber)/src

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O -g

SRCS	=\
	httphelp.cpp\
	settings.cpp\
	mapfile.cpp\
	mapfileex.cpp\
	filelock.cpp\
	dirscan.cpp
			

EXOBJS	=\
	$(oDir)/httphelp.o\
	$(oDir)/settings.o\
	$(oDir)/mapfile.o\
	$(oDir)/mapfileex.o\
	$(oDir)/filelock.o\
	$(oDir)/dirscan.o


ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libcommon.so
ALLTGT	=	$(Bin)/libcommon.so

# User defines:

#@# Targets follow ---------------------------------

all:	$(ALLTGT)

objs:	$(ALLOBJS)

cleanobjs:
	rm -f $(ALLOBJS)

cleanbin:
	rm -f $(ALLBIN)

clean:	cleanobjs cleanbin

cleanall:	cleanobjs cleanbin

#@# User Targets follow ---------------------------------


#@# Dependency rules follow -----------------------------

$(Bin)/libcommon.so: $(EXOBJS)
	$(CC) -shared -o $@ $(EXOBJS) 

$(oDir)/httphelp.o: httphelp.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/settings.o: settings.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/mapfile.o: mapfile.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/mapfileex.o: mapfileex.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/filelock.o: filelock.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/dirscan.o: dirscan.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

