# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=qtools - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to qtools - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "qtools - Win32 Release" && "$(CFG)" != "qtools - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "qtools.mak" CFG="qtools - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qtools - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "qtools - Win32 Debug" (based on "Win32 (x86) Static Library")
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
CPP=cl.exe

!IF  "$(CFG)" == "qtools - Win32 Release"

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

ALL : "$(OUTDIR)\qtools.lib"

CLEAN : 
	-@erase "$(INTDIR)\qbitarry.obj"
	-@erase "$(INTDIR)\qbuffer.obj"
	-@erase "$(INTDIR)\qcollect.obj"
	-@erase "$(INTDIR)\qdatetm.obj"
	-@erase "$(INTDIR)\qdir.obj"
	-@erase "$(INTDIR)\qdstream.obj"
	-@erase "$(INTDIR)\qfile.obj"
	-@erase "$(INTDIR)\qfileinf.obj"
	-@erase "$(INTDIR)\qgarray.obj"
	-@erase "$(INTDIR)\qgcache.obj"
	-@erase "$(INTDIR)\qgdict.obj"
	-@erase "$(INTDIR)\qglist.obj"
	-@erase "$(INTDIR)\qglobal.obj"
	-@erase "$(INTDIR)\qgvector.obj"
	-@erase "$(INTDIR)\qiodev.obj"
	-@erase "$(INTDIR)\qregexp.obj"
	-@erase "$(INTDIR)\qstring.obj"
	-@erase "$(INTDIR)\qtstream.obj"
	-@erase "$(OUTDIR)\qtools.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/qtools.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qtools.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/qtools.lib" 
LIB32_OBJS= \
	"$(INTDIR)\qbitarry.obj" \
	"$(INTDIR)\qbuffer.obj" \
	"$(INTDIR)\qcollect.obj" \
	"$(INTDIR)\qdatetm.obj" \
	"$(INTDIR)\qdir.obj" \
	"$(INTDIR)\qdstream.obj" \
	"$(INTDIR)\qfile.obj" \
	"$(INTDIR)\qfileinf.obj" \
	"$(INTDIR)\qgarray.obj" \
	"$(INTDIR)\qgcache.obj" \
	"$(INTDIR)\qgdict.obj" \
	"$(INTDIR)\qglist.obj" \
	"$(INTDIR)\qglobal.obj" \
	"$(INTDIR)\qgvector.obj" \
	"$(INTDIR)\qiodev.obj" \
	"$(INTDIR)\qregexp.obj" \
	"$(INTDIR)\qstring.obj" \
	"$(INTDIR)\qtstream.obj"

"$(OUTDIR)\qtools.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qtools - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "qtools__"
# PROP BASE Intermediate_Dir "qtools__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "qtools__"
# PROP Intermediate_Dir "qtools__"
# PROP Target_Dir ""
OUTDIR=.\qtools__
INTDIR=.\qtools__

ALL : "$(OUTDIR)\qtools.lib"

CLEAN : 
	-@erase "$(INTDIR)\qbitarry.obj"
	-@erase "$(INTDIR)\qbuffer.obj"
	-@erase "$(INTDIR)\qcollect.obj"
	-@erase "$(INTDIR)\qdatetm.obj"
	-@erase "$(INTDIR)\qdir.obj"
	-@erase "$(INTDIR)\qdstream.obj"
	-@erase "$(INTDIR)\qfile.obj"
	-@erase "$(INTDIR)\qfileinf.obj"
	-@erase "$(INTDIR)\qgarray.obj"
	-@erase "$(INTDIR)\qgcache.obj"
	-@erase "$(INTDIR)\qgdict.obj"
	-@erase "$(INTDIR)\qglist.obj"
	-@erase "$(INTDIR)\qglobal.obj"
	-@erase "$(INTDIR)\qgvector.obj"
	-@erase "$(INTDIR)\qiodev.obj"
	-@erase "$(INTDIR)\qregexp.obj"
	-@erase "$(INTDIR)\qstring.obj"
	-@erase "$(INTDIR)\qtstream.obj"
	-@erase "$(OUTDIR)\qtools.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/qtools.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\qtools__/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qtools.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/qtools.lib" 
LIB32_OBJS= \
	"$(INTDIR)\qbitarry.obj" \
	"$(INTDIR)\qbuffer.obj" \
	"$(INTDIR)\qcollect.obj" \
	"$(INTDIR)\qdatetm.obj" \
	"$(INTDIR)\qdir.obj" \
	"$(INTDIR)\qdstream.obj" \
	"$(INTDIR)\qfile.obj" \
	"$(INTDIR)\qfileinf.obj" \
	"$(INTDIR)\qgarray.obj" \
	"$(INTDIR)\qgcache.obj" \
	"$(INTDIR)\qgdict.obj" \
	"$(INTDIR)\qglist.obj" \
	"$(INTDIR)\qglobal.obj" \
	"$(INTDIR)\qgvector.obj" \
	"$(INTDIR)\qiodev.obj" \
	"$(INTDIR)\qregexp.obj" \
	"$(INTDIR)\qstring.obj" \
	"$(INTDIR)\qtstream.obj"

"$(OUTDIR)\qtools.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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

# Name "qtools - Win32 Release"
# Name "qtools - Win32 Debug"

!IF  "$(CFG)" == "qtools - Win32 Release"

!ELSEIF  "$(CFG)" == "qtools - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\tools\qtstream.cpp

!IF  "$(CFG)" == "qtools - Win32 Release"

NODEP_CPP_QTSTR=\
	".\tools\qbuffer.h"\
	".\tools\qfile.h"\
	".\tools\qtstream.h"\
	

