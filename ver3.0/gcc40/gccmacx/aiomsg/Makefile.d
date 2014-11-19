# Standard defines:
CC  	=	g++
LD  	=	g++
CCOMP	=	gcc
oDir	=	../../obj_gccmacx/debug/aiomsg
Bin	=	../../output_gccmacx/debug
libDirs	=
incDirs	=	-I../../src/os/macos -I../../src
srcDirs	=	../../src/aiomsg

LD_FLAGS =	
LIBS	=	
C_FLAGS	=	-O -g

SRCS	=\
	$(srcDirs)/msg_c.cpp\
	$(srcDirs)/msg_comm.cpp\
	$(srcDirs)/msg_conn.cpp\
	$(srcDirs)/msg_cpp.cpp\
	$(srcDirs)/msg_lsnr.cpp\
	$(srcDirs)/msg_que.cpp\
	$(srcDirs)/msg_sock.cpp\
	$(srcDirs)/msg_user.cpp\
	$(srcDirs)/msgimpl.cpp\
	$(srcDirs)/msg_base.cpp

EXOBJS	=\
	$(oDir)/msg_c.o\
	$(oDir)/msg_comm.o\
	$(oDir)/msg_conn.o\
	$(oDir)/msg_cpp.o\
	$(oDir)/msg_lsnr.o\
	$(oDir)/msg_que.o\
	$(oDir)/msg_sock.o\
	$(oDir)/msg_user.o\
	$(oDir)/msgimpl.o\
	$(oDir)/msg_base.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/libmsg.a
ALLTGT	=	$(Bin)/libmsg.a

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

$(Bin)/libmsg.a: $(EXOBJS)
	rm -f $(Bin)/libmsg.a
	ar cr $(Bin)/libmsg.a $(EXOBJS)
	ranlib $(Bin)/libmsg.a

$(oDir)/msg_c.o: $(srcDirs)/msg_c.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/msg_comm.o: $(srcDirs)/msg_comm.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/msg_conn.o: $(srcDirs)/msg_conn.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/msg_cpp.o: $(srcDirs)/msg_cpp.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/msg_lsnr.o: $(srcDirs)/msg_lsnr.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/msg_que.o: $(srcDirs)/msg_que.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/msg_sock.o: $(srcDirs)/msg_sock.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/msg_trd.o: $(srcDirs)/msg_trd.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/msg_user.o: $(srcDirs)/msg_user.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/msgimpl.o: $(srcDirs)/msgimpl.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/msg_base.o: $(srcDirs)/msg_base.cpp
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
