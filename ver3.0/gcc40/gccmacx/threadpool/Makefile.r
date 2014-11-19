# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/release/threadpool
Bin	=	../../output_gccmacx/release
libDirs	=
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/threadpool

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O

SRCS	=\
	$(srcDirs)/threadpool/threadpool.cpp\
	$(srcDirs)/threadpool/thread.cpp\
	$(srcDirs)/threadpool/timer.cpp

EXOBJS	=\
	$(oDir)/threadpool.o\
	$(oDir)/thread.o\
	$(oDir)/timer.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libthreadpool.a
ALLTGT	=	$(Bin)/libthreadpool.a

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

$(Bin)/libthreadpool.a: $(EXOBJS)
	rm -f $(Bin)/libthreadpool.a
	ar cr $(Bin)/libthreadpool.a $(EXOBJS)
	ranlib $(Bin)/libthreadpool.a

$(oDir)/threadpool.o: $(srcDirs)/threadpool.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/thread.o: $(srcDirs)/thread.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/timer.o: $(srcDirs)/timer.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