"$(INTDIR)\qtstream.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qtools - Win32 Debug"


"$(INTDIR)\qtstream.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qbuffer.cpp
NODEP_CPP_QBUFF=\
	".\tools\qbuffer.h"\
	

"$(INTDIR)\qbuffer.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qcollect.cpp
NODEP_CPP_QCOLL=\
	".\tools\qcollect.h"\
	

"$(INTDIR)\qcollect.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qdatetm.cpp
NODEP_CPP_QDATE=\
	".\tools\qdatetm.h"\
	".\tools\qdstream.h"\
	

"$(INTDIR)\qdatetm.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qdir.cpp
NODEP_CPP_QDIR_=\
	".\tools\qdir.h"\
	".\tools\qfiledef.h"\
	".\tools\qfileinf.h"\
	".\tools\qregexp.h"\
	

"$(INTDIR)\qdir.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qdstream.cpp
DEP_CPP_QDSTR=\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_QDSTR=\
	".\tools\qbuffer.h"\
	".\tools\qdstream.h"\
	

"$(INTDIR)\qdstream.obj" : $(SOURCE) $(DEP_CPP_QDSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qfile.cpp
NODEP_CPP_QFILE=\
	".\tools\qfile.h"\
	".\tools\qfiledef.h"\
	

"$(INTDIR)\qfile.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qfileinf.cpp
NODEP_CPP_QFILEI=\
	".\tools\qdatetm.h"\
	".\tools\qdir.h"\
	".\tools\qfiledef.h"\
	".\tools\qfileinf.h"\
	".\tools\qglobal.h"\
	

"$(INTDIR)\qfileinf.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qgarray.cpp
DEP_CPP_QGARR=\
	"..\include\qarray.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qshared.h"\
	"..\include\qstring.h"\
	

"$(INTDIR)\qgarray.obj" : $(SOURCE) $(DEP_CPP_QGARR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qgcache.cpp
DEP_CPP_QGCAC=\
	"..\include\qarray.h"\
	"..\include\qcollect.h"\
	"..\include\qdict.h"\
	"..\include\qgarray.h"\
	"..\include\qgcache.h"\
	"..\include\qgdict.h"\
	"..\include\qgeneric.h"\
	"..\include\qglist.h"\
	"..\include\qglobal.h"\
	"..\include\qlist.h"\
	"..\include\qshared.h"\
	"..\include\qstring.h"\
	

"$(INTDIR)\qgcache.obj" : $(SOURCE) $(DEP_CPP_QGCAC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qgdict.cpp
DEP_CPP_QGDIC=\
	"..\include\qarray.h"\
	"..\include\qcollect.h"\
	"..\include\qdstream.h"\
	"..\include\qgarray.h"\
	"..\include\qgdict.h"\
	"..\include\qgeneric.h"\
	"..\include\qglist.h"\
	"..\include\qglobal.h"\
	"..\include\qiodev.h"\
	"..\include\qlist.h"\
	"..\include\qshared.h"\
	"..\include\qstring.h"\
	

"$(INTDIR)\qgdict.obj" : $(SOURCE) $(DEP_CPP_QGDIC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qglist.cpp
DEP_CPP_QGLIS=\
	"..\include\qarray.h"\
	"..\include\qcollect.h"\
	"..\include\qdstream.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglist.h"\
	"..\include\qglobal.h"\
	"..\include\qgvector.h"\
	"..\include\qiodev.h"\
	"..\include\qshared.h"\
	"..\include\qstring.h"\
	

"$(INTDIR)\qglist.obj" : $(SOURCE) $(DEP_CPP_QGLIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qglobal.cpp
DEP_CPP_QGLOB=\
	"..\include\qapp.h"\
	"..\include\qarray.h"\
	"..\include\qcollect.h"\
	"..\include\qcolor.h"\
	"..\include\qcursor.h"\
	"..\include\qdict.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgdict.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\qglobal.obj" : $(SOURCE) $(DEP_CPP_QGLOB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qgvector.cpp
DEP_CPP_QGVEC=\
	"..\include\qarray.h"\
	"..\include\qcollect.h"\
	"..\include\qdstream.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglist.h"\
	"..\include\qglobal.h"\
	"..\include\qgvector.h"\
	"..\include\qiodev.h"\
	"..\include\qshared.h"\
	"..\include\qstring.h"\
	

"$(INTDIR)\qgvector.obj" : $(SOURCE) $(DEP_CPP_QGVEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qiodev.cpp
DEP_CPP_QIODE=\
	"..\include\qglobal.h"\
	"..\include\qiodev.h"\
	

"$(INTDIR)\qiodev.obj" : $(SOURCE) $(DEP_CPP_QIODE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qregexp.cpp
DEP_CPP_QREGE=\
	"..\include\qarray.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qregexp.h"\
	"..\include\qshared.h"\
	"..\include\qstring.h"\
	

"$(INTDIR)\qregexp.obj" : $(SOURCE) $(DEP_CPP_QREGE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qstring.cpp
DEP_CPP_QSTRI=\
	"..\include\qarray.h"\
	"..\include\qdstream.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qiodev.h"\
	"..\include\qshared.h"\
	"..\include\qstring.h"\
	

"$(INTDIR)\qstring.obj" : $(SOURCE) $(DEP_CPP_QSTRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qbitarry.cpp
DEP_CPP_QBITA=\
	"..\include\qarray.h"\
	"..\include\qbitarry.h"\
	"..\include\qdstream.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qiodev.h"\
	"..\include\qshared.h"\
	"..\include\qstring.h"\
	

"$(INTDIR)\qbitarry.obj" : $(SOURCE) $(DEP_CPP_QBITA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
