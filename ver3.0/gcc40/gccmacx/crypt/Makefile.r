# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/release/crypt
Bin	=	../../output_gccmacx/release
libDirs	=
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/crypt

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O

SRCS	=\
	$(srcDirs)/base64.cpp\
	$(srcDirs)/crtable.cpp\
	$(srcDirs)/crypt.cpp\
	$(srcDirs)/cryptimpl.cpp\
	$(srcDirs)/integer.cpp\
	$(srcDirs)/arithmet.cpp

EXOBJS	=\
	$(oDir)/base64.o\
	$(oDir)/crtable.o\
	$(oDir)/crypt.o\
	$(oDir)/cryptimpl.o\
	$(oDir)/integer.o\
	$(oDir)/arithmet.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libcrypt.a
ALLTGT	=	$(Bin)/libcrypt.a

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

$(Bin)/libcrypt.a: $(EXOBJS)
	rm -f $(Bin)/libcrypt.a
	ar cr $(Bin)/libcrypt.a $(EXOBJS)
	ranlib $(Bin)/libcrypt.a


$(oDir)/base64.o: $(srcDirs)/base64.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/crtable.o: $(srcDirs)/crtable.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/crypt.o: $(srcDirs)/crypt.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/cryptimpl.o: $(srcDirs)/cryptimpl.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/integer.o: $(srcDirs)/integer.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/arithmet.o: $(srcDirs)/arithmet.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<


