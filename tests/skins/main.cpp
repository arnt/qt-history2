/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qwindowsstyle.h>
#include <qfile.h>
#include <qtextstream.h>
#include "themes.h"
#include "qskin.h"

int main( int argc, char ** argv )
{

    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication a( argc, argv );

    QFile f("example.skin");
    f.open(IO_ReadOnly);
    QTextStream t(&f);
    QApplication::setStyle(new QSkinStyle(t));
    f.close();


    Themes themes;
    themes.setCaption( "Qt Example - Themes (QStyle)" );
    themes.resize( 640, 400 );
    a.setMainWidget( &themes );
    themes.show();

    return a.exec();
}
