# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=trivial - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to trivial - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "trivial - Win32 Release" && "$(CFG)" !=\
 "trivial - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "trivial.mak" CFG="trivial - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "trivial - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "trivial - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "trivial - Win32 Debug"
RSC=rc.exe
CPP=cl.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "trivial - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\trivial.dll"

CLEAN : 
	-@erase "$(INTDIR)\trivial.obj"
	-@erase "$(INTDIR)\trivial.res"
	-@erase "$(OUTDIR)\trivial.dll"
	-@erase "$(OUTDIR)\trivial.exp"
	-@erase "$(OUTDIR)\trivial.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/trivial.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/trivial.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/trivial.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib /nologo /subsystem:windows /verbose /dll /machine:I386 /nodefaultlib:"libcmtd"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib wsock32.lib /nologo /subsystem:windows /verbose /dll\
 /incremental:no /pdb:"$(OUTDIR)/trivial.pdb" /machine:I386\
 /nodefaultlib:"libcmtd" /out:"$(OUTDIR)/trivial.dll"\
 /implib:"$(OUTDIR)/trivial.lib" 
LINK32_OBJS= \
	"$(INTDIR)\trivial.obj" \
	"$(INTDIR)\trivial.res" \
	"..\..\..\..\lib\qnp.lib" \
	"..\..\..\..\lib\qt.lib"

"$(OUTDIR)\trivial.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "trivial - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\nptrivial.dll"

CLEAN : 
	-@erase "$(INTDIR)\trivial.obj"
	-@erase "$(INTDIR)\trivial.res"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\nptrivial.dll"
	-@erase "$(OUTDIR)\nptrivial.exp"
	-@erase "$(OUTDIR)\nptrivial.ilk"
	-@erase "$(OUTDIR)\nptrivial.lib"
	-@erase "$(OUTDIR)\nptrivial.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /MTd /W3 /Gm /GX /Zi /Od /I "./" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /YX /c
CPP_PROJ=/MTd /W3 /Gm /GX /Zi /Od /I "./" /D "_DEBUG" /D "_WINDOWS" /D "WIN32"\
 /Fp"$(INTDIR)/trivial.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/trivial.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/trivial.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"LIBCMTD" /out:"Debug/nptrivial.dll"
# SUBTRACT LINK32 /verbose
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib wsock32.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/nptrivial.pdb" /debug /machine:I386\
 /nodefaultlib:"LIBCMTD" /out:"$(OUTDIR)/nptrivial.dll"\
 /implib:"$(OUTDIR)/nptrivial.lib" 
LINK32_OBJS= \
	"$(INTDIR)\trivial.obj" \
	"$(INTDIR)\trivial.res" \
	"..\..\..\..\lib\qnp.lib" \
	"..\..\..\..\lib\qt.lib"

"$(OUTDIR)\nptrivial.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "trivial - Win32 Release"
# Name "trivial - Win32 Debug"

!IF  "$(CFG)" == "trivial - Win32 Release"

!ELSEIF  "$(CFG)" == "trivial - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\trivial.cpp
DEP_CPP_GRAPH=\
	".\trivial.moc"\
	{$(INCLUDE)}"\qarray.h"\
	{$(INCLUDE)}"\qbrush.h"\
	{$(INCLUDE)}"\qbuffer.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdialog.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgeneric.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmenubar.h"\
	{$(INCLUDE)}"\qmenudta.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qmsgbox.h"\
	{$(INCLUDE)}"\qnp.h"\
	{$(INCLUDE)}"\qobjdefs.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpen.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpopmenu.h"\
	{$(INCLUDE)}"\qpushbt.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qshared.h"\
	{$(INCLUDE)}"\qsignal.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qtstream.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwindefs.h"\
	{$(INCLUDE)}"\qwmatrix.h"\
	

"$(INTDIR)\trivial.obj" : $(SOURCE) $(DEP_CPP_GRAPH) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=\qt\lib\qt.lib

!IF  "$(CFG)" == "trivial - Win32 Release"

!ELSEIF  "$(CFG)" == "trivial - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\qt\lib\qnp.lib

!IF  "$(CFG)" == "trivial - Win32 Release"

!ELSEIF  "$(CFG)" == "trivial - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\trivial.rc

"$(INTDIR)\trivial.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
