/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#10 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "view.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qfiledialog.h>

View::View()
    : QVBox()
{
    // setup the GUI
    setSpacing( 5 );
    setMargin( 5 );

    QLabel *l = new QLabel( this );
    l->setAlignment( Qt::WordBreak ),
    l->setText( tr( "The button below opens the QFileDialog and you "
		    "can choose a file then which is downloaded and "
		    "opened below then. You can use for that the <b>local "
		    "filesystem</b> using the file protocol, you can download "
		    "files from an <b>FTP</b> server using the ftp protocol and "
		    "you can download and open <b>USENET</b> articles using the "
		    "demo implementation of the nntp protocol of this "
		    "exmaple (<i>This implementation of the nntp protocol is a very "
		    "basic and incomplete one, so you need to connect to a news server "
		    "which allows reading without authentification</i>)\n"
		    "To open a file from the local filesystem, enter in the "
		    "path combobox of the file dialog a url starting with file "
		    "(like <u>file:/usr/bin</u>), to download something from an FTP "
		    "server, use something like <u>ftp://ftp.troll.no</u> as url, and "
		    "for downloading a news article start with an url like "
		    "<u>nntp://news.tu-graz.ac.at</u> " ) );
    QPushButton *b = new QPushButton( tr( "Open a file..." ), this );
    connect( b, SIGNAL( clicked() ),
	     this, SLOT( downloadFile() ) );

    fileView = new QMultiLineEdit( this );
    fileView->setReadOnly( TRUE );

    // if new data comes in, display it
    connect( &op, SIGNAL( data( const QByteArray &, QNetworkOperation * ) ),
	     this, SLOT( newData( const QByteArray & ) ) );
}

void View::downloadFile()
{
    QString file = QFileDialog::getOpenFileName();
    if ( !file.isEmpty() ) {
	// clear the view
	fileView->clear();
	
	// download the data
	op = file;
	op.get();
    }
}

void View::newData( const QByteArray &ba )
{
    // append new data
    fileView->append( ba );
}
