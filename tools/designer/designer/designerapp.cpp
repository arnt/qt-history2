 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "designerappiface.h"
#include "designerapp.h"
#include "mainwindow.h"
#include "formwindow.h"
#include <qfile.h>
#include <qdir.h>
#include <qobjectlist.h>

#ifdef Q_WS_WIN
#include <qt_windows.h>
#include <process.h>
#endif

#if defined(HAVE_KDE)
DesignerApplication::DesignerApplication( int &argc, char **argv, const QCString &rAppName )
    : KApplication( argc, argv, rAppName ), appIface( 0 )
{
}

#else

DesignerApplication::DesignerApplication( int &argc, char **argv )
    : QApplication( argc, argv )
{
#if defined(Q_WS_WIN)
    if ( winVersion() & Qt::WV_NT_based )
	    DESIGNER_OPENFILE = RegisterWindowMessage((TCHAR*)"QT_DESIGNER_OPEN_FILE");
    else
	    DESIGNER_OPENFILE = RegisterWindowMessageA("QT_DESIGNER_OPEN_FILE");
#endif
}

#endif

#if defined(Q_WS_WIN)
#include <qt_windows.h>
#include <process.h>
bool DesignerApplication::winEventFilter( MSG *msg )
{
    if ( msg->message == DESIGNER_OPENFILE ) {
	QFile f( QDir::homeDirPath() + "/.designerargs" );
	QFileInfo fi(f);
	if ( fi.lastModified() == lastMod )
	    return QApplication::winEventFilter( msg );
	lastMod = fi.lastModified();
	f.open( IO_ReadOnly );
	QString args;
	f.readLine( args, f.size() );
	QStringList lst = QStringList::split( " ", args );

	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	    QString arg = (*it).stripWhiteSpace();
	    if ( arg[0] != '-' ) {
		QObjectList* l = MainWindow::self->queryList( "FormWindow" );
		FormWindow* fw = (FormWindow*) l->first();
		FormWindow* totop = 0;
		bool haveit = FALSE;
		while ( fw ) {
		    haveit = haveit || fw->fileName() == arg;
		    if ( haveit )
			totop = fw;

		    fw = (FormWindow*) l->next();
		}
		
		if ( !haveit ) {
		    FlashWindow( MainWindow::self->winId(), TRUE );
		    MainWindow::self->openFile( arg );
		} else if ( totop ) {
		    totop->setFocus();
		}
		delete l;
	    }
	}
	return TRUE;
    }
    return QApplication::winEventFilter( msg );
}
#endif
