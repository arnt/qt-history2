/****************************************************************************
** $Id: //depot/qt/main/tests/qbrowser/main.cpp#9 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
#include <qnetwork.h>

int main( int argc, char ** argv )
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication a(argc, argv);

    qInitNetworkProtocols();

    QStyleSheetItem* style;

    // Modify the application-wide default style sheet to handle
    // some extra HTML gracefully.
    //
    // Ignore any body text in <head>...</head>:
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

    if (argc > 1)
        home = argv[1];
    else
        home = QString(getenv("QTDIR")) + "/doc/html/index.html";

    ( new HelpWindow(home, ".", 0, "qbrowser") )->show();

    QObject::connect( &a, SIGNAL(lastWindowClosed()),
                      &a, SLOT(quit()) );

    return a.exec();
}
