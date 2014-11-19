# Microsoft Developer Studio Project File - Name="base" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=base - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "base.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "base.mak" CFG="base - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "base - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "base - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "base - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\output_vc6\release"
# PROP Intermediate_Dir "..\..\obj_vc6\release\base"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Zp4 /MT /W3 /GR /GX /O2 /I "../../src/os/win" /I "../../src" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fr /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "base - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\output_vc6\debug"
# PROP Intermediate_Dir "..\..\obj_vc6\debug\base"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GR /GX /Zi /Od /I "../../src/os/win" /I "../../src" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /FD /c
# SUBTRACT CPP /YX
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

# Name "base - Win32 Release"
# Name "base - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\base\common.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\date.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\except.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\memory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\numeric.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\os\win\ostypes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\primitives.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\base\algorith.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\algorith.hpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\common.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\common.hpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\date.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\except.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\keymaker.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\keymaker.hpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\list.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\list.hpp
# End Source File
# Begin Source File

SOURCE=..\..\src\log.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\map.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\map.hpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\memory.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\memory.hpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\ns.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\number.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\number.hpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\numeric.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\primitives.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\socket.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\stack.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\stack.hpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\string.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\string.hpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\template.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\template.hpp
# End Source File
# Begin Source File

SOURCE=..\..\src\base\types.h
# End Source File
# Begin Source File

SOURCE=..\..\src\os\value.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\vector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\vector.hpp
# End Source File
# Begin Source File

SOURCE=..\..\src\os\vttypes.h
# End Source File
# Begin Source File

SOURCE=..\..\src\base\xmlconfig.h
# End Source File
# End Group
# End Target
# End Project
