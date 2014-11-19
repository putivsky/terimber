# Microsoft Developer Studio Project File - Name="aiomsg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=aiomsg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "aiomsg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "aiomsg.mak" CFG="aiomsg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "aiomsg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "aiomsg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "aiomsg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\output_vc6\release"
# PROP Intermediate_Dir "..\..\obj_vc6\release\aiomsg"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Zp4 /MT /W3 /GR /GX /O2 /I "../../src/os/win" /I "../../src" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MSG_EXPORTS" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "aiomsg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\output_vc6\debug"
# PROP Intermediate_Dir "..\..\obj_vc6\debug\aiomsg"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GR /GX /Zi /Od /I "../../src/os/win" /I "../../src" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "MSG_EXPORTS" /FR /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "aiomsg - Win32 Release"
# Name "aiomsg - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_base.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_c.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_comm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_conn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_cpp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_lsnr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_que.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_rpc.cpp
# ADD CPP /I "../../src/aiomsg/x32old"
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_sock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_user.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msgimpl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\aiomsg\aiomsgfactory.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_base.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_c.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_comm.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_conn.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_cpp.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_lsnr.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_que.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_rpc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_sock.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msg_user.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\msgimpl.h
# End Source File
# End Group
# Begin Group "C Files"

# PROP Default_Filter ".c"
# Begin Group "x32"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=..\..\src\aiomsg\x32old\imsg.h
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\x32old\imsg_c.c
# End Source File
# Begin Source File

SOURCE=..\..\src\aiomsg\x32old\imsg_s.c
# End Source File
# End Group
# End Group
# End Target
# End Project
