# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=qt - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to qt - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "qt - Win32 Release" && "$(CFG)" != "qt - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
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
	-@erase "$(INTDIR)\moc_qaccel.obj"
	-@erase "$(INTDIR)\moc_qapp.obj"
	-@erase "$(INTDIR)\moc_qbttngrp.obj"
	-@erase "$(INTDIR)\moc_qbutton.obj"
	-@erase "$(INTDIR)\moc_qchkbox.obj"
	-@erase "$(INTDIR)\moc_qclipbrd.obj"
	-@erase "$(INTDIR)\moc_qcombo.obj"
	-@erase "$(INTDIR)\moc_qdialog.obj"
	-@erase "$(INTDIR)\moc_qfiledlg.obj"
	-@erase "$(INTDIR)\moc_qframe.obj"
	-@erase "$(INTDIR)\moc_qgmanagr.obj"
	-@erase "$(INTDIR)\moc_qgrpbox.obj"
	-@erase "$(INTDIR)\moc_qlabel.obj"
	-@erase "$(INTDIR)\moc_qlcdnum.obj"
	-@erase "$(INTDIR)\moc_qlined.obj"
	-@erase "$(INTDIR)\moc_qlistbox.obj"
	-@erase "$(INTDIR)\moc_qmenubar.obj"
	-@erase "$(INTDIR)\moc_qmlined.obj"
	-@erase "$(INTDIR)\moc_qmsgbox.obj"
	-@erase "$(INTDIR)\moc_qpopmenu.obj"
	-@erase "$(INTDIR)\moc_qpushbt.obj"
	-@erase "$(INTDIR)\moc_qradiobt.obj"
	-@erase "$(INTDIR)\moc_qscrbar.obj"
	-@erase "$(INTDIR)\moc_qslider.obj"
	-@erase "$(INTDIR)\moc_qsocknot.obj"
	-@erase "$(INTDIR)\moc_qtabbar.obj"
	-@erase "$(INTDIR)\moc_qtabdlg.obj"
	-@erase "$(INTDIR)\moc_qtablevw.obj"
	-@erase "$(INTDIR)\moc_qtimer.obj"
	-@erase "$(INTDIR)\moc_qtooltip.obj"
	-@erase "$(INTDIR)\moc_qwidget.obj"
	-@erase "$(INTDIR)\moc_qwindow.obj"
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
	-@erase "$(INTDIR)\qgmanagr.obj"
	-@erase "$(INTDIR)\qgrpbox.obj"
	-@erase "$(INTDIR)\qgvector.obj"
	-@erase "$(INTDIR)\qimage.obj"
	-@erase "$(INTDIR)\qiodev.obj"
	-@erase "$(INTDIR)\qlabel.obj"
	-@erase "$(INTDIR)\qlayout.obj"
	-@erase "$(INTDIR)\qlcdnum.obj"
	-@erase "$(INTDIR)\qlined.obj"
	-@erase "$(INTDIR)\qlistbox.obj"
	-@erase "$(INTDIR)\qmenubar.obj"
	-@erase "$(INTDIR)\qmenudta.obj"
	-@erase "$(INTDIR)\qmetaobj.obj"
	-@erase "$(INTDIR)\qmlined.obj"
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
	-@erase "$(INTDIR)\qslider.obj"
	-@erase "$(INTDIR)\qsocknot.obj"
	-@erase "$(INTDIR)\qstring.obj"
	-@erase "$(INTDIR)\qtabbar.obj"
	-@erase "$(INTDIR)\qtabdlg.obj"
	-@erase "$(INTDIR)\qtablevw.obj"
	-@erase "$(INTDIR)\qtimer.obj"
	-@erase "$(INTDIR)\qtooltip.obj"
	-@erase "$(INTDIR)\qtstream.obj"
	-@erase "$(INTDIR)\qwid_win.obj"
	-@erase "$(INTDIR)\qwidget.obj"
	-@erase "$(INTDIR)\qwindow.obj"
	-@erase "$(INTDIR)\qwmatrix.obj"
	-@erase "..\lib\qt.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/qt.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qt.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\qt.lib"
LIB32_FLAGS=/nologo /out:"..\lib\qt.lib" 
LIB32_OBJS= \
	"$(INTDIR)\moc_qaccel.obj" \
	"$(INTDIR)\moc_qapp.obj" \
	"$(INTDIR)\moc_qbttngrp.obj" \
	"$(INTDIR)\moc_qbutton.obj" \
	"$(INTDIR)\moc_qchkbox.obj" \
	"$(INTDIR)\moc_qclipbrd.obj" \
	"$(INTDIR)\moc_qcombo.obj" \
	"$(INTDIR)\moc_qdialog.obj" \
	"$(INTDIR)\moc_qfiledlg.obj" \
	"$(INTDIR)\moc_qframe.obj" \
	"$(INTDIR)\moc_qgmanagr.obj" \
	"$(INTDIR)\moc_qgrpbox.obj" \
	"$(INTDIR)\moc_qlabel.obj" \
	"$(INTDIR)\moc_qlcdnum.obj" \
	"$(INTDIR)\moc_qlined.obj" \
	"$(INTDIR)\moc_qlistbox.obj" \
	"$(INTDIR)\moc_qmenubar.obj" \
	"$(INTDIR)\moc_qmlined.obj" \
	"$(INTDIR)\moc_qmsgbox.obj" \
	"$(INTDIR)\moc_qpopmenu.obj" \
	"$(INTDIR)\moc_qpushbt.obj" \
	"$(INTDIR)\moc_qradiobt.obj" \
	"$(INTDIR)\moc_qscrbar.obj" \
	"$(INTDIR)\moc_qslider.obj" \
	"$(INTDIR)\moc_qsocknot.obj" \
	"$(INTDIR)\moc_qtabbar.obj" \
	"$(INTDIR)\moc_qtabdlg.obj" \
	"$(INTDIR)\moc_qtablevw.obj" \
	"$(INTDIR)\moc_qtimer.obj" \
	"$(INTDIR)\moc_qtooltip.obj" \
	"$(INTDIR)\moc_qwidget.obj" \
	"$(INTDIR)\moc_qwindow.obj" \
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
	"$(INTDIR)\qgmanagr.obj" \
	"$(INTDIR)\qgrpbox.obj" \
	"$(INTDIR)\qgvector.obj" \
	"$(INTDIR)\qimage.obj" \
	"$(INTDIR)\qiodev.obj" \
	"$(INTDIR)\qlabel.obj" \
	"$(INTDIR)\qlayout.obj" \
	"$(INTDIR)\qlcdnum.obj" \
	"$(INTDIR)\qlined.obj" \
	"$(INTDIR)\qlistbox.obj" \
	"$(INTDIR)\qmenubar.obj" \
	"$(INTDIR)\qmenudta.obj" \
	"$(INTDIR)\qmetaobj.obj" \
	"$(INTDIR)\qmlined.obj" \
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
	"$(INTDIR)\qslider.obj" \
	"$(INTDIR)\qsocknot.obj" \
	"$(INTDIR)\qstring.obj" \
	"$(INTDIR)\qtabbar.obj" \
	"$(INTDIR)\qtabdlg.obj" \
	"$(INTDIR)\qtablevw.obj" \
	"$(INTDIR)\qtimer.obj" \
	"$(INTDIR)\qtooltip.obj" \
	"$(INTDIR)\qtstream.obj" \
	"$(INTDIR)\qwid_win.obj" \
	"$(INTDIR)\qwidget.obj" \
	"$(INTDIR)\qwindow.obj" \
	"$(INTDIR)\qwmatrix.obj"

