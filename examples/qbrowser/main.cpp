/****************************************************************************
** $Id: //depot/qt/main/examples/qbrowser/main.cpp#3 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qwindowsstyle.h>
#include <qstylesheet.h>
#include "helpwindow.h"
#include <stdlib.h>


int main( int argc, char ** argv )
{
    QApplication::setColorSpec( QApplication::ManyColor );
    //QApplication::setDesktopSettingsAware( FALSE );
    QApplication a(argc, argv);

    QStyleSheetItem* style;

    // Modify the application-wide default style sheet to handle
    // some extra HTML gracefully.
    //
    // Ignore any bodytext in <head>...</head>:
    style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "head" );
    style->setDisplayMode(QStyleSheetItem::DisplayNone);
    //
    // Not in default style sheet, just fake it:
    style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "dl" );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "dt" );
    style->setDisplayMode(QStyleSheetItem::DisplayBlock);
    style->setContexts("dl");
    //
    // Many HTML files omit the </p> or </li>, so we add this for efficiency:
    QStyleSheet::defaultSheet()->item("p")->setSelfNesting( FALSE );
    QStyleSheet::defaultSheet()->item("li")->setSelfNesting( FALSE );

    QString home;

    bool slideshow = FALSE;
    if (argc > 2  && QString("-slideshow") == argv[1] )
	slideshow = TRUE;
    else if (argc > 1)
	home = argv[1];
    else
	home = QString(getenv("QTDIR")) + "/doc/index.html";

    HelpWindow h(home, "");

    if (slideshow)
	h.setupSlideshow( argv[2] );

    a.setMainWidget( &h );
    h.show();

    return a.exec();
}
