/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Linguist.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "trwindow.h"

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qsettings.h>
#include <qsplashscreen.h>
#include <qcursor.h>
#include <qmimefactory.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QApplication::setOverrideCursor( Qt::waitCursor );


    QTranslator translator( 0 );
    translator.load( QString( "linguist_" ) + QTextCodec::locale(), "." );
    app.installTranslator( &translator );

    bool showSplash = TRUE;

    QString keybase("/Qt Linguist/3.1/");
    QSettings config;
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QRect r( QApplication::desktop()->screenGeometry() );
    r.setX( config.readNumEntry( keybase + "Geometry/MainwindowX", r.x() ) );
    r.setY( config.readNumEntry( keybase + "Geometry/MainwindowY", r.y() ) );
    r.setWidth( config.readNumEntry( keybase + "Geometry/MainwindowWidth", r.width() ) );
    r.setHeight( config.readNumEntry( keybase + "Geometry/MainwindowHeight", r.height() ) );

    QSplashScreen *splash = 0;
    if ( showSplash ) {
	splash = new QSplashScreen( qPixmapFromMimeSource("splash.png"),
				    Qt::WDestructiveClose );
	splash->show();
    }

    TrWindow *tw = new TrWindow;
    app.setMainWidget( tw );

    if ( app.argc() > 1 )
	tw->openFile( QString(app.argv()[app.argc() - 1]) );

    if ( config.readBoolEntry( keybase + "Geometry/MainwindowMaximized", FALSE ) )
	tw->showMaximized();
    else
	tw->show();
    if ( splash )
	splash->finish( tw );
    QApplication::restoreOverrideCursor();

    return app.exec();
}
