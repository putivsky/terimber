# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gcc/debug/dbmysql
Bin	=	../../output_gcc/debug
libDirs	=
incDirs	=	-I../../src/os/linux -I../../src -I../../src/db -I../../src/mysqlinc
srcDirs	=	../../src/dbmysql

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O -g

SRCS	=\
	$(srcDirs)/mysqlsrv.cpp\
	$(srcDirs)/termysql.cpp

EXOBJS	=\
	$(oDir)/mysqlsrv.o\
	$(oDir)/termysql.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libdbmysql.a
ALLTGT	=	$(Bin)/libdbmysql.a

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

$(Bin)/libdbmysql.a: $(EXOBJS)
	rm -f $(Bin)/libdbmysql.a
	ar cr $(Bin)/libdbmysql.a $(EXOBJS)
	ranlib $(Bin)/libdbmysql.a

$(oDir)/mysqlsrv.o: $(srcDirs)/mysqlsrv.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/termysql.o: $(srcDirs)/termysql.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