"..\lib\qt.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qt - Win32 Debug"

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
	-@erase "$(INTDIR)\moc_qaccel.obj"
	-@erase "$(INTDIR)\moc_qapp.obj"
	-@erase "$(INTDIR)\moc_qbttngrp.obj"
	-@erase "$(INTDIR)\moc_qbutton.obj"
	-@erase "$(INTDIR)\moc_qchkbox.obj"
	-@erase "$(INTDIR)\moc_qclipbrd.obj"
	-@erase "$(INTDIR)\moc_qcombo.obj"
	-@erase "$(INTDIR)\moc_qdialog.obj"
	-@erase "$(INTDIR)\moc_qfiledlg.obj"
	-@erase "$(INTDIR)\moc_qframe.obj"
	-@erase "$(INTDIR)\moc_qgmanagr.obj"
	-@erase "$(INTDIR)\moc_qgrpbox.obj"
	-@erase "$(INTDIR)\moc_qlabel.obj"
	-@erase "$(INTDIR)\moc_qlcdnum.obj"
	-@erase "$(INTDIR)\moc_qlined.obj"
	-@erase "$(INTDIR)\moc_qlistbox.obj"
	-@erase "$(INTDIR)\moc_qmenubar.obj"
	-@erase "$(INTDIR)\moc_qmlined.obj"
	-@erase "$(INTDIR)\moc_qmsgbox.obj"
	-@erase "$(INTDIR)\moc_qpopmenu.obj"
	-@erase "$(INTDIR)\moc_qpushbt.obj"
	-@erase "$(INTDIR)\moc_qradiobt.obj"
	-@erase "$(INTDIR)\moc_qscrbar.obj"
	-@erase "$(INTDIR)\moc_qslider.obj"
	-@erase "$(INTDIR)\moc_qsocknot.obj"
	-@erase "$(INTDIR)\moc_qtabbar.obj"
	-@erase "$(INTDIR)\moc_qtabdlg.obj"
	-@erase "$(INTDIR)\moc_qtablevw.obj"
	-@erase "$(INTDIR)\moc_qtimer.obj"
	-@erase "$(INTDIR)\moc_qtooltip.obj"
	-@erase "$(INTDIR)\moc_qwidget.obj"
	-@erase "$(INTDIR)\moc_qwindow.obj"
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
	-@erase "$(INTDIR)\qgmanagr.obj"
	-@erase "$(INTDIR)\qgrpbox.obj"
	-@erase "$(INTDIR)\qgvector.obj"
	-@erase "$(INTDIR)\qimage.obj"
	-@erase "$(INTDIR)\qiodev.obj"
	-@erase "$(INTDIR)\qlabel.obj"
	-@erase "$(INTDIR)\qlayout.obj"
	-@erase "$(INTDIR)\qlcdnum.obj"
	-@erase "$(INTDIR)\qlined.obj"
	-@erase "$(INTDIR)\qlistbox.obj"
	-@erase "$(INTDIR)\qmenubar.obj"
	-@erase "$(INTDIR)\qmenudta.obj"
	-@erase "$(INTDIR)\qmetaobj.obj"
	-@erase "$(INTDIR)\qmlined.obj"
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
	-@erase "$(INTDIR)\qslider.obj"
	-@erase "$(INTDIR)\qsocknot.obj"
	-@erase "$(INTDIR)\qstring.obj"
	-@erase "$(INTDIR)\qtabbar.obj"
	-@erase "$(INTDIR)\qtabdlg.obj"
	-@erase "$(INTDIR)\qtablevw.obj"
	-@erase "$(INTDIR)\qtimer.obj"
	-@erase "$(INTDIR)\qtooltip.obj"
	-@erase "$(INTDIR)\qtstream.obj"
	-@erase "$(INTDIR)\qwid_win.obj"
	-@erase "$(INTDIR)\qwidget.obj"
	-@erase "$(INTDIR)\qwindow.obj"
	-@erase "$(INTDIR)\qwmatrix.obj"
	-@erase "..\lib\qt.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/qt.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qt.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\qt.lib"
