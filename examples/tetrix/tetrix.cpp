/****************************************************************************
** $Id: //depot/qt/main/examples/tetrix/tetrix.cpp#2 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qtetrix.h"
#include "qdragapp.h"
#include "qfont.h"

int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QDragApplication a(argc,argv);
    QApplication::setFont( QFont( "helvetica", 12 ) );
    QTetrix *tetrix = new QTetrix;
    a.setMainWidget(tetrix);
    tetrix->show();
    return a.exec();
}
