/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   main.cpp
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#include "trwindow.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qsettings.h>

#if defined(_WS_X11_)
extern void qt_wait_for_window_manager( QWidget * );
#endif

#ifdef Q_OS_MACX
#include <stdlib.h>
#include <qdir.h>
#endif

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QApplication::setOverrideCursor( Qt::waitCursor );

#ifdef Q_OS_MACX
    QString qdir = QDir::cleanDirPath(QDir::currentDirPath() + QDir::separator() +
				      ".." + QDir::separator());
    setenv("QTDIR", qdir.latin1(), 0);
#endif

    QTranslator translator( 0 );
    translator.load( QString( "linguist_" ) + QTextCodec::locale(), "." );
    app.installTranslator( &translator );

    QTimer timer;
    bool showSplash = TRUE;

     if ( showSplash )
 	timer.start( 1000, TRUE );

    QLabel *splash = 0;
    QRect screen = QApplication::desktop()->screenGeometry();
    if ( showSplash ) {
	splash = new QLabel( 0, "splash", Qt::WDestructiveClose |
			     Qt::WStyle_Customize | Qt::WStyle_NoBorder |
			     Qt::WX11BypassWM );
	splash->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	splash->setPixmap( TrWindow::splash() );
	splash->adjustSize();
	splash->setCaption( "Qt Linguist" );
	splash->move( screen.center() - QPoint( splash->width() / 2,
						splash->height() / 2 ) );
	splash->show();
	splash->repaint( FALSE );
	QApplication::flushX();

    }

    TrWindow *tw = new TrWindow;
    app.setMainWidget( tw );

    if ( app.argc() > 1 )
	tw->openFile( QString(app.argv()[app.argc() - 1]) );

    QString   keybase("/Qt Linguist/3.0/");
    QSettings config;
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );
    if ( config.readBoolEntry( keybase + "Geometry/MainwindowMaximized", FALSE ) )     
	tw->showMaximized();
    else
	tw->show();
#if defined(_WS_X11_)
    qt_wait_for_window_manager( tw );
#endif
    while ( timer.isActive() ) // evil loop
 	app.processEvents();
    delete splash;
    QApplication::restoreOverrideCursor();

    return app.exec();
}
