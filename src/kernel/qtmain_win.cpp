/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtmain_win.cpp#8 $
**
** Implementation of Win32 startup routines.
**
** Created : 980823
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qapplication.h"
#include "qt_windows.h"


/*
  This file contains the code in the qtmain library for Windows.
  qtmain contains the Windows startup code and is required for
  linking to the Qt DLL.

  When a Windows application starts, the WinMain function is
  invoked. WinMain calls qWinMain in the Qt DLL/library, which
  initializes Qt.
*/

extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QArray<pchar> &);

#if defined(NEEDS_QMAIN)
int qMain( int, char ** );
#else
extern "C" int main( int, char ** );
#endif


/*
  WinMain() - Initializes Windows and calls user's startup function main().
  NOTE: WinMain() won't be called if the application was linked as a "console"
  application.
*/

extern "C"
int APIENTRY WinMain( HINSTANCE instance, HINSTANCE prevInstance,
		      LPSTR  cmdParam, int cmdShow )
{
    int argc = 0;
    QArray<pchar> argv( 8 );
    qWinMain( instance, prevInstance, cmdParam, cmdShow, argc, argv );
    return main( argc, argv.data() );
}
