/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

/****************************************************************************
**
** ui.h extension file, included from the uic-generated form implementation.
**
** The init() function is used in place of a constructor.
** The destroy() function is used in place of a destructor.
** The slots uploadFile(), downloadFile(), removeFile() and connectToHost() are
** connected with the resp. actions of the GUI.
**
*****************************************************************************/

#include <qftp.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include "connectdialog.h"

void FtpMainWindow::init()
{
    ftp = new QFtp( this );
    connect( ftp, SIGNAL(listInfo(const QUrlInfo &)),
	    SLOT(ftp_listInfo(const QUrlInfo &)) );
}

void FtpMainWindow::destroy()
{
    if ( ftp->state() != QFtp::Unconnected )
	ftp->close();
}

void FtpMainWindow::uploadFile()
{

}

void FtpMainWindow::downloadFile()
{

}

void FtpMainWindow::removeFile()
{

}

void FtpMainWindow::connectToHost()
{
    ConnectDialog connectDialog;
    if ( connectDialog.exec() == QDialog::Rejected )
	return;

    if ( ftp->state() != QFtp::Unconnected )
	ftp->close();

    ftp->connectToHost( connectDialog.host->text(), connectDialog.port->value() );
    ftp->login( connectDialog.username->text(), connectDialog.password->text() );

    remotePath->clear();
    remotePath->insertItem( "/", 0 );
    changePath( "/" );
}

// This slot is connected to the QComboBox::activated() signal of the
// remotePath.
void FtpMainWindow::changePath( const QString &newPath )
{
    remoteView->clear();
    ftp->cd( newPath );
    ftp->list();
}

/****************************************************************************
**
** Slots connected to signals of the QFtp class
**
*****************************************************************************/

void FtpMainWindow::ftp_listInfo( const QUrlInfo &i )
{
    QListViewItem *item = new QListViewItem( remoteView,
	    i.name(),
	    QString::number( i.size() ),
	    i.lastModified().toString() );
    // the pixmaps for folders and files are in an image collection
    if ( i.isDir() )
	item->setPixmap( 0, QPixmap::fromMimeSource( "folder.png" ) );
    else
	item->setPixmap( 0, QPixmap::fromMimeSource( "file.png" ) );
}
