/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "frame.h"
#include "graph.h"
#include "display.h"
#include "icons.h"

#include "textdrawing/textedit.h"
#include "textdrawing/helpwindow.h"
#include "dnd/dnd.h"
#include "i18n/i18n.h"

#include <qmodules.h>

#if defined(QT_MODULE_OPENGL)
#include "opengl/glworkspace.h"
#include "opengl/gllandscapeviewer.h"
#endif

#if defined(QT_MODULE_CANVAS)
#include "qasteroids/toplevel.h"
#endif

#if defined(QT_MODULE_TABLE)
#include "widgets/widgetsbase.h"
#else
#include "widgets/widgetsbase_pro.h"
#endif

#include <stdlib.h>

#include <qapplication.h>
#include <qimage.h>
#include <qwindowsstyle.h>
#include <qtabwidget.h>
#include <qfont.h>
#include <qworkspace.h>

#if defined(QT_MODULE_SQL)
#include <qsqldatabase.h>
#include "sql/book.h"
#include "sql/sqlex.h"
#endif

#ifdef Q_OS_MACX
#include <stdlib.h>
#include <qdir.h>
#endif

QWidget *Frame::createCategory(const QString &cat)
{
    QTabWidget *tab = 0;
    QWidget *w = 0;
    if(cat == "Widgets") {
	tab = new QTabWidget();
	w = new WidgetsBase( tab );
	tab->addTab( w, tr( "Widgets" ) );
	tab->addTab( new DnDDemo, tr( "Drag and Drop" ) );
#if defined(QT_MODULE_SQL)
    } else if(cat == "Database") {
	tab = new QTabWidget();
	w = new BookForm( tab );
	tab->addTab( new SqlEx(), tr( "SQL Explorer" ) );
	tab->addTab( w, tr( "Database" ) );
#endif
#if defined(QT_MODULE_CANVAS)
    } else if(cat == "2D Graphics") {
	tab = new QTabWidget();
	w = new GraphWidget( tab );
	tab->addTab( w, tr( "Graph Drawing" ) );
	tab->addTab( new DisplayWidget(), tr( "Display" ) );
#endif
#if defined(QT_MODULE_OPENGL)
    } else if(cat == "3D Graphics") {
	tab = new QTabWidget();
	w = new GLWorkspace( tab );
	tab->addTab( w, tr( "3d Demo" ) );
	w = new GLLandscapeViewer( tab );
	tab->addTab( w, tr( "Fractal landscape" ) );
#endif
    } else if(cat == "Text Drawing/Editing") {
	tab = new QTabWidget();
	TextEdit *te = new TextEdit( tab );
	te->load( "textdrawing/example.html" );
	te->load( "textdrawing/bidi.txt" );
	w = te;
	tab->addTab( w, tr( "Richtext Editor" ) );
	QString home = QString(getenv("QTDIR")) + "/doc/html/index.html";
	w = new HelpWindow( home, ".", 0, "helpviewer" );
	tab->addTab( w, tr( "Help Browser" ) );
    } else if(cat == "Internationalization") {
	tab = new QTabWidget();
	w = new I18nDemo(tab);
	tab->addTab( w, tr( "Internationalization" ) );
#if defined(QT_MODULE_CANVAS)
    } else if(cat == "Games") {
	tab = new QTabWidget();
	w = new KAstTopLevel(tab);
	tab->addTab( w, tr( "Asteroids" ) );
#endif
    } else {
	qDebug("Unknown category: %s", cat.latin1());
    }
    return tab;
}


int main( int argc, char **argv )
{
    QApplication a( argc, argv );
#ifdef Q_OS_MACX
    setenv("QTDIR", QDir::cleanDirPath(QDir::currentDirPath() + QDir::separator() + ".." + QDir::separator() + ".."), 0);
#endif

    Frame::updateTranslators();

    // #### how about a splash screen?
    Frame frame;

    // example 1
    QPixmap widgetpix( widgeticon );
    QPixmap widgetpix_sel( widgeticon_sel );
    frame.addCategory( NULL, widgetpix, widgetpix_sel, "Widgets" );

#if defined(QT_MODULE_SQL)
    // Database
    QPixmap dbpix( dbicon );
    QPixmap dbpix_sel( dbicon_sel );
    frame.addCategory( NULL, dbpix, dbpix_sel, "Database" );
#endif

#if defined(QT_MODULE_CANVAS)
    // 2D Graphics
    QPixmap twodpix( twodicon );
    QPixmap twodpix_sel( twodicon_sel );
    frame.addCategory( NULL, twodpix, twodpix_sel, "2D Graphics" );
#endif

#if defined(QT_MODULE_OPENGL)
    // 3D Graphics
    QPixmap threedpix( threedicon );
    QPixmap threedpix_sel( threedicon_sel );
    frame.addCategory( NULL, threedpix, threedpix_sel, "3D Graphics" );
#endif

    // example 4
    QPixmap textpix( texticon );
    QPixmap textpix_sel( texticon_sel );
    frame.addCategory( NULL, textpix, textpix_sel, "Text Drawing/Editing" );

    // example 5
    QPixmap internpix( internicon );
    QPixmap internpix_sel( internicon_sel );
    frame.addCategory( NULL, internpix, internpix_sel, "Internationalization");

#if defined(QT_MODULE_CANVAS)
    // example 6
    QPixmap joypix( joyicon );
    QPixmap joypix_sel( joyicon_sel );
    frame.addCategory( NULL, joypix, joypix_sel, "Games" );
#endif

    a.setMainWidget( &frame );
    frame.show();

    return a.exec();
}
