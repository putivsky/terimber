# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gcc/release/smart
Bin	=	../../output_gcc/release
libDirs	=
incDirs	=	-I../../src/os/linux -I../../src
srcDirs	=	../../src/smart

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O

SRCS	=\
	$(srcDirs)/byterep.cpp\
	$(srcDirs)/varfactory.cpp\
	$(srcDirs)/varmap.cpp\
	$(srcDirs)/varobj.cpp\
	$(srcDirs)/vardatabase.cpp

EXOBJS	=\
	$(oDir)/byterep.o\
	$(oDir)/varfactory.o\
	$(oDir)/varmap.o\
	$(oDir)/varobj.o\
	$(oDir)/vardatabase.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libsmart.a
ALLTGT	=	$(Bin)/libsmart.a

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

$(Bin)/libsmart.a: $(EXOBJS)
	rm -f $(Bin)/libsmart.a
	ar cr $(Bin)/libsmart.a $(EXOBJS)
	ranlib $(Bin)/libsmart.a

$(oDir)/byterep.o: $(srcDirs)/byterep.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/varfactory.o: $(srcDirs)/varfactory.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/varmap.o: $(srcDirs)/varmap.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/varobj.o: $(srcDirs)/varobj.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/vardatabase.o: $(srcDirs)/vardatabase.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
