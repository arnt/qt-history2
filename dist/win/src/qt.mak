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
# PROP Output_Dir "qt.release"
# PROP Intermediate_Dir "qt.release"
# PROP Target_Dir ""
OUTDIR=.\qt.release
INTDIR=.\qt.release

ALL : "..\lib\qt.lib"

CLEAN :
	-@erase "$(INTDIR)\maccel.obj"
	-@erase "$(INTDIR)\mapp.obj"
	-@erase "$(INTDIR)\mbttngrp.obj"
	-@erase "$(INTDIR)\mbutton.obj"
	-@erase "$(INTDIR)\mchkbox.obj"
	-@erase "$(INTDIR)\mclipbrd.obj"
	-@erase "$(INTDIR)\mcombo.obj"
	-@erase "$(INTDIR)\mdialog.obj"
	-@erase "$(INTDIR)\mfiledlg.obj"
	-@erase "$(INTDIR)\mframe.obj"
	-@erase "$(INTDIR)\mgrpbox.obj"
	-@erase "$(INTDIR)\mlabel.obj"
	-@erase "$(INTDIR)\mlcdnum.obj"
	-@erase "$(INTDIR)\mlined.obj"
	-@erase "$(INTDIR)\mlistbox.obj"
	-@erase "$(INTDIR)\mmenubar.obj"
	-@erase "$(INTDIR)\mmsgbox.obj"
	-@erase "$(INTDIR)\mpopmenu.obj"
	-@erase "$(INTDIR)\mpushbt.obj"
	-@erase "$(INTDIR)\mradiobt.obj"
	-@erase "$(INTDIR)\mscrbar.obj"
	-@erase "$(INTDIR)\msocknot.obj"
	-@erase "$(INTDIR)\mtablevw.obj"
	-@erase "$(INTDIR)\mtimer.obj"
	-@erase "$(INTDIR)\mwidget.obj"
	-@erase "$(INTDIR)\mwindow.obj"
	-@erase "$(INTDIR)\qaccel.obj"
	-@erase "$(INTDIR)\qapp.obj"
	-@erase "$(INTDIR)\qapp_win.obj"
	-@erase "$(INTDIR)\qbitarry.obj"
	-@erase "$(INTDIR)\qbitmap.obj"
	-@erase "$(INTDIR)\qbttngrp.obj"
	-@erase "$(INTDIR)\qbuffer.obj"
	-@erase "$(INTDIR)\qbutton.obj"
	-@erase "$(INTDIR)\qchkbox.obj"
	-@erase "$(INTDIR)\qclb_win.obj"
	-@erase "$(INTDIR)\qclipbrd.obj"
	-@erase "$(INTDIR)\qcol_win.obj"
	-@erase "$(INTDIR)\qcollect.obj"
	-@erase "$(INTDIR)\qcolor.obj"
	-@erase "$(INTDIR)\qcombo.obj"
	-@erase "$(INTDIR)\qconnect.obj"
	-@erase "$(INTDIR)\qcur_win.obj"
	-@erase "$(INTDIR)\qcursor.obj"
	-@erase "$(INTDIR)\qdatetm.obj"
	-@erase "$(INTDIR)\qdialog.obj"
	-@erase "$(INTDIR)\qdir.obj"
	-@erase "$(INTDIR)\qdrawutl.obj"
	-@erase "$(INTDIR)\qdstream.obj"
	-@erase "$(INTDIR)\qevent.obj"
	-@erase "$(INTDIR)\qfile.obj"
	-@erase "$(INTDIR)\qfiledlg.obj"
	-@erase "$(INTDIR)\qfileinf.obj"
	-@erase "$(INTDIR)\qfnt_win.obj"
	-@erase "$(INTDIR)\qfont.obj"
	-@erase "$(INTDIR)\qframe.obj"
	-@erase "$(INTDIR)\qgarray.obj"
	-@erase "$(INTDIR)\qgcache.obj"
	-@erase "$(INTDIR)\qgdict.obj"
	-@erase "$(INTDIR)\qglist.obj"
	-@erase "$(INTDIR)\qglobal.obj"
	-@erase "$(INTDIR)\qgrpbox.obj"
	-@erase "$(INTDIR)\qgvector.obj"
	-@erase "$(INTDIR)\qimage.obj"
	-@erase "$(INTDIR)\qiodev.obj"
	-@erase "$(INTDIR)\qlabel.obj"
	-@erase "$(INTDIR)\qlcdnum.obj"
	-@erase "$(INTDIR)\qlined.obj"
	-@erase "$(INTDIR)\qlistbox.obj"
	-@erase "$(INTDIR)\qmenubar.obj"
	-@erase "$(INTDIR)\qmenudta.obj"
	-@erase "$(INTDIR)\qmetaobj.obj"
	-@erase "$(INTDIR)\qmsgbox.obj"
	-@erase "$(INTDIR)\qobject.obj"
	-@erase "$(INTDIR)\qpainter.obj"
	-@erase "$(INTDIR)\qpalette.obj"
	-@erase "$(INTDIR)\qpdevmet.obj"
	-@erase "$(INTDIR)\qpic_win.obj"
	-@erase "$(INTDIR)\qpicture.obj"
	-@erase "$(INTDIR)\qpixmap.obj"
	-@erase "$(INTDIR)\qpm_win.obj"
	-@erase "$(INTDIR)\qpmcache.obj"
	-@erase "$(INTDIR)\qpntarry.obj"
	-@erase "$(INTDIR)\qpoint.obj"
	-@erase "$(INTDIR)\qpopmenu.obj"
	-@erase "$(INTDIR)\qprinter.obj"
	-@erase "$(INTDIR)\qprn_win.obj"
	-@erase "$(INTDIR)\qptd_win.obj"
	-@erase "$(INTDIR)\qptr_win.obj"
	-@erase "$(INTDIR)\qpushbt.obj"
	-@erase "$(INTDIR)\qradiobt.obj"
	-@erase "$(INTDIR)\qrangect.obj"
	-@erase "$(INTDIR)\qrect.obj"
	-@erase "$(INTDIR)\qregexp.obj"
	-@erase "$(INTDIR)\qregion.obj"
	-@erase "$(INTDIR)\qrgn_win.obj"
	-@erase "$(INTDIR)\qscrbar.obj"
	-@erase "$(INTDIR)\qsignal.obj"
	-@erase "$(INTDIR)\qsize.obj"
	-@erase "$(INTDIR)\qsocknot.obj"
	-@erase "$(INTDIR)\qstring.obj"
	-@erase "$(INTDIR)\qtablevw.obj"
	-@erase "$(INTDIR)\qtimer.obj"
	-@erase "$(INTDIR)\qtstream.obj"
	-@erase "$(INTDIR)\qwid_win.obj"
	-@erase "$(INTDIR)\qwidget.obj"
	-@erase "$(INTDIR)\qwindow.obj"
	-@erase "$(INTDIR)\qwmatrix.obj"
	-@erase "..\lib\qt.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qt.pch" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\qt.release/
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
	"$(INTDIR)\maccel.obj" \
	"$(INTDIR)\mapp.obj" \
	"$(INTDIR)\mbttngrp.obj" \
	"$(INTDIR)\mbutton.obj" \
	"$(INTDIR)\mchkbox.obj" \
	"$(INTDIR)\mclipbrd.obj" \
	"$(INTDIR)\mcombo.obj" \
	"$(INTDIR)\mdialog.obj" \
	"$(INTDIR)\mfiledlg.obj" \
	"$(INTDIR)\mframe.obj" \
	"$(INTDIR)\mgrpbox.obj" \
	"$(INTDIR)\mlabel.obj" \
	"$(INTDIR)\mlcdnum.obj" \
	"$(INTDIR)\mlined.obj" \
	"$(INTDIR)\mlistbox.obj" \
	"$(INTDIR)\mmenubar.obj" \
	"$(INTDIR)\mmsgbox.obj" \
	"$(INTDIR)\mpopmenu.obj" \
	"$(INTDIR)\mpushbt.obj" \
	"$(INTDIR)\mradiobt.obj" \
	"$(INTDIR)\mscrbar.obj" \
	"$(INTDIR)\msocknot.obj" \
	"$(INTDIR)\mtablevw.obj" \
	"$(INTDIR)\mtimer.obj" \
	"$(INTDIR)\mwidget.obj" \
	"$(INTDIR)\mwindow.obj" \
	"$(INTDIR)\qaccel.obj" \
	"$(INTDIR)\qapp.obj" \
	"$(INTDIR)\qapp_win.obj" \
	"$(INTDIR)\qbitarry.obj" \
	"$(INTDIR)\qbitmap.obj" \
	"$(INTDIR)\qbttngrp.obj" \
	"$(INTDIR)\qbuffer.obj" \
	"$(INTDIR)\qbutton.obj" \
	"$(INTDIR)\qchkbox.obj" \
	"$(INTDIR)\qclb_win.obj" \
	"$(INTDIR)\qclipbrd.obj" \
	"$(INTDIR)\qcol_win.obj" \
	"$(INTDIR)\qcollect.obj" \
	"$(INTDIR)\qcolor.obj" \
	"$(INTDIR)\qcombo.obj" \
	"$(INTDIR)\qconnect.obj" \
	"$(INTDIR)\qcur_win.obj" \
	"$(INTDIR)\qcursor.obj" \
	"$(INTDIR)\qdatetm.obj" \
	"$(INTDIR)\qdialog.obj" \
	"$(INTDIR)\qdir.obj" \
	"$(INTDIR)\qdrawutl.obj" \
	"$(INTDIR)\qdstream.obj" \
	"$(INTDIR)\qevent.obj" \
	"$(INTDIR)\qfile.obj" \
	"$(INTDIR)\qfiledlg.obj" \
	"$(INTDIR)\qfileinf.obj" \
	"$(INTDIR)\qfnt_win.obj" \
	"$(INTDIR)\qfont.obj" \
	"$(INTDIR)\qframe.obj" \
	"$(INTDIR)\qgarray.obj" \
	"$(INTDIR)\qgcache.obj" \
	"$(INTDIR)\qgdict.obj" \
	"$(INTDIR)\qglist.obj" \
	"$(INTDIR)\qglobal.obj" \
	"$(INTDIR)\qgrpbox.obj" \
	"$(INTDIR)\qgvector.obj" \
	"$(INTDIR)\qimage.obj" \
	"$(INTDIR)\qiodev.obj" \
	"$(INTDIR)\qlabel.obj" \
	"$(INTDIR)\qlcdnum.obj" \
	"$(INTDIR)\qlined.obj" \
	"$(INTDIR)\qlistbox.obj" \
	"$(INTDIR)\qmenubar.obj" \
	"$(INTDIR)\qmenudta.obj" \
	"$(INTDIR)\qmetaobj.obj" \
	"$(INTDIR)\qmsgbox.obj" \
	"$(INTDIR)\qobject.obj" \
	"$(INTDIR)\qpainter.obj" \
	"$(INTDIR)\qpalette.obj" \
	"$(INTDIR)\qpdevmet.obj" \
	"$(INTDIR)\qpic_win.obj" \
	"$(INTDIR)\qpicture.obj" \
	"$(INTDIR)\qpixmap.obj" \
	"$(INTDIR)\qpm_win.obj" \
	"$(INTDIR)\qpmcache.obj" \
	"$(INTDIR)\qpntarry.obj" \
	"$(INTDIR)\qpoint.obj" \
	"$(INTDIR)\qpopmenu.obj" \
	"$(INTDIR)\qprinter.obj" \
	"$(INTDIR)\qprn_win.obj" \
	"$(INTDIR)\qptd_win.obj" \
	"$(INTDIR)\qptr_win.obj" \
	"$(INTDIR)\qpushbt.obj" \
	"$(INTDIR)\qradiobt.obj" \
	"$(INTDIR)\qrangect.obj" \
	"$(INTDIR)\qrect.obj" \
	"$(INTDIR)\qregexp.obj" \
	"$(INTDIR)\qregion.obj" \
	"$(INTDIR)\qrgn_win.obj" \
	"$(INTDIR)\qscrbar.obj" \
	"$(INTDIR)\qsignal.obj" \
	"$(INTDIR)\qsize.obj" \
	"$(INTDIR)\qsocknot.obj" \
	"$(INTDIR)\qstring.obj" \
	"$(INTDIR)\qtablevw.obj" \
	"$(INTDIR)\qtimer.obj" \
	"$(INTDIR)\qtstream.obj" \
	"$(INTDIR)\qwid_win.obj" \
	"$(INTDIR)\qwidget.obj" \
	"$(INTDIR)\qwindow.obj" \
	"$(INTDIR)\qwmatrix.obj"

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
# PROP Output_Dir "qt.debug"
# PROP Intermediate_Dir "qt.debug"
# PROP Target_Dir ""
OUTDIR=.\qt.debug
INTDIR=.\qt.debug

