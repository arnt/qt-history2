/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtmain_win.cpp#9 $
**
** Implementation of Win32 startup routines.
**
** Created : 980823
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
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
    char* cmdp = 0;
    if ( cmdParam ) {
	cmdp = new char[ qstrlen( cmdParam ) + 1 ];
	qstrcpy( cmdp, cmdParam );
    }
    QArray<pchar> argv( 8 );
    qWinMain( instance, prevInstance, cmdp, cmdShow, argc, argv );
    int result = main( argc, argv.data() );
    if ( cmdp ) delete [] cmdp;
    return result;
}

