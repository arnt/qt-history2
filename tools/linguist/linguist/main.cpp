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

    QTranslator translator( 0 );
    translator.load( QString( "linguist_" ) + QTextCodec::locale(), "." );
    app.installTranslator( &translator );

    QTimer timer;
    bool showSplash = TRUE;

     if ( showSplash )
 	timer.start( 1000, TRUE );

    QString   keybase("/Qt Linguist/3.1/");
    QSettings config;
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QRect r( QApplication::desktop()->screenGeometry() );
    r.setX( config.readNumEntry( keybase + "Geometry/MainwindowX", r.x() ) );
    r.setY( config.readNumEntry( keybase + "Geometry/MainwindowY", r.y() ) );
    r.setWidth( config.readNumEntry( keybase + "Geometry/MainwindowWidth", r.width() ) );
    r.setHeight( config.readNumEntry( keybase + "Geometry/MainwindowHeight", r.height() ) );

    QLabel *splash = 0;
    int nscreen = QApplication::desktop()->screenNumber( r.center() );
    QRect screen = QApplication::desktop()->screenGeometry( nscreen );
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
