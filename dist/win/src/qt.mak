# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=qt - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to qt - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "qt - Win32 Release" && "$(CFG)" != "qt - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.	 For example:
!MESSAGE
!MESSAGE NMAKE /f "qt.mak" CFG="qt - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "qt - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "qt - Win32 Debug" (based on "Win32 (x86) Static Library")
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
# PROP Target_Last_Scanned "qt - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "qt - Win32 Release"

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

ALL : "..\lib\qt.lib"

CLEAN :
	-@erase "..\lib\qt.lib"
	-@erase ".\Release\maccel.obj"
	-@erase ".\Release\mapp.obj"
	-@erase ".\Release\mbttngrp.obj"
	-@erase ".\Release\mbutton.obj"
	-@erase ".\Release\mchkbox.obj"
	-@erase ".\Release\mclipbrd.obj"
	-@erase ".\Release\mcombo.obj"
	-@erase ".\Release\mdialog.obj"
	-@erase ".\Release\mfiledlg.obj"
	-@erase ".\Release\mframe.obj"
	-@erase ".\Release\mgrpbox.obj"
	-@erase ".\Release\mlabel.obj"
	-@erase ".\Release\mlcdnum.obj"
	-@erase ".\Release\mlined.obj"
	-@erase ".\Release\mlistbox.obj"
	-@erase ".\Release\mmenubar.obj"
	-@erase ".\Release\mmsgbox.obj"
	-@erase ".\Release\mpopmenu.obj"
	-@erase ".\Release\mpushbt.obj"
	-@erase ".\Release\mradiobt.obj"
	-@erase ".\Release\mscrbar.obj"
	-@erase ".\Release\msocknot.obj"
	-@erase ".\Release\mtablevw.obj"
	-@erase ".\Release\mtimer.obj"
	-@erase ".\Release\mwidget.obj"
	-@erase ".\Release\mwindow.obj"
	-@erase ".\Release\qaccel.obj"
	-@erase ".\Release\qapp.obj"
	-@erase ".\Release\qapp_win.obj"
	-@erase ".\Release\qbitarry.obj"
	-@erase ".\Release\qbitmap.obj"
	-@erase ".\Release\qbttngrp.obj"
	-@erase ".\Release\qbuffer.obj"
	-@erase ".\Release\qbutton.obj"
	-@erase ".\Release\qchkbox.obj"
	-@erase ".\Release\qclb_win.obj"
	-@erase ".\Release\qclipbrd.obj"
	-@erase ".\Release\qcol_win.obj"
	-@erase ".\Release\qcollect.obj"
	-@erase ".\Release\qcolor.obj"
	-@erase ".\Release\qcombo.obj"
	-@erase ".\Release\qconnect.obj"
	-@erase ".\Release\qcur_win.obj"
	-@erase ".\Release\qcursor.obj"
	-@erase ".\Release\qdatetm.obj"
	-@erase ".\Release\qdialog.obj"
	-@erase ".\Release\qdir.obj"
	-@erase ".\Release\qdrawutl.obj"
	-@erase ".\Release\qdstream.obj"
	-@erase ".\Release\qevent.obj"
	-@erase ".\Release\qfile.obj"
	-@erase ".\Release\qfiledlg.obj"
	-@erase ".\Release\qfileinf.obj"
	-@erase ".\Release\qfnt_win.obj"
	-@erase ".\Release\qfont.obj"
	-@erase ".\Release\qframe.obj"
	-@erase ".\Release\qgarray.obj"
	-@erase ".\Release\qgcache.obj"
	-@erase ".\Release\qgdict.obj"
	-@erase ".\Release\qglist.obj"
	-@erase ".\Release\qglobal.obj"
	-@erase ".\Release\qgrpbox.obj"
	-@erase ".\Release\qgvector.obj"
	-@erase ".\Release\qimage.obj"
	-@erase ".\Release\qiodev.obj"
	-@erase ".\Release\qlabel.obj"
	-@erase ".\Release\qlcdnum.obj"
	-@erase ".\Release\qlined.obj"
	-@erase ".\Release\qlistbox.obj"
	-@erase ".\Release\qmenubar.obj"
	-@erase ".\Release\qmenudta.obj"
	-@erase ".\Release\qmetaobj.obj"
	-@erase ".\Release\qmsgbox.obj"
	-@erase ".\Release\qobject.obj"
	-@erase ".\Release\qpainter.obj"
	-@erase ".\Release\qpalette.obj"
	-@erase ".\Release\qpdevmet.obj"
	-@erase ".\Release\qpic_win.obj"
	-@erase ".\Release\qpicture.obj"
	-@erase ".\Release\qpixmap.obj"
	-@erase ".\Release\qpm_win.obj"
	-@erase ".\Release\qpmcache.obj"
	-@erase ".\Release\qpntarry.obj"
	-@erase ".\Release\qpoint.obj"
	-@erase ".\Release\qpopmenu.obj"
	-@erase ".\Release\qprinter.obj"
	-@erase ".\Release\qprn_win.obj"
	-@erase ".\Release\qptd_win.obj"
	-@erase ".\Release\qptr_win.obj"
	-@erase ".\Release\qpushbt.obj"
	-@erase ".\Release\qradiobt.obj"
	-@erase ".\Release\qrangect.obj"
	-@erase ".\Release\qrect.obj"
	-@erase ".\Release\qregexp.obj"
	-@erase ".\Release\qregion.obj"
	-@erase ".\Release\qrgn_win.obj"
	-@erase ".\Release\qscrbar.obj"
	-@erase ".\Release\qsignal.obj"
	-@erase ".\Release\qsize.obj"
	-@erase ".\Release\qsocknot.obj"
	-@erase ".\Release\qstring.obj"
	-@erase ".\Release\qtablevw.obj"
	-@erase ".\Release\qtimer.obj"
	-@erase ".\Release\qtstream.obj"
	-@erase ".\Release\qwid_win.obj"
	-@erase ".\Release\qwidget.obj"
	-@erase ".\Release\qwindow.obj"
	-@erase ".\Release\qwmatrix.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qt.pch" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\Release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qt.bsc"
