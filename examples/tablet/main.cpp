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

#include "scribble.h"
#include "tabletstats.h"
#include <qapplication.h>
#include <qtabwidget.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    QTabWidget tab;
    Scribble scribble(&tab, "scribble");
    TabletStats tabStats( &tab, "tablet stats" );

    scribble.setMinimumSize( 500, 350 );
    tabStats.setMinimumSize( 500, 350 );
    tab.addTab(&scribble, "Scribble" );
    tab.addTab(&tabStats, "Tablet Stats" );

    a.setMainWidget( &tab );
    tab.show();
    return a.exec();
}
