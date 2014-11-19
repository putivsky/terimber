# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gcc/debug/xml
Bin	=	../../output_gcc/debug
libDirs	=
incDirs	=	-I../../src/os/linux -I../../src
srcDirs	=	../../src/xml

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O -g

SRCS	=\
	$(srcDirs)/defxml.cpp\
	$(srcDirs)/sxs.cpp\
	$(srcDirs)/dtdxml.cpp\
	$(srcDirs)/miscxml.cpp\
	$(srcDirs)/mngxml.cpp\
	$(srcDirs)/parsexml.cpp\
	$(srcDirs)/persxml.cpp\
	$(srcDirs)/storexml.cpp\
	$(srcDirs)/sxml.cpp\
	$(srcDirs)/xmlimpl.cpp\
	$(srcDirs)/xmlmodel.cpp\
	$(srcDirs)/socket.cpp

EXOBJS	=\
	$(oDir)/defxml.o\
	$(oDir)/sxs.o\
	$(oDir)/dtdxml.o\
	$(oDir)/miscxml.o\
	$(oDir)/mngxml.o\
	$(oDir)/parsexml.o\
	$(oDir)/persxml.o\
	$(oDir)/storexml.o\
	$(oDir)/sxml.o\
	$(oDir)/xmlimpl.o\
	$(oDir)/xmlmodel.o\
	$(oDir)/socket.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libxml.a
ALLTGT	=	$(Bin)/libxml.a

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

$(Bin)/libxml.a: $(EXOBJS)
	rm -f $(Bin)/libxml.a
	ar cr $(Bin)/libxml.a $(EXOBJS)
	ranlib $(Bin)/libxml.a

$(oDir)/defxml.o: $(srcDirs)/defxml.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sxs.o: $(srcDirs)/sxs.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/dtdxml.o: $(srcDirs)/dtdxml.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/miscxml.o: $(srcDirs)/miscxml.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/mngxml.o: $(srcDirs)/mngxml.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/parsexml.o: $(srcDirs)/parsexml.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/persxml.o: $(srcDirs)/persxml.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/storexml.o: $(srcDirs)/storexml.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sxml.o: $(srcDirs)/sxml.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/xmlimpl.o: $(srcDirs)/xmlimpl.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/xmlmodel.o: $(srcDirs)/xmlmodel.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/socket.o: $(srcDirs)/socket.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