BSC32_SBRS= \

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/qt.lib"
LIB32_FLAGS=/nologo /out:"../lib/qt.lib"
LIB32_OBJS= \
	".\Release\maccel.obj" \
	".\Release\mapp.obj" \
	".\Release\mbttngrp.obj" \
	".\Release\mbutton.obj" \
	".\Release\mchkbox.obj" \
	".\Release\mclipbrd.obj" \
	".\Release\mcombo.obj" \
	".\Release\mdialog.obj" \
	".\Release\mfiledlg.obj" \
	".\Release\mframe.obj" \
	".\Release\mgrpbox.obj" \
	".\Release\mlabel.obj" \
	".\Release\mlcdnum.obj" \
	".\Release\mlined.obj" \
	".\Release\mlistbox.obj" \
	".\Release\mmenubar.obj" \
	".\Release\mmsgbox.obj" \
	".\Release\mpopmenu.obj" \
	".\Release\mpushbt.obj" \
	".\Release\mradiobt.obj" \
	".\Release\mscrbar.obj" \
	".\Release\msocknot.obj" \
	".\Release\mtablevw.obj" \
	".\Release\mtimer.obj" \
	".\Release\mwidget.obj" \
	".\Release\mwindow.obj" \
	".\Release\qaccel.obj" \
	".\Release\qapp.obj" \
	".\Release\qapp_win.obj" \
	".\Release\qbitarry.obj" \
	".\Release\qbitmap.obj" \
	".\Release\qbttngrp.obj" \
	".\Release\qbuffer.obj" \
	".\Release\qbutton.obj" \
	".\Release\qchkbox.obj" \
	".\Release\qclb_win.obj" \
	".\Release\qclipbrd.obj" \
	".\Release\qcol_win.obj" \
	".\Release\qcollect.obj" \
	".\Release\qcolor.obj" \
	".\Release\qcombo.obj" \
	".\Release\qconnect.obj" \
	".\Release\qcur_win.obj" \
	".\Release\qcursor.obj" \
	".\Release\qdatetm.obj" \
	".\Release\qdialog.obj" \
	".\Release\qdir.obj" \
	".\Release\qdrawutl.obj" \
	".\Release\qdstream.obj" \
	".\Release\qevent.obj" \
	".\Release\qfile.obj" \
	".\Release\qfiledlg.obj" \
	".\Release\qfileinf.obj" \
	".\Release\qfnt_win.obj" \
	".\Release\qfont.obj" \
	".\Release\qframe.obj" \
	".\Release\qgarray.obj" \
	".\Release\qgcache.obj" \
	".\Release\qgdict.obj" \
	".\Release\qglist.obj" \
	".\Release\qglobal.obj" \
	".\Release\qgrpbox.obj" \
	".\Release\qgvector.obj" \
	".\Release\qimage.obj" \
	".\Release\qiodev.obj" \
	".\Release\qlabel.obj" \
	".\Release\qlcdnum.obj" \
	".\Release\qlined.obj" \
	".\Release\qlistbox.obj" \
	".\Release\qmenubar.obj" \
	".\Release\qmenudta.obj" \
	".\Release\qmetaobj.obj" \
	".\Release\qmsgbox.obj" \
	".\Release\qobject.obj" \
	".\Release\qpainter.obj" \
	".\Release\qpalette.obj" \
	".\Release\qpdevmet.obj" \
	".\Release\qpic_win.obj" \
	".\Release\qpicture.obj" \
	".\Release\qpixmap.obj" \
	".\Release\qpm_win.obj" \
	".\Release\qpmcache.obj" \
	".\Release\qpntarry.obj" \
	".\Release\qpoint.obj" \
	".\Release\qpopmenu.obj" \
	".\Release\qprinter.obj" \
	".\Release\qprn_win.obj" \
	".\Release\qptd_win.obj" \
	".\Release\qptr_win.obj" \
	".\Release\qpushbt.obj" \
	".\Release\qradiobt.obj" \
	".\Release\qrangect.obj" \
	".\Release\qrect.obj" \
	".\Release\qregexp.obj" \
	".\Release\qregion.obj" \
	".\Release\qrgn_win.obj" \
	".\Release\qscrbar.obj" \
	".\Release\qsignal.obj" \
	".\Release\qsize.obj" \
	".\Release\qsocknot.obj" \
	".\Release\qstring.obj" \
	".\Release\qtablevw.obj" \
	".\Release\qtimer.obj" \
	".\Release\qtstream.obj" \
	".\Release\qwid_win.obj" \
	".\Release\qwidget.obj" \
	".\Release\qwindow.obj" \
	".\Release\qwmatrix.obj"

"..\lib\qt.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"

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

ALL : "..\lib\qt.lib"