ALL : "..\lib\qt.lib"

CLEAN :
	-@erase "$(INTDIR)\maccel.obj"
	-@erase "$(INTDIR)\mapp.obj"
	-@erase "$(INTDIR)\mbttngrp.obj"
	-@erase "$(INTDIR)\mbutton.obj"
	-@erase "$(INTDIR)\mchkbox.obj"
	-@erase "$(INTDIR)\mclipbrd.obj"
	-@erase "$(INTDIR)\mcombo.obj"
	-@erase "$(INTDIR)\mdialog.obj"
	-@erase "$(INTDIR)\mfiledlg.obj"
	-@erase "$(INTDIR)\mframe.obj"
	-@erase "$(INTDIR)\mgrpbox.obj"
	-@erase "$(INTDIR)\mlabel.obj"
	-@erase "$(INTDIR)\mlcdnum.obj"
	-@erase "$(INTDIR)\mlined.obj"
	-@erase "$(INTDIR)\mlistbox.obj"
	-@erase "$(INTDIR)\mmenubar.obj"
	-@erase "$(INTDIR)\mmsgbox.obj"
	-@erase "$(INTDIR)\mpopmenu.obj"
	-@erase "$(INTDIR)\mpushbt.obj"
	-@erase "$(INTDIR)\mradiobt.obj"
	-@erase "$(INTDIR)\mscrbar.obj"
	-@erase "$(INTDIR)\msocknot.obj"
	-@erase "$(INTDIR)\mtablevw.obj"
	-@erase "$(INTDIR)\mtimer.obj"
	-@erase "$(INTDIR)\mwidget.obj"
	-@erase "$(INTDIR)\mwindow.obj"
	-@erase "$(INTDIR)\qaccel.obj"
	-@erase "$(INTDIR)\qapp.obj"
	-@erase "$(INTDIR)\qapp_win.obj"
	-@erase "$(INTDIR)\qbitarry.obj"
	-@erase "$(INTDIR)\qbitmap.obj"
	-@erase "$(INTDIR)\qbttngrp.obj"
	-@erase "$(INTDIR)\qbuffer.obj"
	-@erase "$(INTDIR)\qbutton.obj"
	-@erase "$(INTDIR)\qchkbox.obj"
	-@erase "$(INTDIR)\qclb_win.obj"
	-@erase "$(INTDIR)\qclipbrd.obj"
	-@erase "$(INTDIR)\qcol_win.obj"
	-@erase "$(INTDIR)\qcollect.obj"
	-@erase "$(INTDIR)\qcolor.obj"
	-@erase "$(INTDIR)\qcombo.obj"
	-@erase "$(INTDIR)\qconnect.obj"
	-@erase "$(INTDIR)\qcur_win.obj"
	-@erase "$(INTDIR)\qcursor.obj"
	-@erase "$(INTDIR)\qdatetm.obj"
	-@erase "$(INTDIR)\qdialog.obj"
	-@erase "$(INTDIR)\qdir.obj"
	-@erase "$(INTDIR)\qdrawutl.obj"
	-@erase "$(INTDIR)\qdstream.obj"
	-@erase "$(INTDIR)\qevent.obj"
	-@erase "$(INTDIR)\qfile.obj"
	-@erase "$(INTDIR)\qfiledlg.obj"
	-@erase "$(INTDIR)\qfileinf.obj"
	-@erase "$(INTDIR)\qfnt_win.obj"
	-@erase "$(INTDIR)\qfont.obj"
	-@erase "$(INTDIR)\qframe.obj"
	-@erase "$(INTDIR)\qgarray.obj"
	-@erase "$(INTDIR)\qgcache.obj"
	-@erase "$(INTDIR)\qgdict.obj"
	-@erase "$(INTDIR)\qglist.obj"
	-@erase "$(INTDIR)\qglobal.obj"
	-@erase "$(INTDIR)\qgrpbox.obj"
	-@erase "$(INTDIR)\qgvector.obj"
	-@erase "$(INTDIR)\qimage.obj"
	-@erase "$(INTDIR)\qiodev.obj"
	-@erase "$(INTDIR)\qlabel.obj"
	-@erase "$(INTDIR)\qlcdnum.obj"
	-@erase "$(INTDIR)\qlined.obj"
	-@erase "$(INTDIR)\qlistbox.obj"
	-@erase "$(INTDIR)\qmenubar.obj"
	-@erase "$(INTDIR)\qmenudta.obj"
	-@erase "$(INTDIR)\qmetaobj.obj"
	-@erase "$(INTDIR)\qmsgbox.obj"
	-@erase "$(INTDIR)\qobject.obj"
	-@erase "$(INTDIR)\qpainter.obj"
	-@erase "$(INTDIR)\qpalette.obj"
	-@erase "$(INTDIR)\qpdevmet.obj"
	-@erase "$(INTDIR)\qpic_win.obj"
	-@erase "$(INTDIR)\qpicture.obj"
	-@erase "$(INTDIR)\qpixmap.obj"
	-@erase "$(INTDIR)\qpm_win.obj"
	-@erase "$(INTDIR)\qpmcache.obj"
	-@erase "$(INTDIR)\qpntarry.obj"
	-@erase "$(INTDIR)\qpoint.obj"
	-@erase "$(INTDIR)\qpopmenu.obj"
	-@erase "$(INTDIR)\qprinter.obj"
	-@erase "$(INTDIR)\qprn_win.obj"
	-@erase "$(INTDIR)\qptd_win.obj"
	-@erase "$(INTDIR)\qptr_win.obj"
	-@erase "$(INTDIR)\qpushbt.obj"
	-@erase "$(INTDIR)\qradiobt.obj"
	-@erase "$(INTDIR)\qrangect.obj"
	-@erase "$(INTDIR)\qrect.obj"
	-@erase "$(INTDIR)\qregexp.obj"
	-@erase "$(INTDIR)\qregion.obj"
	-@erase "$(INTDIR)\qrgn_win.obj"
	-@erase "$(INTDIR)\qscrbar.obj"
	-@erase "$(INTDIR)\qsignal.obj"
	-@erase "$(INTDIR)\qsize.obj"
	-@erase "$(INTDIR)\qsocknot.obj"
	-@erase "$(INTDIR)\qstring.obj"
	-@erase "$(INTDIR)\qtablevw.obj"
	-@erase "$(INTDIR)\qtimer.obj"
	-@erase "$(INTDIR)\qtstream.obj"
	-@erase "$(INTDIR)\qwid_win.obj"
	-@erase "$(INTDIR)\qwidget.obj"
	-@erase "$(INTDIR)\qwindow.obj"
	-@erase "$(INTDIR)\qwmatrix.obj"
	-@erase "..\lib\qt.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qt.pch" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\qt.debug/
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
	"$(INTDIR)\maccel.obj" \
	"$(INTDIR)\mapp.obj" \
	"$(INTDIR)\mbttngrp.obj" \
	"$(INTDIR)\mbutton.obj" \
	"$(INTDIR)\mchkbox.obj" \
	"$(INTDIR)\mclipbrd.obj" \
	"$(INTDIR)\mcombo.obj" \
	"$(INTDIR)\mdialog.obj" \
	"$(INTDIR)\mfiledlg.obj" \
	"$(INTDIR)\mframe.obj" \
	"$(INTDIR)\mgrpbox.obj" \
	"$(INTDIR)\mlabel.obj" \
	"$(INTDIR)\mlcdnum.obj" \
	"$(INTDIR)\mlined.obj" \
	"$(INTDIR)\mlistbox.obj" \
	"$(INTDIR)\mmenubar.obj" \
	"$(INTDIR)\mmsgbox.obj" \
	"$(INTDIR)\mpopmenu.obj" \
	"$(INTDIR)\mpushbt.obj" \
	"$(INTDIR)\mradiobt.obj" \
	"$(INTDIR)\mscrbar.obj" \
	"$(INTDIR)\msocknot.obj" \
	"$(INTDIR)\mtablevw.obj" \
	"$(INTDIR)\mtimer.obj" \
	"$(INTDIR)\mwidget.obj" \
	"$(INTDIR)\mwindow.obj" \
	"$(INTDIR)\qaccel.obj" \
	"$(INTDIR)\qapp.obj" \
	"$(INTDIR)\qapp_win.obj" \
	"$(INTDIR)\qbitarry.obj" \
	"$(INTDIR)\qbitmap.obj" \
	"$(INTDIR)\qbttngrp.obj" \
	"$(INTDIR)\qbuffer.obj" \
	"$(INTDIR)\qbutton.obj" \
	"$(INTDIR)\qchkbox.obj" \
	"$(INTDIR)\qclb_win.obj" \
	"$(INTDIR)\qclipbrd.obj" \
	"$(INTDIR)\qcol_win.obj" \
	"$(INTDIR)\qcollect.obj" \
	"$(INTDIR)\qcolor.obj" \
	"$(INTDIR)\qcombo.obj" \
	"$(INTDIR)\qconnect.obj" \
	"$(INTDIR)\qcur_win.obj" \
	"$(INTDIR)\qcursor.obj" \
	"$(INTDIR)\qdatetm.obj" \
	"$(INTDIR)\qdialog.obj" \
	"$(INTDIR)\qdir.obj" \
	"$(INTDIR)\qdrawutl.obj" \
	"$(INTDIR)\qdstream.obj" \
	"$(INTDIR)\qevent.obj" \
	"$(INTDIR)\qfile.obj" \
	"$(INTDIR)\qfiledlg.obj" \
	"$(INTDIR)\qfileinf.obj" \
	"$(INTDIR)\qfnt_win.obj" \
	"$(INTDIR)\qfont.obj" \
	"$(INTDIR)\qframe.obj" \
	"$(INTDIR)\qgarray.obj" \
	"$(INTDIR)\qgcache.obj" \
	"$(INTDIR)\qgdict.obj" \
	"$(INTDIR)\qglist.obj" \
	"$(INTDIR)\qglobal.obj" \
	"$(INTDIR)\qgrpbox.obj" \
	"$(INTDIR)\qgvector.obj" \
	"$(INTDIR)\qimage.obj" \
	"$(INTDIR)\qiodev.obj" \
	"$(INTDIR)\qlabel.obj" \
	"$(INTDIR)\qlcdnum.obj" \
	"$(INTDIR)\qlined.obj" \
	"$(INTDIR)\qlistbox.obj" \
	"$(INTDIR)\qmenubar.obj" \
	"$(INTDIR)\qmenudta.obj" \
	"$(INTDIR)\qmetaobj.obj" \
	"$(INTDIR)\qmsgbox.obj" \
	"$(INTDIR)\qobject.obj" \
	"$(INTDIR)\qpainter.obj" \
	"$(INTDIR)\qpalette.obj" \
	"$(INTDIR)\qpdevmet.obj" \
	"$(INTDIR)\qpic_win.obj" \
	"$(INTDIR)\qpicture.obj" \
	"$(INTDIR)\qpixmap.obj" \
	"$(INTDIR)\qpm_win.obj" \
	"$(INTDIR)\qpmcache.obj" \
	"$(INTDIR)\qpntarry.obj" \
	"$(INTDIR)\qpoint.obj" \
	"$(INTDIR)\qpopmenu.obj" \
	"$(INTDIR)\qprinter.obj" \
	"$(INTDIR)\qprn_win.obj" \
	"$(INTDIR)\qptd_win.obj" \
	"$(INTDIR)\qptr_win.obj" \
	"$(INTDIR)\qpushbt.obj" \
	"$(INTDIR)\qradiobt.obj" \
	"$(INTDIR)\qrangect.obj" \
	"$(INTDIR)\qrect.obj" \
	"$(INTDIR)\qregexp.obj" \
	"$(INTDIR)\qregion.obj" \
	"$(INTDIR)\qrgn_win.obj" \
	"$(INTDIR)\qscrbar.obj" \
	"$(INTDIR)\qsignal.obj" \
	"$(INTDIR)\qsize.obj" \
	"$(INTDIR)\qsocknot.obj" \
	"$(INTDIR)\qstring.obj" \
	"$(INTDIR)\qtablevw.obj" \
	"$(INTDIR)\qtimer.obj" \
	"$(INTDIR)\qtstream.obj" \
	"$(INTDIR)\qwid_win.obj" \
	"$(INTDIR)\qwidget.obj" \
	"$(INTDIR)\qwindow.obj" \
	"$(INTDIR)\qwmatrix.obj"

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


