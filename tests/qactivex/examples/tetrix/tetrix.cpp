/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/


#include "qtetrix.h"
#include "qdragapp.h"
#include "qfont.h"

int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QDragApplication a(argc,argv);
    if ( !( argc>1 && !qstrcmp(argv[2],"-activex") ) ) {
	QTetrix *tetrix = new QTetrix;
	tetrix->setCaption("Tetrix");
	a.setMainWidget(tetrix);
	tetrix->setCaption("Qt Example - Tetrix");
	tetrix->show();
    }
    return a.exec();
}
