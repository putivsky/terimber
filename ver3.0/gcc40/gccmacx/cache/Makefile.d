# Standard defines:
CC  	=	g++
LD  	=	g++
oDir	=	../../obj_gcc/debug/cache
Bin	=	../../output_gcc/debug
libDirs	=	-L../../output_gcc/debug
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/cache

LD_FLAGS =	
LIBS	=	-lsmart -lfuzzy -ltokenizer -lxml -ldaemon -laiogate -laiosock -laiocomport -lthreadpool -lbase -lpcre -lpthread -lrt
C_FLAGS	=	-g

SRCS	=\
	$(srcDirs)/cache.cpp\
	$(srcDirs)/sgfactory.cpp

EXOBJS	=\
	$(oDir)/cache.o\
	$(oDir)/sgfactory.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/cache
ALLTGT	=	$(Bin)/cache

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

$(Bin)/cache: $(EXOBJS)
	$(LD) -o $(Bin)/cache $(EXOBJS) $(incDirs) $(libDirs) $(LD_FLAGS) $(LIBS)

$(oDir)/cache.o: $(srcDirs)/cache.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sgfactory.o: $(srcDirs)/sgfactory.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
