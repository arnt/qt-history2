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

#include <qaxfactory.h>

QAXFACTORY_DEFAULT( QTetrix, 
	    "{852558AD-CBD6-4f07-844C-D1E8983CD6FC}", 
	    "{2F5D0068-772C-4d1e-BCD2-D3F6BC7FD315}", 
	    "{769F4820-9F28-490f-BA50-5545BD381DCB}",
	    "{5753B1A8-53B9-4abe-8690-6F14EC5CA8D0}",
	    "{DE2F7CE3-CFA7-4938-A9FC-867E2FEB63BA}" )

int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QDragApplication a(argc,argv);
    
    if ( !QAxFactory::isServer() ) {
	QTetrix *tetrix = new QTetrix;
	tetrix->setCaption("Tetrix");
	a.setMainWidget(tetrix);
	tetrix->setCaption("Qt Example - Tetrix");
	tetrix->show();
    }

    return a.exec();
}