"$(INTDIR)\qwmatrix.obj" : $(SOURCE) $(DEP_CPP_QWMAT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mapp.obj" : $(SOURCE) $(DEP_CPP_MAPP_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mbttngrp.obj" : $(SOURCE) $(DEP_CPP_MBTTN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mbutton.obj" : $(SOURCE) $(DEP_CPP_MBUTT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mchkbox.obj" : $(SOURCE) $(DEP_CPP_MCHKB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mclipbrd.obj" : $(SOURCE) $(DEP_CPP_MCLIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mcombo.obj" : $(SOURCE) $(DEP_CPP_MCOMB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mdialog.obj" : $(SOURCE) $(DEP_CPP_MDIAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mfiledlg.obj" : $(SOURCE) $(DEP_CPP_MFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mframe.obj" : $(SOURCE) $(DEP_CPP_MFRAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mgrpbox.obj" : $(SOURCE) $(DEP_CPP_MGRPB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mlabel.obj" : $(SOURCE) $(DEP_CPP_MLABE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mlcdnum.obj" : $(SOURCE) $(DEP_CPP_MLCDN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mlined.obj" : $(SOURCE) $(DEP_CPP_MLINE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mlistbox.obj" : $(SOURCE) $(DEP_CPP_MLIST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mmenubar.obj" : $(SOURCE) $(DEP_CPP_MMENU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mmsgbox.obj" : $(SOURCE) $(DEP_CPP_MMSGB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mpopmenu.obj" : $(SOURCE) $(DEP_CPP_MPOPM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mpushbt.obj" : $(SOURCE) $(DEP_CPP_MPUSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mradiobt.obj" : $(SOURCE) $(DEP_CPP_MRADI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mscrbar.obj" : $(SOURCE) $(DEP_CPP_MSCRB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\msocknot.obj" : $(SOURCE) $(DEP_CPP_MSOCK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mtablevw.obj" : $(SOURCE) $(DEP_CPP_MTABL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mtimer.obj" : $(SOURCE) $(DEP_CPP_MTIME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mwidget.obj" : $(SOURCE) $(DEP_CPP_MWIDG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\mwindow.obj" : $(SOURCE) $(DEP_CPP_MWIND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qaccel.obj" : $(SOURCE) $(DEP_CPP_QACCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qapp.obj" : $(SOURCE) $(DEP_CPP_QAPP_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qapp_win.obj" : $(SOURCE) $(DEP_CPP_QAPP_W) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qbitarry.obj" : $(SOURCE) $(DEP_CPP_QBITA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qbitmap.obj" : $(SOURCE) $(DEP_CPP_QBITM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qbttngrp.obj" : $(SOURCE) $(DEP_CPP_QBTTN) "$(INTDIR)"
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


"$(INTDIR)\qbutton.obj" : $(SOURCE) $(DEP_CPP_QBUTT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qchkbox.obj" : $(SOURCE) $(DEP_CPP_QCHKB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qclb_win.obj" : $(SOURCE) $(DEP_CPP_QCLB_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qclipbrd.obj" : $(SOURCE) $(DEP_CPP_QCLIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qcol_win.obj" : $(SOURCE) $(DEP_CPP_QCOL_) "$(INTDIR)"
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


"$(INTDIR)\qcolor.obj" : $(SOURCE) $(DEP_CPP_QCOLO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qcombo.obj" : $(SOURCE) $(DEP_CPP_QCOMB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qconnect.obj" : $(SOURCE) $(DEP_CPP_QCONN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qcur_win.obj" : $(SOURCE) $(DEP_CPP_QCUR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qcursor.obj" : $(SOURCE) $(DEP_CPP_QCURS) "$(INTDIR)"
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


"$(INTDIR)\qdialog.obj" : $(SOURCE) $(DEP_CPP_QDIAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qdir.obj" : $(SOURCE) $(DEP_CPP_QDIR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qdrawutl.obj" : $(SOURCE) $(DEP_CPP_QDRAW) "$(INTDIR)"
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

SOURCE=.\win\qevent.cpp
DEP_CPP_QEVEN=\
	"..\include\qobjdefs.h"\
	"..\include\qwindefs.h"\
	{$(INCLUDE)}"\qevent.h"\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qpoint.h"\
	{$(INCLUDE)}"\qrect.h"\
	{$(INCLUDE)}"\qsize.h"\


"$(INTDIR)\qevent.obj" : $(SOURCE) $(DEP_CPP_QEVEN) "$(INTDIR)"
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


"$(INTDIR)\qfiledlg.obj" : $(SOURCE) $(DEP_CPP_QFILED) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qfileinf.obj" : $(SOURCE) $(DEP_CPP_QFILEI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qfnt_win.obj" : $(SOURCE) $(DEP_CPP_QFNT_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qfont.obj" : $(SOURCE) $(DEP_CPP_QFONT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qframe.obj" : $(SOURCE) $(DEP_CPP_QFRAM) "$(INTDIR)"
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


"$(INTDIR)\qgrpbox.obj" : $(SOURCE) $(DEP_CPP_QGRPB) "$(INTDIR)"
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


"$(INTDIR)\qimage.obj" : $(SOURCE) $(DEP_CPP_QIMAG) "$(INTDIR)"
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


"$(INTDIR)\qlabel.obj" : $(SOURCE) $(DEP_CPP_QLABE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qlcdnum.obj" : $(SOURCE) $(DEP_CPP_QLCDN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qlined.obj" : $(SOURCE) $(DEP_CPP_QLINE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qlistbox.obj" : $(SOURCE) $(DEP_CPP_QLIST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qmenubar.obj" : $(SOURCE) $(DEP_CPP_QMENU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qmenudta.obj" : $(SOURCE) $(DEP_CPP_QMENUD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qmetaobj.obj" : $(SOURCE) $(DEP_CPP_QMETA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qmsgbox.obj" : $(SOURCE) $(DEP_CPP_QMSGB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qobject.obj" : $(SOURCE) $(DEP_CPP_QOBJE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpainter.obj" : $(SOURCE) $(DEP_CPP_QPAIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpalette.obj" : $(SOURCE) $(DEP_CPP_QPALE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpdevmet.obj" : $(SOURCE) $(DEP_CPP_QPDEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpic_win.obj" : $(SOURCE) $(DEP_CPP_QPIC_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpicture.obj" : $(SOURCE) $(DEP_CPP_QPICT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpixmap.obj" : $(SOURCE) $(DEP_CPP_QPIXM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpm_win.obj" : $(SOURCE) $(DEP_CPP_QPM_W) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpmcache.obj" : $(SOURCE) $(DEP_CPP_QPMCA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpntarry.obj" : $(SOURCE) $(DEP_CPP_QPNTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpoint.obj" : $(SOURCE) $(DEP_CPP_QPOIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpopmenu.obj" : $(SOURCE) $(DEP_CPP_QPOPM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qprinter.obj" : $(SOURCE) $(DEP_CPP_QPRIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qprn_win.obj" : $(SOURCE) $(DEP_CPP_QPRN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qptd_win.obj" : $(SOURCE) $(DEP_CPP_QPTD_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qptr_win.obj" : $(SOURCE) $(DEP_CPP_QPTR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qpushbt.obj" : $(SOURCE) $(DEP_CPP_QPUSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qradiobt.obj" : $(SOURCE) $(DEP_CPP_QRADI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\win\qrangect.cpp
DEP_CPP_QRANG=\
	{$(INCLUDE)}"\qglobal.h"\
	{$(INCLUDE)}"\qrangect.h"\


"$(INTDIR)\qrangect.obj" : $(SOURCE) $(DEP_CPP_QRANG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qrect.obj" : $(SOURCE) $(DEP_CPP_QRECT) "$(INTDIR)"
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


"$(INTDIR)\qregion.obj" : $(SOURCE) $(DEP_CPP_QREGI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qrgn_win.obj" : $(SOURCE) $(DEP_CPP_QRGN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qscrbar.obj" : $(SOURCE) $(DEP_CPP_QSCRB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qsignal.obj" : $(SOURCE) $(DEP_CPP_QSIGN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qsize.obj" : $(SOURCE) $(DEP_CPP_QSIZE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qsocknot.obj" : $(SOURCE) $(DEP_CPP_QSOCK) "$(INTDIR)"
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


"$(INTDIR)\qtablevw.obj" : $(SOURCE) $(DEP_CPP_QTABL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qtimer.obj" : $(SOURCE) $(DEP_CPP_QTIME) "$(INTDIR)"
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


"$(INTDIR)\qwid_win.obj" : $(SOURCE) $(DEP_CPP_QWID_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qwidget.obj" : $(SOURCE) $(DEP_CPP_QWIDG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\qwindow.obj" : $(SOURCE) $(DEP_CPP_QWIND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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


"$(INTDIR)\maccel.obj" : $(SOURCE) $(DEP_CPP_MACCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
