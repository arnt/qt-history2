/**********************************************************************
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of Qt GUI Designer.
**
** See the file LICENSE included in the distribution for the usage
** and distribution terms.
**
**********************************************************************/

#include <qapplication.h>
#include "spunk.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );
    Spunk * mw = new Spunk;
    mw->show();
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}
