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
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qprogressdialog.h>

#include "connectdialog.h"
#include "ftpviewitem.h"

void FtpMainWindow::init()
{
    stateFtp = new QLabel( tr("Unconnected"), statusBar() );
    statusBar()->addWidget( stateFtp, 0, TRUE );

    ftp = new QFtp( this );
    connect( ftp, SIGNAL(start(int)),
	    SLOT(ftp_start()) );
    connect( ftp, SIGNAL(doneError(const QString&)),
	    SLOT(ftp_doneError(const QString&)) );
    connect( ftp, SIGNAL(stateChanged(int)),
	    SLOT(ftp_stateChanged(int)) );
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
    QString fileName = QFileDialog::getOpenFileName(
	    QString::null,
	    QString::null,
	    this,
	    "upload file dialog",
	    tr("Choose a file to upload") );
    if ( fileName.isNull() )
	return;

    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
	QMessageBox::critical( this, tr("Upload error"),
		tr("Can't open file '%1' for reading.").arg(fileName) );
	return;
    }
    QFileInfo fi( fileName );
    ftp->put( file.readAll(), fi.fileName() );
    ftp->list();
}

void FtpMainWindow::downloadFile()
{
    FtpViewItem *item = (FtpViewItem*)remoteView->selectedItem();
    if ( !item || item->isDir() )
	return;

    QString fileName = QFileDialog::getSaveFileName(
	    item->text(0),
	    QString::null,
	    this,
	    "download file dialog",
	    tr("Save downloaded file as") );
    if ( fileName.isNull() )
	return;

    // create file on the heap because it has to be valid throughout the whole
    // asynchronous download operation
    QFile *file = new QFile( fileName );
    if ( !file->open( IO_WriteOnly ) ) {
	QMessageBox::critical( this, tr("Download error"),
		tr("Can't open file '%1' for writing.").arg(fileName) );
	delete file;
	return;
    }

    QProgressDialog progress(
	    tr("Downloading file..."),
	    tr("Cancel"),
	    0,
	    this,
	    "download progress dialog",
	    TRUE );

    connect( ftp, SIGNAL(dataSize(int)),
	    &progress, SLOT(setTotalSteps(int)) );
    connect( ftp, SIGNAL(dataProgress(int)),
	    &progress, SLOT(setProgress(int)) );

    connect( ftp, SIGNAL(finishedSuccess(int)),
	    &progress, SLOT(reset()) );
    connect( ftp, SIGNAL(finishedError(int,const QString&)),
	    &progress, SLOT(reset()) );

    connect( &progress, SIGNAL(cancelled()),
	    ftp, SLOT(abort()) );

    ftp->get( item->text(0), file );
    progress.exec(); // ### takes a lot of time!!!
}

void FtpMainWindow::removeFile()
{
    FtpViewItem *item = (FtpViewItem*)remoteView->selectedItem();
    if ( !item || item->isDir() )
	return;

    ftp->remove( item->text(0) );
    ftp->list();
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
    currentFtpDir = newPath;
    ftp->cd( newPath );
    ftp->list();
}

// This slot is connected to the QListView::doubleClicked() and
// QListView::returnPressed() signals of the remoteView.
void FtpMainWindow::changePathOrDownload( QListViewItem *item )
{
    if ( ((FtpViewItem*)item)->isDir() )
	changePath( item->text(0) );
    else
	downloadFile();
}

/****************************************************************************
**
** Slots connected to signals of the QFtp class
**
*****************************************************************************/

void FtpMainWindow::ftp_start()
{
    if ( ftp->currentCommand() == QFtp::List ) {
	remoteView->clear();
	if ( currentFtpDir != "/" )
	    new FtpViewItem( remoteView, FtpViewItem::Directory, "..", "", "" );
    }
}

void FtpMainWindow::ftp_doneError( const QString &msg )
{
    QMessageBox::critical( this, tr("FTP Error"), msg );

    // If we are connected, but not logged in, it is not meaningful to stay
    // connected to the server since the error is a really fatal one (login
    // failed).
    if ( ftp->state() == QFtp::Connected )
	ftp->close();
}

void FtpMainWindow::ftp_stateChanged( int state )
{
    switch ( (QFtp::State)state ) {
	case QFtp::Unconnected:
	    stateFtp->setText( tr("Unconnected") );
	    break;
	case QFtp::HostLookup:
	    stateFtp->setText( tr("Host lookup") );
	    break;
	case QFtp::Connecting:
	    stateFtp->setText( tr("Connecting") );
	    break;
	case QFtp::Connected:
	    stateFtp->setText( tr("Connected") );
	    break;
	case QFtp::LoggedIn:
	    stateFtp->setText( tr("Logged in") );
	    break;
	case QFtp::Closing:
	    stateFtp->setText( tr("Closing") );
	    break;
    }
}

void FtpMainWindow::ftp_listInfo( const QUrlInfo &i )
{
    FtpViewItem::Type type;
    if ( i.isDir() )
	type = FtpViewItem::Directory;
    else
	type = FtpViewItem::File;

    new FtpViewItem( remoteView, type,
	    i.name(), QString::number(i.size()), i.lastModified().toString() );
}