CLEAN :
	-@erase "..\lib\qt.lib"
	-@erase ".\Debug\maccel.obj"
	-@erase ".\Debug\mapp.obj"
	-@erase ".\Debug\mbttngrp.obj"
	-@erase ".\Debug\mbutton.obj"
	-@erase ".\Debug\mchkbox.obj"
	-@erase ".\Debug\mclipbrd.obj"
	-@erase ".\Debug\mcombo.obj"
	-@erase ".\Debug\mdialog.obj"
	-@erase ".\Debug\mfiledlg.obj"
	-@erase ".\Debug\mframe.obj"
	-@erase ".\Debug\mgrpbox.obj"
	-@erase ".\Debug\mlabel.obj"
	-@erase ".\Debug\mlcdnum.obj"
	-@erase ".\Debug\mlined.obj"
	-@erase ".\Debug\mlistbox.obj"
	-@erase ".\Debug\mmenubar.obj"
	-@erase ".\Debug\mmsgbox.obj"
	-@erase ".\Debug\mpopmenu.obj"
	-@erase ".\Debug\mpushbt.obj"
	-@erase ".\Debug\mradiobt.obj"
	-@erase ".\Debug\mscrbar.obj"
	-@erase ".\Debug\msocknot.obj"
	-@erase ".\Debug\mtablevw.obj"
	-@erase ".\Debug\mtimer.obj"
	-@erase ".\Debug\mwidget.obj"
	-@erase ".\Debug\mwindow.obj"
	-@erase ".\Debug\qaccel.obj"
	-@erase ".\Debug\qapp.obj"
	-@erase ".\Debug\qapp_win.obj"
	-@erase ".\Debug\qbitarry.obj"
	-@erase ".\Debug\qbitmap.obj"
	-@erase ".\Debug\qbttngrp.obj"
	-@erase ".\Debug\qbuffer.obj"
	-@erase ".\Debug\qbutton.obj"
	-@erase ".\Debug\qchkbox.obj"
	-@erase ".\Debug\qclb_win.obj"
	-@erase ".\Debug\qclipbrd.obj"
	-@erase ".\Debug\qcol_win.obj"
	-@erase ".\Debug\qcollect.obj"
	-@erase ".\Debug\qcolor.obj"
	-@erase ".\Debug\qcombo.obj"
	-@erase ".\Debug\qconnect.obj"
	-@erase ".\Debug\qcur_win.obj"
	-@erase ".\Debug\qcursor.obj"
	-@erase ".\Debug\qdatetm.obj"
	-@erase ".\Debug\qdialog.obj"
	-@erase ".\Debug\qdir.obj"
	-@erase ".\Debug\qdrawutl.obj"
	-@erase ".\Debug\qdstream.obj"
	-@erase ".\Debug\qevent.obj"
	-@erase ".\Debug\qfile.obj"
	-@erase ".\Debug\qfiledlg.obj"
	-@erase ".\Debug\qfileinf.obj"
	-@erase ".\Debug\qfnt_win.obj"
	-@erase ".\Debug\qfont.obj"
	-@erase ".\Debug\qframe.obj"
	-@erase ".\Debug\qgarray.obj"
	-@erase ".\Debug\qgcache.obj"
	-@erase ".\Debug\qgdict.obj"
	-@erase ".\Debug\qglist.obj"
	-@erase ".\Debug\qglobal.obj"
	-@erase ".\Debug\qgrpbox.obj"
	-@erase ".\Debug\qgvector.obj"
	-@erase ".\Debug\qimage.obj"
	-@erase ".\Debug\qiodev.obj"
	-@erase ".\Debug\qlabel.obj"
	-@erase ".\Debug\qlcdnum.obj"
	-@erase ".\Debug\qlined.obj"
	-@erase ".\Debug\qlistbox.obj"
	-@erase ".\Debug\qmenubar.obj"
	-@erase ".\Debug\qmenudta.obj"
	-@erase ".\Debug\qmetaobj.obj"
	-@erase ".\Debug\qmsgbox.obj"
	-@erase ".\Debug\qobject.obj"
	-@erase ".\Debug\qpainter.obj"
	-@erase ".\Debug\qpalette.obj"
	-@erase ".\Debug\qpdevmet.obj"
	-@erase ".\Debug\qpic_win.obj"
	-@erase ".\Debug\qpicture.obj"
	-@erase ".\Debug\qpixmap.obj"
	-@erase ".\Debug\qpm_win.obj"
	-@erase ".\Debug\qpmcache.obj"
	-@erase ".\Debug\qpntarry.obj"
	-@erase ".\Debug\qpoint.obj"
	-@erase ".\Debug\qpopmenu.obj"
	-@erase ".\Debug\qprinter.obj"
	-@erase ".\Debug\qprn_win.obj"
	-@erase ".\Debug\qptd_win.obj"
	-@erase ".\Debug\qptr_win.obj"
	-@erase ".\Debug\qpushbt.obj"
	-@erase ".\Debug\qradiobt.obj"
	-@erase ".\Debug\qrangect.obj"
	-@erase ".\Debug\qrect.obj"
	-@erase ".\Debug\qregexp.obj"
	-@erase ".\Debug\qregion.obj"
	-@erase ".\Debug\qrgn_win.obj"
	-@erase ".\Debug\qscrbar.obj"
	-@erase ".\Debug\qsignal.obj"
	-@erase ".\Debug\qsize.obj"
	-@erase ".\Debug\qsocknot.obj"
	-@erase ".\Debug\qstring.obj"
	-@erase ".\Debug\qtablevw.obj"
	-@erase ".\Debug\qtimer.obj"
	-@erase ".\Debug\qtstream.obj"
	-@erase ".\Debug\qwid_win.obj"
	-@erase ".\Debug\qwidget.obj"
	-@erase ".\Debug\qwindow.obj"
	-@erase ".\Debug\qwmatrix.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qt.pch" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qt.bsc"
BSC32_SBRS= \

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/qt.lib"
LIB32_FLAGS=/nologo /out:"../lib/qt.lib"
LIB32_OBJS= \
	".\Debug\maccel.obj" \
	".\Debug\mapp.obj" \
	".\Debug\mbttngrp.obj" \
	".\Debug\mbutton.obj" \
	".\Debug\mchkbox.obj" \
	".\Debug\mclipbrd.obj" \
	".\Debug\mcombo.obj" \
	".\Debug\mdialog.obj" \
	".\Debug\mfiledlg.obj" \
	".\Debug\mframe.obj" \
	".\Debug\mgrpbox.obj" \
	".\Debug\mlabel.obj" \
	".\Debug\mlcdnum.obj" \
	".\Debug\mlined.obj" \
	".\Debug\mlistbox.obj" \
	".\Debug\mmenubar.obj" \
	".\Debug\mmsgbox.obj" \
	".\Debug\mpopmenu.obj" \
	".\Debug\mpushbt.obj" \
	".\Debug\mradiobt.obj" \
	".\Debug\mscrbar.obj" \
	".\Debug\msocknot.obj" \
	".\Debug\mtablevw.obj" \
	".\Debug\mtimer.obj" \
	".\Debug\mwidget.obj" \
	".\Debug\mwindow.obj" \
	".\Debug\qaccel.obj" \
	".\Debug\qapp.obj" \
	".\Debug\qapp_win.obj" \
	".\Debug\qbitarry.obj" \
	".\Debug\qbitmap.obj" \
	".\Debug\qbttngrp.obj" \
	".\Debug\qbuffer.obj" \
	".\Debug\qbutton.obj" \
	".\Debug\qchkbox.obj" \
	".\Debug\qclb_win.obj" \
	".\Debug\qclipbrd.obj" \
	".\Debug\qcol_win.obj" \
	".\Debug\qcollect.obj" \
	".\Debug\qcolor.obj" \
	".\Debug\qcombo.obj" \
	".\Debug\qconnect.obj" \
	".\Debug\qcur_win.obj" \
	".\Debug\qcursor.obj" \
	".\Debug\qdatetm.obj" \
	".\Debug\qdialog.obj" \
	".\Debug\qdir.obj" \
	".\Debug\qdrawutl.obj" \
	".\Debug\qdstream.obj" \
	".\Debug\qevent.obj" \
	".\Debug\qfile.obj" \
	".\Debug\qfiledlg.obj" \
	".\Debug\qfileinf.obj" \
	".\Debug\qfnt_win.obj" \
	".\Debug\qfont.obj" \
	".\Debug\qframe.obj" \
	".\Debug\qgarray.obj" \
	".\Debug\qgcache.obj" \
	".\Debug\qgdict.obj" \
	".\Debug\qglist.obj" \
	".\Debug\qglobal.obj" \
	".\Debug\qgrpbox.obj" \
	".\Debug\qgvector.obj" \
	".\Debug\qimage.obj" \
	".\Debug\qiodev.obj" \
	".\Debug\qlabel.obj" \
	".\Debug\qlcdnum.obj" \
	".\Debug\qlined.obj" \
	".\Debug\qlistbox.obj" \
	".\Debug\qmenubar.obj" \
	".\Debug\qmenudta.obj" \
	".\Debug\qmetaobj.obj" \
	".\Debug\qmsgbox.obj" \
	".\Debug\qobject.obj" \
	".\Debug\qpainter.obj" \
	".\Debug\qpalette.obj" \
	".\Debug\qpdevmet.obj" \
	".\Debug\qpic_win.obj" \
	".\Debug\qpicture.obj" \
	".\Debug\qpixmap.obj" \
	".\Debug\qpm_win.obj" \
	".\Debug\qpmcache.obj" \
	".\Debug\qpntarry.obj" \
	".\Debug\qpoint.obj" \
	".\Debug\qpopmenu.obj" \
	".\Debug\qprinter.obj" \
	".\Debug\qprn_win.obj" \
	".\Debug\qptd_win.obj" \
	".\Debug\qptr_win.obj" \
	".\Debug\qpushbt.obj" \
	".\Debug\qradiobt.obj" \
	".\Debug\qrangect.obj" \
	".\Debug\qrect.obj" \
	".\Debug\qregexp.obj" \
	".\Debug\qregion.obj" \
	".\Debug\qrgn_win.obj" \
	".\Debug\qscrbar.obj" \
	".\Debug\qsignal.obj" \
	".\Debug\qsize.obj" \
	".\Debug\qsocknot.obj" \
	".\Debug\qstring.obj" \
	".\Debug\qtablevw.obj" \
	".\Debug\qtimer.obj" \
	".\Debug\qtstream.obj" \
	".\Debug\qwid_win.obj" \
	".\Debug\qwidget.obj" \
	".\Debug\qwindow.obj" \
	".\Debug\qwmatrix.obj"