LIB32_FLAGS=/nologo /out:"..\lib\qt.lib" 
LIB32_OBJS= \
	"$(INTDIR)\moc_qaccel.obj" \
	"$(INTDIR)\moc_qapp.obj" \
	"$(INTDIR)\moc_qbttngrp.obj" \
	"$(INTDIR)\moc_qbutton.obj" \
	"$(INTDIR)\moc_qchkbox.obj" \
	"$(INTDIR)\moc_qclipbrd.obj" \
	"$(INTDIR)\moc_qcombo.obj" \
	"$(INTDIR)\moc_qdialog.obj" \
	"$(INTDIR)\moc_qfiledlg.obj" \
	"$(INTDIR)\moc_qframe.obj" \
	"$(INTDIR)\moc_qgmanagr.obj" \
	"$(INTDIR)\moc_qgrpbox.obj" \
	"$(INTDIR)\moc_qlabel.obj" \
	"$(INTDIR)\moc_qlcdnum.obj" \
	"$(INTDIR)\moc_qlined.obj" \
	"$(INTDIR)\moc_qlistbox.obj" \
	"$(INTDIR)\moc_qmenubar.obj" \
	"$(INTDIR)\moc_qmlined.obj" \
	"$(INTDIR)\moc_qmsgbox.obj" \
	"$(INTDIR)\moc_qpopmenu.obj" \
	"$(INTDIR)\moc_qpushbt.obj" \
	"$(INTDIR)\moc_qradiobt.obj" \
	"$(INTDIR)\moc_qscrbar.obj" \
	"$(INTDIR)\moc_qslider.obj" \
	"$(INTDIR)\moc_qsocknot.obj" \
	"$(INTDIR)\moc_qtabbar.obj" \
	"$(INTDIR)\moc_qtabdlg.obj" \
	"$(INTDIR)\moc_qtablevw.obj" \
	"$(INTDIR)\moc_qtimer.obj" \
	"$(INTDIR)\moc_qtooltip.obj" \
	"$(INTDIR)\moc_qwidget.obj" \
	"$(INTDIR)\moc_qwindow.obj" \
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
	"$(INTDIR)\qgmanagr.obj" \
	"$(INTDIR)\qgrpbox.obj" \
	"$(INTDIR)\qgvector.obj" \
	"$(INTDIR)\qimage.obj" \
	"$(INTDIR)\qiodev.obj" \
	"$(INTDIR)\qlabel.obj" \
	"$(INTDIR)\qlayout.obj" \
	"$(INTDIR)\qlcdnum.obj" \
	"$(INTDIR)\qlined.obj" \
	"$(INTDIR)\qlistbox.obj" \
	"$(INTDIR)\qmenubar.obj" \
	"$(INTDIR)\qmenudta.obj" \
	"$(INTDIR)\qmetaobj.obj" \
	"$(INTDIR)\qmlined.obj" \
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
	"$(INTDIR)\qslider.obj" \
	"$(INTDIR)\qsocknot.obj" \
	"$(INTDIR)\qstring.obj" \
	"$(INTDIR)\qtabbar.obj" \
	"$(INTDIR)\qtabdlg.obj" \
	"$(INTDIR)\qtablevw.obj" \
	"$(INTDIR)\qtimer.obj" \
	"$(INTDIR)\qtooltip.obj" \
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

!ELSEIF  "$(CFG)" == "qt - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\tools\qtstream.cpp
NODEP_CPP_QTSTR=\
	".\tools\qbuffer.h"\
	".\tools\qfile.h"\
	".\tools\qtstream.h"\
	

"$(INTDIR)\qtstream.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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
NODEP_CPP_QGARR=\
	".\tools\qgarray.h"\
	".\tools\qstring.h"\
	

"$(INTDIR)\qgarray.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qgcache.cpp
NODEP_CPP_QGCAC=\
	".\tools\qdict.h"\
	".\tools\qgcache.h"\
	".\tools\qlist.h"\
	".\tools\qstring.h"\
	

"$(INTDIR)\qgcache.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qgdict.cpp
NODEP_CPP_QGDIC=\
	".\tools\qdstream.h"\
	".\tools\qgdict.h"\
	".\tools\qlist.h"\
	".\tools\qstring.h"\
	

"$(INTDIR)\qgdict.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qglist.cpp
NODEP_CPP_QGLIS=\
	".\tools\qdstream.h"\
	".\tools\qglist.h"\
	".\tools\qgvector.h"\
	

"$(INTDIR)\qglist.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qglobal.cpp
NODEP_CPP_QGLOB=\
	".\tools\qdict.h"\
	".\tools\qglobal.h"\
	".\tools\qstring.h"\
	

"$(INTDIR)\qglobal.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qgvector.cpp
NODEP_CPP_QGVEC=\
	".\tools\qdstream.h"\
	".\tools\qglist.h"\
	".\tools\qgvector.h"\
	".\tools\qstring.h"\
	

"$(INTDIR)\qgvector.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qiodev.cpp
NODEP_CPP_QIODE=\
	".\tools\qiodev.h"\
	

"$(INTDIR)\qiodev.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qregexp.cpp
NODEP_CPP_QREGE=\
	".\tools\qregexp.h"\
	

"$(INTDIR)\qregexp.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qstring.cpp
NODEP_CPP_QSTRI=\
	".\tools\qdstream.h"\
	".\tools\qstring.h"\
	

"$(INTDIR)\qstring.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\tools\qbitarry.cpp
NODEP_CPP_QBITA=\
	".\tools\qbitarry.h"\
	".\tools\qdstream.h"\
	

"$(INTDIR)\qbitarry.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qwmatrix.cpp
NODEP_CPP_QWMAT=\
	".\kernel\qdstream.h"\
	".\kernel\qwmatrix.h"\
	

"$(INTDIR)\qwmatrix.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\moc_qapp.cpp
DEP_CPP_MOC_Q=\
	"..\include\qapp.h"\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qapp.obj" : $(SOURCE) $(DEP_CPP_MOC_Q) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\moc_qclipbrd.cpp
