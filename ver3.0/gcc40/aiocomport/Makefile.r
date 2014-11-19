# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gcc/release/aiocomport
Bin	=	../../output_gcc/release
libDirs	=
incDirs	=	-I../../src/os/linux -I../../src
srcDirs	=	../../src/aiocomport

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O

SRCS	=\
	$(srcDirs)/aiocomport.cpp

EXOBJS	=\
	$(oDir)/aiocomport.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libaiocomport.a
ALLTGT	=	$(Bin)/libaiocomport.a

# User defines:

#@# Targets follow ---------------------------------

all:	pre\
	$(ALLTGT)

pre: 	
	mkdir -p $(oDir)
	mkdir -p $(Bin)

clean:
	rm -f $(ALLOBJS)\
	rm -f $(ALLBIN)



#@# User Targets follow ---------------------------------


#@# Dependency rules follow -----------------------------

$(Bin)/libaiocomport.a: $(EXOBJS)
	rm -f $(Bin)/libaiocomport.a
	ar cr $(Bin)/libaiocomport.a $(EXOBJS)
	ranlib $(Bin)/libaiocomport.a

$(oDir)/aiocomport.o: $(srcDirs)/aiocomport.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