"..\lib\qt.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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

# Name "qt - Win32 Release"
# Name "qt - Win32 Debug"

!IF  "$(CFG)" == "qt - Win32 Release"

!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"

!ENDIF

################################################################################
# Begin Source File

SOURCE=.\win\qwmatrix.cpp
DEP_CPP_QWMAT=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qwmatrix.obj" : $(SOURCE) $(DEP_CPP_QWMAT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qwmatrix.obj" : $(SOURCE) $(DEP_CPP_QWMAT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mapp.cpp
DEP_CPP_MAPP_=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mapp.obj" : $(SOURCE) $(DEP_CPP_MAPP_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mapp.obj" : $(SOURCE) $(DEP_CPP_MAPP_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mbttngrp.cpp
DEP_CPP_MBTTN=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbttngrp.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qgrpbox.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mbttngrp.obj" : $(SOURCE) $(DEP_CPP_MBTTN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mbttngrp.obj" : $(SOURCE) $(DEP_CPP_MBTTN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mbutton.cpp
DEP_CPP_MBUTT=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mbutton.obj" : $(SOURCE) $(DEP_CPP_MBUTT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mbutton.obj" : $(SOURCE) $(DEP_CPP_MBUTT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mchkbox.cpp
DEP_CPP_MCHKB=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qchkbox.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mchkbox.obj" : $(SOURCE) $(DEP_CPP_MCHKB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mchkbox.obj" : $(SOURCE) $(DEP_CPP_MCHKB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mclipbrd.cpp
DEP_CPP_MCLIP=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qclipbrd.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mclipbrd.obj" : $(SOURCE) $(DEP_CPP_MCLIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mclipbrd.obj" : $(SOURCE) $(DEP_CPP_MCLIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mcombo.cpp
DEP_CPP_MCOMB=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcombo.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mcombo.obj" : $(SOURCE) $(DEP_CPP_MCOMB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mcombo.obj" : $(SOURCE) $(DEP_CPP_MCOMB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mdialog.cpp
DEP_CPP_MDIAL=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdialog.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwindow.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mdialog.obj" : $(SOURCE) $(DEP_CPP_MDIAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mdialog.obj" : $(SOURCE) $(DEP_CPP_MDIAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mfiledlg.cpp
DEP_CPP_MFILE=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdatetm.h"\
	{$(INCLUDE)}"\qdialog.h"\
	{$(INCLUDE)}"\qdir.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfile.h"\
	{$(INCLUDE)}"\qfiledlg.h"\
	{$(INCLUDE)}"\qfileinf.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qstrlist.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwindow.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mfiledlg.obj" : $(SOURCE) $(DEP_CPP_MFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mfiledlg.obj" : $(SOURCE) $(DEP_CPP_MFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mframe.cpp
DEP_CPP_MFRAM=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mframe.obj" : $(SOURCE) $(DEP_CPP_MFRAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mframe.obj" : $(SOURCE) $(DEP_CPP_MFRAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mgrpbox.cpp
DEP_CPP_MGRPB=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qgrpbox.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mgrpbox.obj" : $(SOURCE) $(DEP_CPP_MGRPB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mgrpbox.obj" : $(SOURCE) $(DEP_CPP_MGRPB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mlabel.cpp
DEP_CPP_MLABE=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlabel.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mlabel.obj" : $(SOURCE) $(DEP_CPP_MLABE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mlabel.obj" : $(SOURCE) $(DEP_CPP_MLABE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mlcdnum.cpp
DEP_CPP_MLCDN=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbitarry.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlcdnum.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mlcdnum.obj" : $(SOURCE) $(DEP_CPP_MLCDN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mlcdnum.obj" : $(SOURCE) $(DEP_CPP_MLCDN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mlined.cpp
DEP_CPP_MLINE=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlined.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mlined.obj" : $(SOURCE) $(DEP_CPP_MLINE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mlined.obj" : $(SOURCE) $(DEP_CPP_MLINE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mlistbox.cpp
DEP_CPP_MLIST=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlistbox.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mlistbox.obj" : $(SOURCE) $(DEP_CPP_MLIST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mlistbox.obj" : $(SOURCE) $(DEP_CPP_MLIST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mmenubar.cpp
DEP_CPP_MMENU=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmenubar.h"\
	{$(INCLUDE)}"\qmenudta.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpopmenu.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsignal.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mmenubar.obj" : $(SOURCE) $(DEP_CPP_MMENU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mmenubar.obj" : $(SOURCE) $(DEP_CPP_MMENU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mmsgbox.cpp
DEP_CPP_MMSGB=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdialog.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qmsgbox.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwindow.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mmsgbox.obj" : $(SOURCE) $(DEP_CPP_MMSGB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mmsgbox.obj" : $(SOURCE) $(DEP_CPP_MMSGB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mpopmenu.cpp
DEP_CPP_MPOPM=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmenudta.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpopmenu.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsignal.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mpopmenu.obj" : $(SOURCE) $(DEP_CPP_MPOPM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mpopmenu.obj" : $(SOURCE) $(DEP_CPP_MPOPM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mpushbt.cpp
DEP_CPP_MPUSH=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpushbt.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mpushbt.obj" : $(SOURCE) $(DEP_CPP_MPUSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mpushbt.obj" : $(SOURCE) $(DEP_CPP_MPUSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mradiobt.cpp
DEP_CPP_MRADI=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qradiobt.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mradiobt.obj" : $(SOURCE) $(DEP_CPP_MRADI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mradiobt.obj" : $(SOURCE) $(DEP_CPP_MRADI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mscrbar.cpp
DEP_CPP_MSCRB=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrangect.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qscrbar.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mscrbar.obj" : $(SOURCE) $(DEP_CPP_MSCRB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mscrbar.obj" : $(SOURCE) $(DEP_CPP_MSCRB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\msocknot.cpp
DEP_CPP_MSOCK=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qsocknot.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\msocknot.obj" : $(SOURCE) $(DEP_CPP_MSOCK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\msocknot.obj" : $(SOURCE) $(DEP_CPP_MSOCK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mtablevw.cpp
DEP_CPP_MTABL=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mtablevw.obj" : $(SOURCE) $(DEP_CPP_MTABL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mtablevw.obj" : $(SOURCE) $(DEP_CPP_MTABL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mtimer.cpp
DEP_CPP_MTIME=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtimer.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mtimer.obj" : $(SOURCE) $(DEP_CPP_MTIME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mtimer.obj" : $(SOURCE) $(DEP_CPP_MTIME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mwidget.cpp
DEP_CPP_MWIDG=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mwidget.obj" : $(SOURCE) $(DEP_CPP_MWIDG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mwidget.obj" : $(SOURCE) $(DEP_CPP_MWIDG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\mwindow.cpp
DEP_CPP_MWIND=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwindow.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\mwindow.obj" : $(SOURCE) $(DEP_CPP_MWIND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\mwindow.obj" : $(SOURCE) $(DEP_CPP_MWIND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qaccel.cpp
DEP_CPP_QACCE=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qaccel.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qkeycode.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsignal.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qaccel.obj" : $(SOURCE) $(DEP_CPP_QACCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qaccel.obj" : $(SOURCE) $(DEP_CPP_QACCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qapp.cpp
DEP_CPP_QAPP_=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdict.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qintdict.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobjcoll.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidcoll.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qapp.obj" : $(SOURCE) $(DEP_CPP_QAPP_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qapp.obj" : $(SOURCE) $(DEP_CPP_QAPP_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qapp_win.cpp
DEP_CPP_QAPP_W=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qgvector.h"\
	{$(INCLUDE)}"\qintdict.h"\
	{$(INCLUDE)}"\qkeycode.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpmcache.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qqueue.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qvector.h"\
	{$(INCLUDE)}"\qwidcoll.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qapp_win.obj" : $(SOURCE) $(DEP_CPP_QAPP_W) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qapp_win.obj" : $(SOURCE) $(DEP_CPP_QAPP_W) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qbitarry.obj" : $(SOURCE) $(DEP_CPP_QBITA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qbitarry.obj" : $(SOURCE) $(DEP_CPP_QBITA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qbitmap.cpp
DEP_CPP_QBITM=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbitmap.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qimage.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qbitmap.obj" : $(SOURCE) $(DEP_CPP_QBITM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qbitmap.obj" : $(SOURCE) $(DEP_CPP_QBITM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qbttngrp.cpp
DEP_CPP_QBTTN=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbttngrp.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qgrpbox.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qbttngrp.obj" : $(SOURCE) $(DEP_CPP_QBTTN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qbttngrp.obj" : $(SOURCE) $(DEP_CPP_QBTTN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qbuffer.obj" : $(SOURCE) $(DEP_CPP_QBUFF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qbuffer.obj" : $(SOURCE) $(DEP_CPP_QBUFF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qbutton.cpp
DEP_CPP_QBUTT=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbttngrp.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qgrpbox.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qbutton.obj" : $(SOURCE) $(DEP_CPP_QBUTT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qbutton.obj" : $(SOURCE) $(DEP_CPP_QBUTT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qchkbox.cpp
DEP_CPP_QCHKB=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qchkbox.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdrawutl.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpmcache.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qchkbox.obj" : $(SOURCE) $(DEP_CPP_QCHKB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qchkbox.obj" : $(SOURCE) $(DEP_CPP_QCHKB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qclb_win.cpp
DEP_CPP_QCLB_=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qclipbrd.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdatetm.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qclb_win.obj" : $(SOURCE) $(DEP_CPP_QCLB_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qclb_win.obj" : $(SOURCE) $(DEP_CPP_QCLB_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qclipbrd.cpp
DEP_CPP_QCLIP=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qclipbrd.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qclipbrd.obj" : $(SOURCE) $(DEP_CPP_QCLIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qclipbrd.obj" : $(SOURCE) $(DEP_CPP_QCLIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qcol_win.cpp
DEP_CPP_QCOL_=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qcol_win.obj" : $(SOURCE) $(DEP_CPP_QCOL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qcol_win.obj" : $(SOURCE) $(DEP_CPP_QCOL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qcollect.cpp
DEP_CPP_QCOLL=\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qglobal.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qcollect.obj" : $(SOURCE) $(DEP_CPP_QCOLL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qcollect.obj" : $(SOURCE) $(DEP_CPP_QCOLL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qcolor.cpp
DEP_CPP_QCOLO=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qcolor.obj" : $(SOURCE) $(DEP_CPP_QCOLO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qcolor.obj" : $(SOURCE) $(DEP_CPP_QCOLO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qcombo.cpp
DEP_CPP_QCOMB=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcombo.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdrawutl.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qkeycode.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmenudta.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpopmenu.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsignal.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qstrlist.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qcombo.obj" : $(SOURCE) $(DEP_CPP_QCOMB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qcombo.obj" : $(SOURCE) $(DEP_CPP_QCOMB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qconnect.cpp
DEP_CPP_QCONN=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qconnect.obj" : $(SOURCE) $(DEP_CPP_QCONN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qconnect.obj" : $(SOURCE) $(DEP_CPP_QCONN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qcur_win.cpp
DEP_CPP_QCUR_=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qbitmap.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qimage.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qcur_win.obj" : $(SOURCE) $(DEP_CPP_QCUR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qcur_win.obj" : $(SOURCE) $(DEP_CPP_QCUR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qcursor.cpp
DEP_CPP_QCURS=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbitmap.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qcursor.obj" : $(SOURCE) $(DEP_CPP_QCURS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qcursor.obj" : $(SOURCE) $(DEP_CPP_QCURS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qdatetm.obj" : $(SOURCE) $(DEP_CPP_QDATE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qdatetm.obj" : $(SOURCE) $(DEP_CPP_QDATE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qdialog.cpp
DEP_CPP_QDIAL=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdialog.h"\
	{$(INCLUDE)}"\qdict.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qkeycode.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobjcoll.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpushbt.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwindow.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qdialog.obj" : $(SOURCE) $(DEP_CPP_QDIAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qdialog.obj" : $(SOURCE) $(DEP_CPP_QDIAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qdir.cpp
DEP_CPP_QDIR_=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
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
	{$(INCLUDE)}"\qstrlist.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	{$(INCLUDE)}"\sys\types.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qdir.obj" : $(SOURCE) $(DEP_CPP_QDIR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qdir.obj" : $(SOURCE) $(DEP_CPP_QDIR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qdrawutl.cpp
DEP_CPP_QDRAW=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qdrawutl.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qdrawutl.obj" : $(SOURCE) $(DEP_CPP_QDRAW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qdrawutl.obj" : $(SOURCE) $(DEP_CPP_QDRAW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qdstream.obj" : $(SOURCE) $(DEP_CPP_QDSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qdstream.obj" : $(SOURCE) $(DEP_CPP_QDSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qevent.cpp
DEP_CPP_QEVEN=\
	"..\include\qobjdefs.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qevent.obj" : $(SOURCE) $(DEP_CPP_QEVEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qevent.obj" : $(SOURCE) $(DEP_CPP_QEVEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qfile.obj" : $(SOURCE) $(DEP_CPP_QFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qfile.obj" : $(SOURCE) $(DEP_CPP_QFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qfiledlg.cpp
DEP_CPP_QFILED=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcombo.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdatetm.h"\
	{$(INCLUDE)}"\qdialog.h"\
	{$(INCLUDE)}"\qdir.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfile.h"\
	{$(INCLUDE)}"\qfiledlg.h"\
	{$(INCLUDE)}"\qfileinf.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qlabel.h"\
	{$(INCLUDE)}"\qlined.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qlistbox.h"\
	{$(INCLUDE)}"\qmsgbox.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpushbt.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qstrlist.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwindow.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qfiledlg.obj" : $(SOURCE) $(DEP_CPP_QFILED) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qfiledlg.obj" : $(SOURCE) $(DEP_CPP_QFILED) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qfileinf.cpp
DEP_CPP_QFILEI=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qshared.h"\
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
	{$(INCLUDE)}"\qstrlist.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	{$(INCLUDE)}"\sys\types.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qfileinf.obj" : $(SOURCE) $(DEP_CPP_QFILEI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qfileinf.obj" : $(SOURCE) $(DEP_CPP_QFILEI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qfnt_win.cpp
DEP_CPP_QFNT_=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontdta.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qfnt_win.obj" : $(SOURCE) $(DEP_CPP_QFNT_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qfnt_win.obj" : $(SOURCE) $(DEP_CPP_QFNT_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qfont.cpp
DEP_CPP_QFONT=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdict.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontdta.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qstrlist.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qfont.obj" : $(SOURCE) $(DEP_CPP_QFONT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qfont.obj" : $(SOURCE) $(DEP_CPP_QFONT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qframe.cpp
DEP_CPP_QFRAM=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdrawutl.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qframe.obj" : $(SOURCE) $(DEP_CPP_QFRAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qframe.obj" : $(SOURCE) $(DEP_CPP_QFRAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qgarray.obj" : $(SOURCE) $(DEP_CPP_QGARR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qgarray.obj" : $(SOURCE) $(DEP_CPP_QGARR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qgcache.obj" : $(SOURCE) $(DEP_CPP_QGCAC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qgcache.obj" : $(SOURCE) $(DEP_CPP_QGCAC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qgdict.obj" : $(SOURCE) $(DEP_CPP_QGDIC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qgdict.obj" : $(SOURCE) $(DEP_CPP_QGDIC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qglist.obj" : $(SOURCE) $(DEP_CPP_QGLIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qglist.obj" : $(SOURCE) $(DEP_CPP_QGLIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qglobal.obj" : $(SOURCE) $(DEP_CPP_QGLOB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qglobal.obj" : $(SOURCE) $(DEP_CPP_QGLOB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qgrpbox.cpp
DEP_CPP_QGRPB=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qgrpbox.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qgrpbox.obj" : $(SOURCE) $(DEP_CPP_QGRPB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qgrpbox.obj" : $(SOURCE) $(DEP_CPP_QGRPB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qgvector.obj" : $(SOURCE) $(DEP_CPP_QGVEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qgvector.obj" : $(SOURCE) $(DEP_CPP_QGVEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qimage.cpp
DEP_CPP_QIMAG=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qfile.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qimage.h"\
	{$(INCLUDE)}"\qintdict.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregexp.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qimage.obj" : $(SOURCE) $(DEP_CPP_QIMAG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qimage.obj" : $(SOURCE) $(DEP_CPP_QIMAG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qiodev.cpp
DEP_CPP_QIODE=\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qiodev.obj" : $(SOURCE) $(DEP_CPP_QIODE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qiodev.obj" : $(SOURCE) $(DEP_CPP_QIODE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qlabel.cpp
DEP_CPP_QLABE=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlabel.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qlabel.obj" : $(SOURCE) $(DEP_CPP_QLABE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qlabel.obj" : $(SOURCE) $(DEP_CPP_QLABE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qlcdnum.cpp
DEP_CPP_QLCDN=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbitarry.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlcdnum.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qlcdnum.obj" : $(SOURCE) $(DEP_CPP_QLCDN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qlcdnum.obj" : $(SOURCE) $(DEP_CPP_QLCDN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qlined.cpp
DEP_CPP_QLINE=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qclipbrd.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdrawutl.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qkeycode.h"\
	{$(INCLUDE)}"\qlined.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qlined.obj" : $(SOURCE) $(DEP_CPP_QLINE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qlined.obj" : $(SOURCE) $(DEP_CPP_QLINE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qlistbox.cpp
DEP_CPP_QLIST=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qkeycode.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qlistbox.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qstrlist.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qlistbox.obj" : $(SOURCE) $(DEP_CPP_QLIST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qlistbox.obj" : $(SOURCE) $(DEP_CPP_QLIST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qmenubar.cpp
DEP_CPP_QMENU=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qaccel.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdrawutl.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qkeycode.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmenubar.h"\
	{$(INCLUDE)}"\qmenudta.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpopmenu.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsignal.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qmenubar.obj" : $(SOURCE) $(DEP_CPP_QMENU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qmenubar.obj" : $(SOURCE) $(DEP_CPP_QMENU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qmenudta.cpp
DEP_CPP_QMENUD=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmenudta.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpopmenu.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsignal.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qmenudta.obj" : $(SOURCE) $(DEP_CPP_QMENUD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qmenudta.obj" : $(SOURCE) $(DEP_CPP_QMENUD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qmetaobj.cpp
DEP_CPP_QMETA=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qdict.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobjcoll.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qstrlist.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qmetaobj.obj" : $(SOURCE) $(DEP_CPP_QMETA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qmetaobj.obj" : $(SOURCE) $(DEP_CPP_QMETA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qmsgbox.cpp
DEP_CPP_QMSGB=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdialog.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlabel.h"\
	{$(INCLUDE)}"\qmsgbox.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpushbt.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwindow.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qmsgbox.obj" : $(SOURCE) $(DEP_CPP_QMSGB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qmsgbox.obj" : $(SOURCE) $(DEP_CPP_QMSGB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qobject.cpp
DEP_CPP_QOBJE=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qdict.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobjcoll.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregexp.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qobject.obj" : $(SOURCE) $(DEP_CPP_QOBJE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qobject.obj" : $(SOURCE) $(DEP_CPP_QOBJE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpainter.cpp
DEP_CPP_QPAIN=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbitmap.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpaintdc.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstack.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpainter.obj" : $(SOURCE) $(DEP_CPP_QPAIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpainter.obj" : $(SOURCE) $(DEP_CPP_QPAIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpalette.cpp
DEP_CPP_QPALE=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpalette.obj" : $(SOURCE) $(DEP_CPP_QPALE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpalette.obj" : $(SOURCE) $(DEP_CPP_QPALE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpdevmet.cpp
DEP_CPP_QPDEV=\
	"..\include\qobjdefs.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpaintdc.h"\
	{$(INCLUDE)}"\qpdevmet.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpdevmet.obj" : $(SOURCE) $(DEP_CPP_QPDEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpdevmet.obj" : $(SOURCE) $(DEP_CPP_QPDEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpic_win.cpp
DEP_CPP_QPIC_=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbuffer.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpicture.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpic_win.obj" : $(SOURCE) $(DEP_CPP_QPIC_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpic_win.obj" : $(SOURCE) $(DEP_CPP_QPIC_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpicture.cpp
DEP_CPP_QPICT=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbuffer.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qfile.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpaintdc.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpicture.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpicture.obj" : $(SOURCE) $(DEP_CPP_QPICT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpicture.obj" : $(SOURCE) $(DEP_CPP_QPICT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpixmap.cpp
DEP_CPP_QPIXM=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbitmap.h"\
	{$(INCLUDE)}"\qbuffer.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qimage.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpixmap.obj" : $(SOURCE) $(DEP_CPP_QPIXM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpixmap.obj" : $(SOURCE) $(DEP_CPP_QPIXM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpm_win.cpp
DEP_CPP_QPM_W=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qbitmap.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qimage.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpaintdc.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpm_win.obj" : $(SOURCE) $(DEP_CPP_QPM_W) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpm_win.obj" : $(SOURCE) $(DEP_CPP_QPM_W) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpmcache.cpp
DEP_CPP_QPMCA=\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcache.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qgcache.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpmcache.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpmcache.obj" : $(SOURCE) $(DEP_CPP_QPMCA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpmcache.obj" : $(SOURCE) $(DEP_CPP_QPMCA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpntarry.cpp
DEP_CPP_QPNTA=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbitarry.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpntarry.obj" : $(SOURCE) $(DEP_CPP_QPNTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpntarry.obj" : $(SOURCE) $(DEP_CPP_QPNTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpoint.cpp
DEP_CPP_QPOIN=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpoint.obj" : $(SOURCE) $(DEP_CPP_QPOIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpoint.obj" : $(SOURCE) $(DEP_CPP_QPOIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpopmenu.cpp
DEP_CPP_QPOPM=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qaccel.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdrawutl.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qkeycode.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmenubar.h"\
	{$(INCLUDE)}"\qmenudta.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpopmenu.h"\
	{$(INCLUDE)}"\qrangect.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qscrbar.h"\
	{$(INCLUDE)}"\qsignal.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpopmenu.obj" : $(SOURCE) $(DEP_CPP_QPOPM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpopmenu.obj" : $(SOURCE) $(DEP_CPP_QPOPM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qprinter.cpp
DEP_CPP_QPRIN=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qprinter.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qprinter.obj" : $(SOURCE) $(DEP_CPP_QPRIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qprinter.obj" : $(SOURCE) $(DEP_CPP_QPRIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qprn_win.cpp
DEP_CPP_QPRN_=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpaintdc.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qprinter.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qprn_win.obj" : $(SOURCE) $(DEP_CPP_QPRN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qprn_win.obj" : $(SOURCE) $(DEP_CPP_QPRN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qptd_win.cpp
DEP_CPP_QPTD_=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qbitmap.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpaintdc.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qptd_win.obj" : $(SOURCE) $(DEP_CPP_QPTD_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qptd_win.obj" : $(SOURCE) $(DEP_CPP_QPTD_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qptr_win.cpp
DEP_CPP_QPTR_=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbitmap.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qintdict.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpaintdc.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpmcache.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtstream.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qptr_win.obj" : $(SOURCE) $(DEP_CPP_QPTR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qptr_win.obj" : $(SOURCE) $(DEP_CPP_QPTR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qpushbt.cpp
DEP_CPP_QPUSH=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdialog.h"\
	{$(INCLUDE)}"\qdrawutl.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpmcache.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qpushbt.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwindow.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qpushbt.obj" : $(SOURCE) $(DEP_CPP_QPUSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qpushbt.obj" : $(SOURCE) $(DEP_CPP_QPUSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qradiobt.cpp
DEP_CPP_QRADI=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbttngrp.h"\
	{$(INCLUDE)}"\qbutton.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qgrpbox.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpmcache.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qradiobt.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qradiobt.obj" : $(SOURCE) $(DEP_CPP_QRADI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qradiobt.obj" : $(SOURCE) $(DEP_CPP_QRADI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qrangect.cpp
DEP_CPP_QRANG=\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qrangect.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qrangect.obj" : $(SOURCE) $(DEP_CPP_QRANG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qrangect.obj" : $(SOURCE) $(DEP_CPP_QRANG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qrect.cpp
DEP_CPP_QRECT=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qrect.obj" : $(SOURCE) $(DEP_CPP_QRECT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qrect.obj" : $(SOURCE) $(DEP_CPP_QRECT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qregexp.obj" : $(SOURCE) $(DEP_CPP_QREGE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qregexp.obj" : $(SOURCE) $(DEP_CPP_QREGE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qregion.cpp
DEP_CPP_QREGI=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbuffer.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qregion.obj" : $(SOURCE) $(DEP_CPP_QREGI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qregion.obj" : $(SOURCE) $(DEP_CPP_QREGI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qrgn_win.cpp
DEP_CPP_QRGN_=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbuffer.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qrgn_win.obj" : $(SOURCE) $(DEP_CPP_QRGN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qrgn_win.obj" : $(SOURCE) $(DEP_CPP_QRGN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qscrbar.cpp
DEP_CPP_QSCRB=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qbitmap.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdrawutl.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrangect.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qscrbar.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qscrbar.obj" : $(SOURCE) $(DEP_CPP_QSCRB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qscrbar.obj" : $(SOURCE) $(DEP_CPP_QSCRB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qsignal.cpp
DEP_CPP_QSIGN=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsignal.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qsignal.obj" : $(SOURCE) $(DEP_CPP_QSIGN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qsignal.obj" : $(SOURCE) $(DEP_CPP_QSIGN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qsize.cpp
DEP_CPP_QSIZE=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qdstream.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qiodev.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qsize.obj" : $(SOURCE) $(DEP_CPP_QSIZE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qsize.obj" : $(SOURCE) $(DEP_CPP_QSIZE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qsocknot.cpp
DEP_CPP_QSOCK=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qsocknot.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qsocknot.obj" : $(SOURCE) $(DEP_CPP_QSOCK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qsocknot.obj" : $(SOURCE) $(DEP_CPP_QSOCK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qstring.obj" : $(SOURCE) $(DEP_CPP_QSTRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qstring.obj" : $(SOURCE) $(DEP_CPP_QSTRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qtablevw.cpp
DEP_CPP_QTABL=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdrawutl.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qframe.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrangect.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qscrbar.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtablevw.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qtablevw.obj" : $(SOURCE) $(DEP_CPP_QTABL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qtablevw.obj" : $(SOURCE) $(DEP_CPP_QTABL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qtimer.cpp
DEP_CPP_QTIME=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qtimer.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qtimer.obj" : $(SOURCE) $(DEP_CPP_QTIME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qtimer.obj" : $(SOURCE) $(DEP_CPP_QTIME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

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


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qtstream.obj" : $(SOURCE) $(DEP_CPP_QTSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qtstream.obj" : $(SOURCE) $(DEP_CPP_QTSTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qwid_win.cpp
DEP_CPP_QWID_=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qpen.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdict.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qintdict.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobjcoll.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpaintdc.h"\
	{$(INCLUDE)}"\qpainter.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpntarry.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qregion.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidcoll.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwindow.h"\
	{$(INCLUDE)}"\qwmatrix.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qwid_win.obj" : $(SOURCE) $(DEP_CPP_QWID_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qwid_win.obj" : $(SOURCE) $(DEP_CPP_QWID_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qwidget.cpp
DEP_CPP_QWIDG=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qapp.h"\
	{$(INCLUDE)}"\qcollect.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qdict.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qgdict.h"\
	{$(INCLUDE)}"\qglist.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qintdict.h"\
	{$(INCLUDE)}"\qkeycode.h"\
	{$(INCLUDE)}"\qlist.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobjcoll.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidcoll.h"\
	{$(INCLUDE)}"\qwidget.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qwidget.obj" : $(SOURCE) $(DEP_CPP_QWIDG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qwidget.obj" : $(SOURCE) $(DEP_CPP_QWIDG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qwindow.cpp
DEP_CPP_QWIND=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qcolor.h"\
	{$(INCLUDE)}"\qcursor.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qfont.h"\
	{$(INCLUDE)}"\qfontinf.h"\
	{$(INCLUDE)}"\qfontmet.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpaintd.h"\
	{$(INCLUDE)}"\qpalette.h"\
	{$(INCLUDE)}"\qpixmap.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\
	{$(INCLUDE)}"\qwidget.h"\
	{$(INCLUDE)}"\qwindow.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\qwindow.obj" : $(SOURCE) $(DEP_CPP_QWIND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\qwindow.obj" : $(SOURCE) $(DEP_CPP_QWIND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\maccel.cpp
DEP_CPP_MACCE=\
	"..\include\qarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qobjdefs.h"\
	"..\include\qshared.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qaccel.h"\
	{$(INCLUDE)}"\qconnect.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qgarray.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qkeycode.h"\
	{$(INCLUDE)}"\qmetaobj.h"\
	{$(INCLUDE)}"\qobject.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\
	{$(INCLUDE)}"\qstring.h"\


!IF  "$(CFG)" == "qt - Win32 Release"


".\Release\maccel.obj" : $(SOURCE) $(DEP_CPP_MACCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF	 "$(CFG)" == "qt - Win32 Debug"


".\Debug\maccel.obj" : $(SOURCE) $(DEP_CPP_MACCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
# End Target
# End Project
################################################################################
