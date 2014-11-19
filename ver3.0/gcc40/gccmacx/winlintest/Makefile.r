# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gcc/release/winlintest
Bin	=	../../output_gcc/release
libDirs	=	-L../../output_gcc/release
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/winlintest

LD_FLAGS =	
LIBS	=	-laiogate -laiosock -laiocomport -lthreadpool -laiofile -lcrypt -lxml -lbase -lpthread
C_FLAGS	=	-O

SRCS	=\
	$(srcDirs)/main.cpp\
	$(srcDirs)/socketport_ut.cpp\
	$(srcDirs)/file_ut.cpp

EXOBJS	=\
	$(oDir)/main.o\
	$(oDir)/socketport_ut.o\
	$(oDir)/file_ut.o


ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/winlintest
ALLTGT	=	$(Bin)/winlintest

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

$(Bin)/winlintest: $(EXOBJS)
	$(LD) -o $(Bin)/winlintest $(EXOBJS) $(incDirs) $(libDirs) $(LD_FLAGS) $(LIBS)

$(oDir)/main.o: $(srcDirs)/main.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/socketport_ut.o: $(srcDirs)/socketport_ut.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/file_ut.o: $(srcDirs)/file_ut.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

