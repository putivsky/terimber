# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/release/dbodbc
Bin	=	../../output_gccmacx/release
libDirs	=
incDirs	=	-I../../src/os/macos -I../../src -I../../src/db -I../../src/odbcinc
srcDirs	=	../../src/dbodbc

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O

SRCS	=\
	$(srcDirs)/odbcsrv.cpp\
	$(srcDirs)/terodbc.cpp

EXOBJS	=\
	$(oDir)/odbcsrv.o\
	$(oDir)/terodbc.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libdbodbc.a
ALLTGT	=	$(Bin)/libdbodbc.a

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

$(Bin)/libdbodbc.a: $(EXOBJS)
	rm -f $(Bin)/libdbodbc.a
	ar cr $(Bin)/libdbodbc.a $(EXOBJS)
	ranlib $(Bin)/libdbodbc.a

$(oDir)/odbcsrv.o: $(srcDirs)/odbcsrv.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/terodbc.o: $(srcDirs)/terodbc.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
