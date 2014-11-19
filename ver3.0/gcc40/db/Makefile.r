# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gcc/release/db
Bin	=	../../output_gcc/release
libDirs	=
incDirs	=	-I../../src/os/linux -I../../src
srcDirs	=	../../src/db

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O

SRCS	=\
	$(srcDirs)/db.cpp

EXOBJS	=\
	$(oDir)/db.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libdb.a
ALLTGT	=	$(Bin)/libdb.a

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

$(Bin)/libdb.a: $(EXOBJS)
	rm -f $(Bin)/libdb.a
	ar cr $(Bin)/libdb.a $(EXOBJS)
	ranlib $(Bin)/libdb.a

$(oDir)/db.o: $(srcDirs)/db.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