DEP_CPP_MOC_QC=\
	"..\include\qarray.h"\
	"..\include\qclipbrd.h"\
	"..\include\qconnect.h"\
	"..\include\qevent.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qclipbrd.obj" : $(SOURCE) $(DEP_CPP_MOC_QC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\moc_qdialog.cpp
DEP_CPP_MOC_QD=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qdialog.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qdialog.obj" : $(SOURCE) $(DEP_CPP_MOC_QD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\moc_qgmanagr.cpp
DEP_CPP_MOC_QG=\
	"..\include\qarray.h"\
	"..\include\qcollect.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgdict.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qgmanagr.h"\
	"..\include\qintdict.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qgmanagr.obj" : $(SOURCE) $(DEP_CPP_MOC_QG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\moc_qsocknot.cpp
DEP_CPP_MOC_QS=\
	"..\include\qarray.h"\
	"..\include\qconnect.h"\
	"..\include\qevent.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qsocknot.h"\
	"..\include\qstring.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qsocknot.obj" : $(SOURCE) $(DEP_CPP_MOC_QS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\moc_qtimer.cpp
DEP_CPP_MOC_QT=\
	"..\include\qarray.h"\
	"..\include\qconnect.h"\
	"..\include\qevent.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qtimer.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qtimer.obj" : $(SOURCE) $(DEP_CPP_MOC_QT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\moc_qwidget.cpp
DEP_CPP_MOC_QW=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qwidget.obj" : $(SOURCE) $(DEP_CPP_MOC_QW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\moc_qwindow.cpp
DEP_CPP_MOC_QWI=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
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
	"..\include\qwindow.h"\
	

"$(INTDIR)\moc_qwindow.obj" : $(SOURCE) $(DEP_CPP_MOC_QWI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qaccel.cpp
NODEP_CPP_QACCE=\
	".\kernel\qaccel.h"\
	".\kernel\qapp.h"\
	".\kernel\qlist.h"\
	".\kernel\qsignal.h"\
	".\kernel\qwidget.h"\
	

"$(INTDIR)\qaccel.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qapp.cpp
NODEP_CPP_QAPP_=\
	".\kernel\qapp.h"\
	".\kernel\qobjcoll.h"\
	".\kernel\qpalette.h"\
	".\kernel\qwidcoll.h"\
	".\kernel\qwidget.h"\
	

"$(INTDIR)\qapp.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qapp_win.cpp
NODEP_CPP_QAPP_W=\
	".\kernel\qapp.h"\
	".\kernel\qdatetm.h"\
	".\kernel\qintdict.h"\
	".\kernel\qkeycode.h"\
	".\kernel\qobjcoll.h"\
	".\kernel\qpainter.h"\
	".\kernel\qpmcache.h"\
	".\kernel\qqueue.h"\
	".\kernel\qvector.h"\
	".\kernel\qwidcoll.h"\
	".\kernel\qwidget.h"\
	

"$(INTDIR)\qapp_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qbitmap.cpp
NODEP_CPP_QBITM=\
	".\kernel\qbitmap.h"\
	".\kernel\qimage.h"\
	

"$(INTDIR)\qbitmap.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qclb_win.cpp
NODEP_CPP_QCLB_=\
	".\kernel\qapp.h"\
	".\kernel\qclipbrd.h"\
	".\kernel\qdatetm.h"\
	".\kernel\qpixmap.h"\
	

"$(INTDIR)\qclb_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qclipbrd.cpp
NODEP_CPP_QCLIP=\
	".\kernel\qapp.h"\
	".\kernel\qclipbrd.h"\
	".\kernel\qpixmap.h"\
	

"$(INTDIR)\qclipbrd.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qcol_win.cpp
NODEP_CPP_QCOL_=\
	".\kernel\qapp.h"\
	".\kernel\qcolor.h"\
	

"$(INTDIR)\qcol_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qcolor.cpp
NODEP_CPP_QCOLO=\
	".\kernel\qcolor.h"\
	".\kernel\qdstream.h"\
	

"$(INTDIR)\qcolor.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qconnect.cpp
NODEP_CPP_QCONN=\
	".\kernel\qconnect.h"\
	

"$(INTDIR)\qconnect.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qcur_win.cpp
NODEP_CPP_QCUR_=\
	".\kernel\qapp.h"\
	".\kernel\qbitmap.h"\
	".\kernel\qcursor.h"\
	".\kernel\qdstream.h"\
	".\kernel\qimage.h"\
	

"$(INTDIR)\qcur_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qcursor.cpp
NODEP_CPP_QCURS=\
	".\kernel\qbitmap.h"\
	".\kernel\qcursor.h"\
	".\kernel\qdstream.h"\
	

"$(INTDIR)\qcursor.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qdialog.cpp
NODEP_CPP_QDIAL=\
	".\kernel\qapp.h"\
	".\kernel\qdialog.h"\
	".\kernel\qkeycode.h"\
	".\kernel\qobjcoll.h"\
	".\kernel\qpushbt.h"\
	

"$(INTDIR)\qdialog.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qdrawutl.cpp
NODEP_CPP_QDRAW=\
	".\kernel\qbitmap.h"\
	".\kernel\qdrawutl.h"\
	".\kernel\qpmcache.h"\
	

"$(INTDIR)\qdrawutl.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qevent.cpp
NODEP_CPP_QEVEN=\
	".\kernel\qevent.h"\
	

"$(INTDIR)\qevent.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qfnt_win.cpp
NODEP_CPP_QFNT_=\
	".\kernel\qcache.h"\
	".\kernel\qdict.h"\
	".\kernel\qfont.h"\
	".\kernel\qfontdta.h"\
	".\kernel\qfontinf.h"\
	".\kernel\qfontmet.h"\
	".\kernel\qpainter.h"\
	".\kernel\qwidget.h"\
	

"$(INTDIR)\qfnt_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qfont.cpp
NODEP_CPP_QFONT=\
	".\kernel\qdict.h"\
	".\kernel\qdstream.h"\
	".\kernel\qfont.h"\
	".\kernel\qfontdta.h"\
	".\kernel\qfontinf.h"\
	".\kernel\qfontmet.h"\
	".\kernel\qpainter.h"\
	".\kernel\qstrlist.h"\
	".\kernel\qwidget.h"\
	

"$(INTDIR)\qfont.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qgmanagr.cpp
NODEP_CPP_QGMAN=\
	".\kernel\qgmanagr.h"\
	".\kernel\qlist.h"\
	

"$(INTDIR)\qgmanagr.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qimage.cpp
NODEP_CPP_QIMAG=\
	".\kernel\qbuffer.h"\
	".\kernel\qdict.h"\
	".\kernel\qdstream.h"\
	".\kernel\qfile.h"\
	".\kernel\qimage.h"\
	".\kernel\qintdict.h"\
	".\kernel\qlist.h"\
	".\kernel\qregexp.h"\
	".\kernel\qtstream.h"\
	

"$(INTDIR)\qimage.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qlayout.cpp
NODEP_CPP_QLAYO=\
	".\kernel\qlayout.h"\
	".\kernel\qmenubar.h"\
	

"$(INTDIR)\qlayout.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qmetaobj.cpp
NODEP_CPP_QMETA=\
	".\kernel\qmetaobj.h"\
	".\kernel\qobjcoll.h"\
	".\kernel\qstrlist.h"\
	

"$(INTDIR)\qmetaobj.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qobject.cpp
NODEP_CPP_QOBJE=\
	".\kernel\qobjcoll.h"\
	".\kernel\qobject.h"\
	".\kernel\qregexp.h"\
	

"$(INTDIR)\qobject.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qpainter.cpp
NODEP_CPP_QPAIN=\
	".\kernel\qbitmap.h"\
	".\kernel\qdstream.h"\
	".\kernel\qpaintdc.h"\
	".\kernel\qpainter.h"\
	".\kernel\qstack.h"\
	

"$(INTDIR)\qpainter.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qpalette.cpp
NODEP_CPP_QPALE=\
	".\kernel\qdstream.h"\
	".\kernel\qpalette.h"\
	

"$(INTDIR)\qpalette.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qpdevmet.cpp
NODEP_CPP_QPDEV=\
	".\kernel\qpdevmet.h"\
	

"$(INTDIR)\qpdevmet.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qpic_win.cpp
NODEP_CPP_QPIC_=\
	".\kernel\qpicture.h"\
	

"$(INTDIR)\qpic_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qpicture.cpp
NODEP_CPP_QPICT=\
	".\kernel\qdstream.h"\
	".\kernel\qfile.h"\
	".\kernel\qpaintdc.h"\
	".\kernel\qpainter.h"\
	".\kernel\qpicture.h"\
	".\kernel\qpixmap.h"\
	

"$(INTDIR)\qpicture.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qpixmap.cpp
NODEP_CPP_QPIXM=\
	".\kernel\qbitmap.h"\
	".\kernel\qbuffer.h"\
	".\kernel\qdstream.h"\
	".\kernel\qimage.h"\
	".\kernel\qpainter.h"\
	".\kernel\qwidget.h"\
	

"$(INTDIR)\qpixmap.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qpm_win.cpp
NODEP_CPP_QPM_W=\
	".\kernel\qapp.h"\
	".\kernel\qbitmap.h"\
	".\kernel\qimage.h"\
	".\kernel\qpaintdc.h"\
	".\kernel\qwmatrix.h"\
	

"$(INTDIR)\qpm_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qpmcache.cpp
NODEP_CPP_QPMCA=\
	".\kernel\qcache.h"\
	".\kernel\qpmcache.h"\
	

"$(INTDIR)\qpmcache.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qpntarry.cpp
NODEP_CPP_QPNTA=\
	".\kernel\qbitarry.h"\
	".\kernel\qdstream.h"\
	".\kernel\qpntarry.h"\
	".\kernel\qrect.h"\
	

"$(INTDIR)\qpntarry.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qpoint.cpp
NODEP_CPP_QPOIN=\
	".\kernel\qdstream.h"\
	".\kernel\qpoint.h"\
	

"$(INTDIR)\qpoint.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qprinter.cpp
NODEP_CPP_QPRIN=\
	".\kernel\qprinter.h"\
	

"$(INTDIR)\qprinter.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qprn_win.cpp
NODEP_CPP_QPRN_=\
	".\kernel\qpaintdc.h"\
	".\kernel\qpainter.h"\
	".\kernel\qpixmap.h"\
	".\kernel\qprinter.h"\
	".\kernel\qwidget.h"\
	

"$(INTDIR)\qprn_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qptd_win.cpp
NODEP_CPP_QPTD_=\
	".\kernel\qapp.h"\
	".\kernel\qbitmap.h"\
	".\kernel\qpaintd.h"\
	".\kernel\qpaintdc.h"\
	".\kernel\qwidget.h"\
	

"$(INTDIR)\qptd_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qptr_win.cpp
NODEP_CPP_QPTR_=\
	".\kernel\qbitmap.h"\
	".\kernel\qintdict.h"\
	".\kernel\qlist.h"\
	".\kernel\qpaintdc.h"\
	".\kernel\qpainter.h"\
	".\kernel\qpmcache.h"\
	".\kernel\qtstream.h"\
	".\kernel\qwidget.h"\
	

"$(INTDIR)\qptr_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qrect.cpp
NODEP_CPP_QRECT=\
	".\kernel\qdstream.h"\
	".\kernel\qrect.h"\
	

"$(INTDIR)\qrect.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qregion.cpp
NODEP_CPP_QREGI=\
	".\kernel\qbuffer.h"\
	".\kernel\qdstream.h"\
	".\kernel\qpntarry.h"\
	".\kernel\qregion.h"\
	

"$(INTDIR)\qregion.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qrgn_win.cpp
NODEP_CPP_QRGN_=\
	".\kernel\qbuffer.h"\
	".\kernel\qpntarry.h"\
	".\kernel\qregion.h"\
	

"$(INTDIR)\qrgn_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qsignal.cpp
NODEP_CPP_QSIGN=\
	".\kernel\qmetaobj.h"\
	".\kernel\qsignal.h"\
	

"$(INTDIR)\qsignal.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qsize.cpp
NODEP_CPP_QSIZE=\
	".\kernel\qdstream.h"\
	".\kernel\qsize.h"\
	

"$(INTDIR)\qsize.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qsocknot.cpp
NODEP_CPP_QSOCK=\
	".\kernel\qevent.h"\
	".\kernel\qsocknot.h"\
	

"$(INTDIR)\qsocknot.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qtimer.cpp
NODEP_CPP_QTIME=\
	".\kernel\qobjcoll.h"\
	".\kernel\qsignal.h"\
	".\kernel\qtimer.h"\
	

"$(INTDIR)\qtimer.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qwid_win.cpp
NODEP_CPP_QWID_=\
	".\kernel\qapp.h"\
	".\kernel\qobjcoll.h"\
	".\kernel\qpaintdc.h"\
	".\kernel\qpainter.h"\
	".\kernel\qpixmap.h"\
	".\kernel\qwidcoll.h"\
	".\kernel\qwindow.h"\
	

"$(INTDIR)\qwid_win.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qwidget.cpp
NODEP_CPP_QWIDG=\
	".\kernel\qapp.h"\
	".\kernel\qkeycode.h"\
	".\kernel\qobjcoll.h"\
	".\kernel\qpixmap.h"\
	".\kernel\qwidcoll.h"\
	".\kernel\qwidget.h"\
	

"$(INTDIR)\qwidget.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\qwindow.cpp
NODEP_CPP_QWIND=\
	".\kernel\qpixmap.h"\
	".\kernel\qwindow.h"\
	

"$(INTDIR)\qwindow.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\kernel\moc_qaccel.cpp
DEP_CPP_MOC_QA=\
	"..\include\qaccel.h"\
	"..\include\qarray.h"\
	"..\include\qconnect.h"\
	"..\include\qevent.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qkeycode.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qaccel.obj" : $(SOURCE) $(DEP_CPP_MOC_QA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qtooltip.cpp
NODEP_CPP_QTOOL=\
	".\widgets\qapp.h"\
	".\widgets\qcolor.h"\
	".\widgets\qlabel.h"\
	".\widgets\qpoint.h"\
	".\widgets\qstring.h"\
	".\widgets\qtooltip.h"\
	".\widgets\qwidget.h"\
	

"$(INTDIR)\qtooltip.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qbutton.cpp
DEP_CPP_MOC_QB=\
	"..\include\qarray.h"\
	"..\include\qbutton.h"\
	"..\include\qcollect.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglist.h"\
	"..\include\qglobal.h"\
	"..\include\qlist.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qbutton.obj" : $(SOURCE) $(DEP_CPP_MOC_QB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qchkbox.cpp
DEP_CPP_MOC_QCH=\
	"..\include\qarray.h"\
	"..\include\qbutton.h"\
	"..\include\qchkbox.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qchkbox.obj" : $(SOURCE) $(DEP_CPP_MOC_QCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qcombo.cpp
DEP_CPP_MOC_QCO=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qcombo.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qcombo.obj" : $(SOURCE) $(DEP_CPP_MOC_QCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qframe.cpp
DEP_CPP_MOC_QF=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qframe.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qframe.obj" : $(SOURCE) $(DEP_CPP_MOC_QF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qgrpbox.cpp
DEP_CPP_MOC_QGR=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qframe.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qgrpbox.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qgrpbox.obj" : $(SOURCE) $(DEP_CPP_MOC_QGR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qlabel.cpp
DEP_CPP_MOC_QL=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qframe.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qlabel.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qlabel.obj" : $(SOURCE) $(DEP_CPP_MOC_QL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qlcdnum.cpp
DEP_CPP_MOC_QLC=\
	"..\include\qarray.h"\
	"..\include\qbitarry.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qframe.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qlcdnum.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qlcdnum.obj" : $(SOURCE) $(DEP_CPP_MOC_QLC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qlined.cpp
DEP_CPP_MOC_QLI=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qlined.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qlined.obj" : $(SOURCE) $(DEP_CPP_MOC_QLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qlistbox.cpp
DEP_CPP_MOC_QLIS=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qframe.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qlistbox.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpixmap.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qtablevw.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qlistbox.obj" : $(SOURCE) $(DEP_CPP_MOC_QLIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qmenubar.cpp
DEP_CPP_MOC_QM=\
	"..\include\qarray.h"\
	"..\include\qcollect.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qframe.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglist.h"\
	"..\include\qglobal.h"\
	"..\include\qlist.h"\
	"..\include\qmenubar.h"\
	"..\include\qmenudta.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpixmap.h"\
	"..\include\qpoint.h"\
	"..\include\qpopmenu.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsignal.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qtablevw.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qmenubar.obj" : $(SOURCE) $(DEP_CPP_MOC_QM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qmlined.cpp
DEP_CPP_MOC_QML=\
	"..\include\qarray.h"\
	"..\include\qcollect.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qframe.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglist.h"\
	"..\include\qglobal.h"\
	"..\include\qlist.h"\
	"..\include\qmetaobj.h"\
	"..\include\qmlined.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qtablevw.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qmlined.obj" : $(SOURCE) $(DEP_CPP_MOC_QML) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qpopmenu.cpp
DEP_CPP_MOC_QP=\
	"..\include\qarray.h"\
	"..\include\qcollect.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qframe.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglist.h"\
	"..\include\qglobal.h"\
	"..\include\qlist.h"\
	"..\include\qmenudta.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpixmap.h"\
	"..\include\qpoint.h"\
	"..\include\qpopmenu.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsignal.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qtablevw.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qpopmenu.obj" : $(SOURCE) $(DEP_CPP_MOC_QP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qpushbt.cpp
DEP_CPP_MOC_QPU=\
	"..\include\qarray.h"\
	"..\include\qbutton.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpoint.h"\
	"..\include\qpushbt.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qpushbt.obj" : $(SOURCE) $(DEP_CPP_MOC_QPU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qradiobt.cpp
DEP_CPP_MOC_QR=\
	"..\include\qarray.h"\
	"..\include\qbutton.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpoint.h"\
	"..\include\qradiobt.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qradiobt.obj" : $(SOURCE) $(DEP_CPP_MOC_QR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qscrbar.cpp
DEP_CPP_MOC_QSC=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpoint.h"\
	"..\include\qrangect.h"\
	"..\include\qrect.h"\
	"..\include\qscrbar.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qscrbar.obj" : $(SOURCE) $(DEP_CPP_MOC_QSC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qslider.cpp
DEP_CPP_MOC_QSL=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpoint.h"\
	"..\include\qrangect.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qslider.h"\
	"..\include\qstring.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qslider.obj" : $(SOURCE) $(DEP_CPP_MOC_QSL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qtabbar.cpp
DEP_CPP_MOC_QTA=\
	"..\include\qarray.h"\
	"..\include\qbrush.h"\
	"..\include\qcollect.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglist.h"\
	"..\include\qglobal.h"\
	"..\include\qlist.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpainter.h"\
	"..\include\qpalette.h"\
	"..\include\qpen.h"\
	"..\include\qpntarry.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qregion.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qtabbar.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	"..\include\qwmatrix.h"\
	

"$(INTDIR)\moc_qtabbar.obj" : $(SOURCE) $(DEP_CPP_MOC_QTA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qtablevw.cpp
DEP_CPP_MOC_QTAB=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qframe.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qtablevw.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qtablevw.obj" : $(SOURCE) $(DEP_CPP_MOC_QTAB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qtooltip.cpp
DEP_CPP_MOC_QTO=\
	"..\include\qarray.h"\
	"..\include\qcollect.h"\
	"..\include\qconnect.h"\
	"..\include\qevent.h"\
	"..\include\qgarray.h"\
	"..\include\qgdict.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qintdict.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qtimer.h"\
	"..\include\qtooltip.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qtooltip.obj" : $(SOURCE) $(DEP_CPP_MOC_QTO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qbttngrp.cpp
NODEP_CPP_QBTTN=\
	".\widgets\qbttngrp.h"\
	".\widgets\qbutton.h"\
	".\widgets\qlist.h"\
	

"$(INTDIR)\qbttngrp.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qbutton.cpp
NODEP_CPP_QBUTT=\
	".\widgets\qbitmap.h"\
	".\widgets\qbttngrp.h"\
	".\widgets\qbutton.h"\
	".\widgets\qkeycode.h"\
	".\widgets\qpainter.h"\
	".\widgets\qtimer.h"\
	

"$(INTDIR)\qbutton.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qchkbox.cpp
NODEP_CPP_QCHKB=\
	".\widgets\qchkbox.h"\
	".\widgets\qdrawutl.h"\
	".\widgets\qpainter.h"\
	".\widgets\qpixmap.h"\
	".\widgets\qpmcache.h"\
	

"$(INTDIR)\qchkbox.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qcombo.cpp
NODEP_CPP_QCOMB=\
	".\widgets\qapp.h"\
	".\widgets\qcombo.h"\
	".\widgets\qdrawutl.h"\
	".\widgets\qkeycode.h"\
	".\widgets\qlined.h"\
	".\widgets\qlistbox.h"\
	".\widgets\qpainter.h"\
	".\widgets\qpixmap.h"\
	".\widgets\qpopmenu.h"\
	".\widgets\qscrbar.h"\
	".\widgets\qstrlist.h"\
	".\widgets\qtimer.h"\
	

"$(INTDIR)\qcombo.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qframe.cpp
NODEP_CPP_QFRAM=\
	".\widgets\qdrawutl.h"\
	".\widgets\qframe.h"\
	".\widgets\qpainter.h"\
	

"$(INTDIR)\qframe.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qgrpbox.cpp
NODEP_CPP_QGRPB=\
	".\widgets\qgrpbox.h"\
	".\widgets\qpainter.h"\
	

"$(INTDIR)\qgrpbox.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qlabel.cpp
NODEP_CPP_QLABE=\
	".\widgets\qaccel.h"\
	".\widgets\qdrawutl.h"\
	".\widgets\qkeycode.h"\
	".\widgets\qlabel.h"\
	".\widgets\qpainter.h"\
	".\widgets\qpixmap.h"\
	

"$(INTDIR)\qlabel.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qlcdnum.cpp
NODEP_CPP_QLCDN=\
	".\widgets\qbitarry.h"\
	".\widgets\qlcdnum.h"\
	".\widgets\qpainter.h"\
	

"$(INTDIR)\qlcdnum.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qlined.cpp
NODEP_CPP_QLINE=\
	".\widgets\qapp.h"\
	".\widgets\qclipbrd.h"\
	".\widgets\qdrawutl.h"\
	".\widgets\qfontmet.h"\
	".\widgets\qkeycode.h"\
	".\widgets\qlined.h"\
	".\widgets\qpainter.h"\
	".\widgets\qpixmap.h"\
	

"$(INTDIR)\qlined.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qlistbox.cpp
NODEP_CPP_QLIST=\
	".\widgets\qapp.h"\
	".\widgets\qfontmet.h"\
	".\widgets\qkeycode.h"\
	".\widgets\qlistbox.h"\
	".\widgets\qpainter.h"\
	".\widgets\qpixmap.h"\
	".\widgets\qstrlist.h"\
	

"$(INTDIR)\qlistbox.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qmenubar.cpp
NODEP_CPP_QMENU=\
	".\widgets\qaccel.h"\
	".\widgets\qapp.h"\
	".\widgets\qdrawutl.h"\
	".\widgets\qmenubar.h"\
	".\widgets\qpainter.h"\
	

"$(INTDIR)\qmenubar.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qmenudta.cpp
NODEP_CPP_QMENUD=\
	".\widgets\qapp.h"\
	".\widgets\qmenudta.h"\
	".\widgets\qpopmenu.h"\
	

"$(INTDIR)\qmenudta.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qmlined.cpp
NODEP_CPP_QMLIN=\
	".\widgets\qapp.h"\
	".\widgets\qclipbrd.h"\
	".\widgets\qkeycode.h"\
	".\widgets\qmlined.h"\
	".\widgets\qpainter.h"\
	".\widgets\qpixmap.h"\
	".\widgets\qscrbar.h"\
	

"$(INTDIR)\qmlined.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qpopmenu.cpp
NODEP_CPP_QPOPM=\
	".\widgets\qaccel.h"\
	".\widgets\qapp.h"\
	".\widgets\qdrawutl.h"\
	".\widgets\qmenubar.h"\
	".\widgets\qpainter.h"\
	".\widgets\qpopmenu.h"\
	".\widgets\qscrbar.h"\
	

"$(INTDIR)\qpopmenu.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qpushbt.cpp
NODEP_CPP_QPUSH=\
	".\widgets\qbitmap.h"\
	".\widgets\qdialog.h"\
	".\widgets\qdrawutl.h"\
	".\widgets\qfontmet.h"\
	".\widgets\qpainter.h"\
	".\widgets\qpixmap.h"\
	".\widgets\qpmcache.h"\
	".\widgets\qpushbt.h"\
	

"$(INTDIR)\qpushbt.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qradiobt.cpp
NODEP_CPP_QRADI=\
	".\widgets\qbitmap.h"\
	".\widgets\qbttngrp.h"\
	".\widgets\qdrawutl.h"\
	".\widgets\qpainter.h"\
	".\widgets\qpixmap.h"\
	".\widgets\qpmcache.h"\
	".\widgets\qradiobt.h"\
	

"$(INTDIR)\qradiobt.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qrangect.cpp
NODEP_CPP_QRANG=\
	".\widgets\qglobal.h"\
	".\widgets\qrangect.h"\
	

"$(INTDIR)\qrangect.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qscrbar.cpp
NODEP_CPP_QSCRB=\
	".\widgets\qbitmap.h"\
	".\widgets\qdrawutl.h"\
	".\widgets\qkeycode.h"\
	".\widgets\qpainter.h"\
	".\widgets\qscrbar.h"\
	

"$(INTDIR)\qscrbar.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qslider.cpp
NODEP_CPP_QSLID=\
	".\widgets\qdrawutl.h"\
	".\widgets\qkeycode.h"\
	".\widgets\qpainter.h"\
	".\widgets\qslider.h"\
	".\widgets\qtimer.h"\
	

"$(INTDIR)\qslider.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qtabbar.cpp
NODEP_CPP_QTABB=\
	".\widgets\qkeycode.h"\
	".\widgets\qtabbar.h"\
	

"$(INTDIR)\qtabbar.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\qtablevw.cpp
NODEP_CPP_QTABL=\
	".\widgets\qdrawutl.h"\
	".\widgets\qpainter.h"\
	".\widgets\qscrbar.h"\
	".\widgets\qtablevw.h"\
	

"$(INTDIR)\qtablevw.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\widgets\moc_qbttngrp.cpp
DEP_CPP_MOC_QBT=\
	"..\include\qarray.h"\
	"..\include\qbttngrp.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qframe.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qgrpbox.h"\
	"..\include\qmetaobj.h"\
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
	

"$(INTDIR)\moc_qbttngrp.obj" : $(SOURCE) $(DEP_CPP_MOC_QBT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dialogs\qtabdlg.cpp
NODEP_CPP_QTABD=\
	".\dialogs\qpainter.h"\
	".\dialogs\qpixmap.h"\
	".\dialogs\qpushbt.h"\
	".\dialogs\qtabbar.h"\
	".\dialogs\qtabdlg.h"\
	

"$(INTDIR)\qtabdlg.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dialogs\moc_qmsgbox.cpp
DEP_CPP_MOC_QMS=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qdialog.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
	"..\include\qmsgbox.h"\
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
	

"$(INTDIR)\moc_qmsgbox.obj" : $(SOURCE) $(DEP_CPP_MOC_QMS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dialogs\moc_qtabdlg.cpp
DEP_CPP_MOC_QTABD=\
	"..\include\qarray.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qdialog.h"\
	"..\include\qevent.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglobal.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qtabdlg.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qtabdlg.obj" : $(SOURCE) $(DEP_CPP_MOC_QTABD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dialogs\qfiledlg.cpp
NODEP_CPP_QFILED=\
	".\dialogs\qapp.h"\
	".\dialogs\qcombo.h"\
	".\dialogs\qfiledlg.h"\
	".\dialogs\qlabel.h"\
	".\dialogs\qlined.h"\
	".\dialogs\qlistbox.h"\
	".\dialogs\qmsgbox.h"\
	".\dialogs\qpushbt.h"\
	

"$(INTDIR)\qfiledlg.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dialogs\qmsgbox.cpp
NODEP_CPP_QMSGB=\
	".\dialogs\qlabel.h"\
	".\dialogs\qmsgbox.h"\
	".\dialogs\qpushbt.h"\
	

"$(INTDIR)\qmsgbox.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\dialogs\moc_qfiledlg.cpp
DEP_CPP_MOC_QFI=\
	"..\include\qarray.h"\
	"..\include\qcollect.h"\
	"..\include\qcolor.h"\
	"..\include\qconnect.h"\
	"..\include\qcursor.h"\
	"..\include\qdatetm.h"\
	"..\include\qdialog.h"\
	"..\include\qdir.h"\
	"..\include\qdstream.h"\
	"..\include\qevent.h"\
	"..\include\qfile.h"\
	"..\include\qfiledlg.h"\
	"..\include\qfileinf.h"\
	"..\include\qfont.h"\
	"..\include\qfontinf.h"\
	"..\include\qfontmet.h"\
	"..\include\qgarray.h"\
	"..\include\qgeneric.h"\
	"..\include\qglist.h"\
	"..\include\qglobal.h"\
	"..\include\qiodev.h"\
	"..\include\qlist.h"\
	"..\include\qmetaobj.h"\
	"..\include\qobjdefs.h"\
	"..\include\qobject.h"\
	"..\include\qpaintd.h"\
	"..\include\qpalette.h"\
	"..\include\qpoint.h"\
	"..\include\qrect.h"\
	"..\include\qshared.h"\
	"..\include\qsize.h"\
	"..\include\qstring.h"\
	"..\include\qstrlist.h"\
	"..\include\qwidget.h"\
	"..\include\qwindefs.h"\
	

"$(INTDIR)\moc_qfiledlg.obj" : $(SOURCE) $(DEP_CPP_MOC_QFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
