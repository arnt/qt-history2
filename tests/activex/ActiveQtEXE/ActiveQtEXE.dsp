# Microsoft Developer Studio Project File - Name="ActiveQtEXE" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ActiveQtEXE - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ActiveQtEXE.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ActiveQtEXE.mak" CFG="ActiveQtEXE - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ActiveQtEXE - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "ActiveQtEXE - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ActiveQtEXE"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ActiveQtEXE - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /ZI /Od /I "$(QTDIR)\include" /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "QT_DLL" /D "UNICODE" /FR /YX"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x814 /d "_DEBUG"
# ADD RSC /l 0x814 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(QTDIR)\lib\qt-mt301.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib delayimp.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"c:\depot\qt\main\lib"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=.\Debug\ActiveQtEXE.exe
InputPath=.\Debug\ActiveQtEXE.exe
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"$(TargetPath)" /RegServer 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	echo Server registration done! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ActiveQtEXE - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ActiveQtEXE___Win32_Release"
# PROP BASE Intermediate_Dir "ActiveQtEXE___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /ZI /Od /I "$(QTDIR)\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "QT_DLL" /FR /YX"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /O1 /I "$(QTDIR)\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "QT_DLL" /D "UNICODE" /YX"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Z<none> /Fr
# ADD MTL /nologo /D "NDEBUG" /mktyplib203
# ADD BASE RSC /l 0x814 /d "_DEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(QTDIR)\lib\qt-mt301.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib delayimp.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"c:\depot\qt\main\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 $(QTDIR)\lib\qt-mt301.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib delayimp.lib /nologo /subsystem:windows /pdb:none /machine:I386 /libpath:"c:\depot\qt\main\lib"
# SUBTRACT LINK32 /debug
# Begin Custom Build - Performing registration
OutDir=.\Release
TargetPath=.\Release\ActiveQtEXE.exe
InputPath=.\Release\ActiveQtEXE.exe
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"$(TargetPath)" /RegServer 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	echo Server registration done! 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "ActiveQtEXE - Win32 Debug"
# Name "ActiveQtEXE - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ActiveQtEXE.cpp
# End Source File
# Begin Source File

SOURCE=.\ActiveQtEXE.idl
# ADD MTL /tlb ".\ActiveQtEXE.tlb" /h "ActiveQtEXE.h" /iid "ActiveQtEXE_i.c" /Oicf
# End Source File
# Begin Source File

SOURCE=.\ActiveQtEXE.rc
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TestWidget.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\QActiveX.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TestWidget.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ActiveQtEXE.rgs
# End Source File
# Begin Source File

SOURCE=.\qexetest.bmp
# End Source File
# Begin Source File

SOURCE=.\QExeTest.rgs
# End Source File
# End Group
# Begin Source File

SOURCE=.\TestControl.htm
# End Source File
# End Target
# End Project
# Section ActiveQtEXE : {00000000-0000-0000-0000-800000800000}
# 	1:9:IDR_QTEST:104
# 	1:9:IDB_QTEST:103
# 	1:9:IDD_QTEST:105
# End Section
