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

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    // #### how about a splash screen?

    Frame frame;

    QTabWidget *tab = 0;
    QWidget *w = 0;

    // example 1
    QPixmap widgetpix( widgeticon );
    QPixmap widgetpix_sel( widgeticon_sel );

    tab = new QTabWidget();
    w = new WidgetsBase( tab );
    tab->addTab( w, "Widget" );
    tab->addTab( new DnDDemo, "Drag and Drop" );
    frame.addCategory( tab, widgetpix, widgetpix_sel, "Widgets" );

#if defined(QT_MODULE_SQL)
    // Database
    QPixmap dbpix( dbicon );
    QPixmap dbpix_sel( dbicon_sel );

    tab = new QTabWidget();
    w = new BookForm( tab );
    tab->addTab( new SqlEx(), "SQL Explorer" );
    tab->addTab( w, "Database" );
    frame.addCategory( tab, dbpix, dbpix_sel, "Database" );
#endif

#if defined(QT_MODULE_CANVAS)
    // 2D Graphics
    QPixmap twodpix( twodicon );
    QPixmap twodpix_sel( twodicon_sel );

    tab = new QTabWidget();
    w = new GraphWidget( tab );
    tab->addTab( w, "Graph Drawing" );
    tab->addTab( new DisplayWidget(), "Display" );
    frame.addCategory( tab, twodpix, twodpix_sel, "2D Graphics" );
#endif

#if defined(QT_MODULE_OPENGL)
    // 3D Graphics
    QPixmap threedpix( threedicon );
    QPixmap threedpix_sel( threedicon_sel );

    tab = new QTabWidget();
    w = new GLWorkspace( tab );
    tab->addTab( w, "3d Demo" );
    w = new GLLandscapeViewer( tab );
    tab->addTab( w, "Fractal landscape" );
    frame.addCategory( tab, threedpix, threedpix_sel, "3D Graphics" );
#endif

    // example 4
    QPixmap textpix( texticon );
    QPixmap textpix_sel( texticon_sel );

    tab = new QTabWidget();
    TextEdit *te = new TextEdit( tab );
    te->load( "textdrawing/example.html" );
    te->load( "textdrawing/bidi.txt" );
    w = te;
    tab->addTab( w, "Richtext Editor" );
    QString home = QString(getenv("QTDIR")) + "/doc/html/index.html";
    w = new HelpWindow( home, ".", 0, "helpviewer" );
    tab->addTab( w, "Help Browser" );
    frame.addCategory( tab, textpix, textpix_sel, "Text Drawing/Editing" );

    // example 5
    QPixmap internpix( internicon );
    QPixmap internpix_sel( internicon_sel );

    tab = new QTabWidget();
    w = new I18nDemo(tab);
    tab->addTab(w, "Internationalization");
    frame.addCategory( tab, internpix, internpix_sel, "Internationalization");

#if defined(QT_MODULE_CANVAS)
    // example 6
    QPixmap joypix( joyicon );
    QPixmap joypix_sel( joyicon_sel );

    tab = new QTabWidget();
    w = new KAstTopLevel(tab);
    tab->addTab( w, "Asteroids" );
    frame.addCategory( tab, joypix, joypix_sel, "Games" );
#endif

    a.setMainWidget( &frame );
    frame.show();

    return a.exec();
}
