# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=qtools - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to qtools - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "qtools - Win32 Release" && "$(CFG)" != "qtools - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.	 For example:
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
# PROP Target_Last_Scanned "qtools - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "qtools - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "qtools__"
# PROP BASE Intermediate_Dir "qtools__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "qtools.release"
# PROP Intermediate_Dir "qtools.release"
# PROP Target_Dir ""
OUTDIR=.\qtools.release
INTDIR=.\qtools.release

ALL : "..\lib\qtools.lib"

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
	-@erase "..\lib\qtools.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qtools.pch" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\qtools.release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qtools.bsc"
BSC32_SBRS= \

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\qtools.lib"
LIB32_FLAGS=/nologo /out:"..\lib\qtools.lib"
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

"..\lib\qtools.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF	 "$(CFG)" == "qtools - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "qtools_0"
# PROP BASE Intermediate_Dir "qtools_0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "qtools.debug"
# PROP Intermediate_Dir "qtools.debug"
# PROP Target_Dir ""
OUTDIR=.\qtools.debug
INTDIR=.\qtools.debug

ALL : "..\lib\qtools.lib"

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
	-@erase "..\lib\qtools.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qtools.pch" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\qtools.debug/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qtools.bsc"
BSC32_SBRS= \

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\qtools.lib"
LIB32_FLAGS=/nologo /out:"..\lib\qtools.lib"
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

"..\lib\qtools.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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

!ELSEIF	 "$(CFG)" == "qtools - Win32 Debug"

!ENDIF

################################################################################
# Begin Source File

SOURCE=.\win\qbitarry.cpp
DEP_CPP_QBITA=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qbitarry.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qstring.h"\


"$(INTDIR)\qbitarry.obj" : $(SOURCE) $(DEP_CPP_QBITA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qtstream.cpp
DEP_CPP_QTSTR=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qbuffer.h"\
	{$(INCLUDE)}"\qfile.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtstream.h"\


"$(INTDIR)\qtstream.obj" : $(SOURCE) $(DEP_CPP_QTSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qbuffer.cpp
DEP_CPP_QBUFF=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qbuffer.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qstring.h"\


"$(INTDIR)\qbuffer.obj" : $(SOURCE) $(DEP_CPP_QBUFF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qcollect.cpp
DEP_CPP_QCOLL=\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qglobal.h"\


"$(INTDIR)\qcollect.obj" : $(SOURCE) $(DEP_CPP_QCOLL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qdatetm.cpp
DEP_CPP_QDATE=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qdatetm.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qstring.h"\


"$(INTDIR)\qdatetm.obj" : $(SOURCE) $(DEP_CPP_QDATE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qdir.cpp
DEP_CPP_QDIR_=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	"..\include\qstrlist.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qdatetm.h"\
	{$(INCLUDE)}"\qdir.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qfile.h"\
	{$(INCLUDE)}"\qfiledef.h"\
	{$(INCLUDE)}"\qfileinf.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qregexp.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	{$(INCLUDE)}"\sys\types.h"\


"$(INTDIR)\qdir.obj" : $(SOURCE) $(DEP_CPP_QDIR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qdstream.cpp
DEP_CPP_QDSTR=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qbuffer.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\sys\types.h"\


"$(INTDIR)\qdstream.obj" : $(SOURCE) $(DEP_CPP_QDSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qfile.cpp
DEP_CPP_QFILE=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qfile.h"\
	{$(INCLUDE)}"\qfiledef.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	{$(INCLUDE)}"\sys\types.h"\


"$(INTDIR)\qfile.obj" : $(SOURCE) $(DEP_CPP_QFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qfileinf.cpp
DEP_CPP_QFILEI=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	"..\include\qstrlist.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qdatetm.h"\
	{$(INCLUDE)}"\qdir.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qfile.h"\
	{$(INCLUDE)}"\qfiledef.h"\
	{$(INCLUDE)}"\qfileinf.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	{$(INCLUDE)}"\sys\types.h"\


"$(INTDIR)\qfileinf.obj" : $(SOURCE) $(DEP_CPP_QFILEI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qgarray.cpp
DEP_CPP_QGARR=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qstring.h"\


"$(INTDIR)\qgarray.obj" : $(SOURCE) $(DEP_CPP_QGARR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qgcache.cpp
DEP_CPP_QGCAC=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qdict.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgcache.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qstring.h"\


"$(INTDIR)\qgcache.obj" : $(SOURCE) $(DEP_CPP_QGCAC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qgdict.cpp
DEP_CPP_QGDIC=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qstring.h"\


"$(INTDIR)\qgdict.obj" : $(SOURCE) $(DEP_CPP_QGDIC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qglist.cpp
DEP_CPP_QGLIS=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qgvector.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qstring.h"\


"$(INTDIR)\qglist.obj" : $(SOURCE) $(DEP_CPP_QGLIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qglobal.cpp
DEP_CPP_QGLOB=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qdict.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qstring.h"\


"$(INTDIR)\qglobal.obj" : $(SOURCE) $(DEP_CPP_QGLOB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qgvector.cpp
DEP_CPP_QGVEC=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qgvector.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qstring.h"\


"$(INTDIR)\qgvector.obj" : $(SOURCE) $(DEP_CPP_QGVEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qiodev.cpp
DEP_CPP_QIODE=\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\


"$(INTDIR)\qiodev.obj" : $(SOURCE) $(DEP_CPP_QIODE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qregexp.cpp
DEP_CPP_QREGE=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qregexp.h"\
	{$(INCLUDE)}"\qstring.h"\


"$(INTDIR)\qregexp.obj" : $(SOURCE) $(DEP_CPP_QREGE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qstring.cpp
DEP_CPP_QSTRI=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qstring.h"\


"$(INTDIR)\qstring.obj" : $(SOURCE) $(DEP_CPP_QSTRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
