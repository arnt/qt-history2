/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#10 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "ftpmainwindow.h"
#include "ftpview.h"

#include <qvbox.h>
#include <qhbox.h>
#include <qsplitter.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qdir.h>
#include <qlinedialog.h>
#include <qapplication.h>

FtpMainWindow::FtpMainWindow()
    : QMainWindow(),
      localOperator( "/" )
{
    setup();

    connect( &localOperator, SIGNAL( newChild( const QUrlInfo &, QNetworkOperation * ) ),
	     leftView, SLOT( slotInsertEntry( const QUrlInfo & ) ) );
    connect( &localOperator, SIGNAL( start( QNetworkOperation * ) ),
	     this, SLOT( slotLocalStart( QNetworkOperation *) ) );
    connect( &localOperator, SIGNAL( finished( QNetworkOperation * ) ),
	     this, SLOT( slotLocalFinished( QNetworkOperation *) ) );
    connect( leftView, SIGNAL( itemSelected( const QUrlInfo & ) ),
	     this, SLOT( slotLocalDirChanged( const QUrlInfo & ) ) );
    connect( &localOperator, SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ),
             this, SLOT( slotLocalDataTransferProgress( int, int, QNetworkOperation * ) ) );

    connect( &remoteOperator, SIGNAL( newChild( const QUrlInfo &, QNetworkOperation * ) ),
	     rightView, SLOT( slotInsertEntry( const QUrlInfo & ) ) );
    connect( &remoteOperator, SIGNAL( start( QNetworkOperation * ) ),
	     this, SLOT( slotRemoteStart( QNetworkOperation *) ) );
    connect( &remoteOperator, SIGNAL( finished( QNetworkOperation * ) ),
	     this, SLOT( slotRemoteFinished( QNetworkOperation *) ) );
    connect( rightView, SIGNAL( itemSelected( const QUrlInfo & ) ),
	     this, SLOT( slotRemoteDirChanged( const QUrlInfo & ) ) );
    connect( &remoteOperator, SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ),
             this, SLOT( slotRemoteDataTransferProgress( int, int, QNetworkOperation * ) ) );

    localOperator.listChildren();
}

void FtpMainWindow::setupLeftSide()
{
    QVBox *layout = new QVBox( splitter );
    layout->setSpacing( 5 );
    layout->setMargin( 5 );

    QHBox *h = new QHBox( layout );
    h->setSpacing( 5 );
    QLabel *l = new QLabel( tr( "Local Path:" ), h );
    l->setFixedWidth( l->sizeHint().width() );
    localCombo = new QComboBox( TRUE, h );
    localCombo->insertItem( "/" );

    connect( localCombo, SIGNAL( activated( const QString & ) ),
	     this, SLOT( slotLocalDirChanged( const QString & ) ) );

    leftView = new FtpView( layout );

    QHBox *bottom = new QHBox( layout );
    bottom->setSpacing( 5 );
    QPushButton *bMkdir = new QPushButton( tr( "New Directory" ), bottom );
    QPushButton *bRemove = new QPushButton( tr( "Remove" ), bottom );
    connect( bMkdir, SIGNAL( clicked() ),
	     this, SLOT( slotLocalMkdir() ) );
    connect( bRemove, SIGNAL( clicked() ),
	     this, SLOT( slotLocalRemove() ) );

    splitter->setResizeMode( layout, QSplitter::Stretch );
}

void FtpMainWindow::setupRightSide()
{
    QVBox *layout = new QVBox( splitter );
    layout->setSpacing( 5 );
    layout->setMargin( 5 );

    QHBox *h = new QHBox( layout );
    h->setSpacing( 5 );
    QLabel *l = new QLabel( tr( "Remote Host:" ), h );
    l->setFixedWidth( l->sizeHint().width() );
    remoteHostCombo = new QComboBox( TRUE, h );

    l = new QLabel( tr( "Port:" ), h );
    l->setFixedWidth( l->sizeHint().width() );
    portSpin = new QSpinBox( 0, 32767, 1, h );
    portSpin->setValue( 21 );
    portSpin->setFixedWidth( portSpin->sizeHint().width() );

    h = new QHBox( layout );
    h->setSpacing( 5 );
    l = new QLabel( tr( "Remote Path:" ), h );
    l->setFixedWidth( l->sizeHint().width() );
    remotePathCombo = new QComboBox( TRUE, h );

    h = new QHBox( layout );
    h->setSpacing( 5 );
    l = new QLabel( tr( "Username:" ), h );
    l->setFixedWidth( l->sizeHint().width() );
    userCombo = new QComboBox( TRUE, h );

    l = new QLabel( tr( "Password:" ), h );
    l->setFixedWidth( l->sizeHint().width() );
    passLined = new QLineEdit( h );
    passLined->setEchoMode( QLineEdit::Password );

    rightView = new FtpView( layout );

    QHBox *bottom = new QHBox( layout );
    bottom->setSpacing( 5 );
    QPushButton *bMkdir = new QPushButton( tr( "New Directory" ), bottom );
    QPushButton *bRemove = new QPushButton( tr( "Remove" ), bottom );
    connect( bMkdir, SIGNAL( clicked() ),
	     this, SLOT( slotRemoteMkdir() ) );
    connect( bRemove, SIGNAL( clicked() ),
	     this, SLOT( slotRemoteRemove() ) );

    splitter->setResizeMode( layout, QSplitter::Stretch );

    connect( remotePathCombo, SIGNAL( activated( const QString & ) ),
	     this, SLOT( slotRemoteDirChanged( const QString & ) ) );
}

void FtpMainWindow::setupCenterCommandBar()
{
    QVBox *w = new QVBox( splitter );
    splitter->setResizeMode( w, QSplitter::FollowSizeHint );
    w->setSpacing( 5 );
    w->setMargin( 5 );

    QPushButton *bConnect = new QPushButton( tr( "&Connect" ), w );
    (void)new QWidget( w );
    QPushButton *bUpload = new QPushButton( tr( "== &Upload ==>" ), w );
    QPushButton *bDownload = new QPushButton( tr( "<== &Download ==" ), w );
    (void)new QWidget( w );

    connect( bConnect, SIGNAL( clicked() ),
	     this, SLOT( slotConnect() ) );
    connect( bUpload, SIGNAL( clicked() ),
	     this, SLOT( slotUpload() ) );
    connect( bDownload, SIGNAL( clicked() ),
	     this, SLOT( slotDownload() ) );
}

void FtpMainWindow::setup()
{
    mainWidget = new QVBox( this );
    splitter = new QSplitter( mainWidget );
    setupLeftSide();
    setupCenterCommandBar();
    setupRightSide();

    progressLabel1 = new QLabel( tr( "No Operation in Progress" ), mainWidget );
    progressBar1 = new QProgressBar( mainWidget );
    progressLabel2 = new QLabel( tr( "No Operation in Progress" ), mainWidget );
    progressBar2 = new QProgressBar( mainWidget );

    progressLabel1->hide();
    progressBar1->hide();
    progressLabel2->hide();
    progressBar2->hide();

    setCentralWidget( mainWidget );
}

void FtpMainWindow::slotLocalDirChanged( const QString &path )
{
    oldLocal = localOperator;
    localOperator.setPath( path );
    localOperator.listChildren();
}

void FtpMainWindow::slotLocalDirChanged( const QUrlInfo &info )
{
    oldLocal = localOperator;
    localOperator.addPath( info.name() );
    localOperator.listChildren();
    localCombo->insertItem( localOperator.path(), 0 );
    localCombo->setCurrentItem( 0 );
}

void FtpMainWindow::slotRemoteDirChanged( const QString &path )
{
    if ( !remoteOperator.isValid() )
	return;
    oldRemote = remoteOperator;
    remoteOperator.setPath( path );
    remoteOperator.listChildren();
}

void FtpMainWindow::slotRemoteDirChanged( const QUrlInfo &info )
{
    oldRemote = remoteOperator;
    remoteOperator.addPath( info.name() );
    remoteOperator.listChildren();
    remotePathCombo->insertItem( remoteOperator.path(), 0 );
    remotePathCombo->setCurrentItem( 0 );
}

void FtpMainWindow::slotConnect()
{
    QString s = "ftp://" + remoteHostCombo->currentText();
    oldRemote = remoteOperator;
    remoteOperator = s;
    if ( !remotePathCombo->currentText().isEmpty() )
	remoteOperator.setPath( remotePathCombo->currentText() );
    else
	remoteOperator.setPath( "/" );
    if ( !userCombo->currentText().isEmpty() &&
	 userCombo->currentText().lower() != "anonymous" &&
	 userCombo->currentText().lower() != "ftp" ) {
	remoteOperator.setUser( userCombo->currentText() );
	remoteOperator.setPass( passLined->text() );
    }
    remoteOperator.setPort( portSpin->value() );
    remoteOperator.listChildren();
}

void FtpMainWindow::slotUpload()
{
    QValueList<QUrlInfo> files = leftView->selectedItems();
    if ( files.isEmpty() )
	return;

    QStringList lst;
    QValueList<QUrlInfo>::Iterator it = files.begin();
    for ( ; it != files.end(); ++it )
	lst << QUrl( localOperator, ( *it ).name() );
    remoteOperator.copy( lst, remoteOperator, FALSE );
}

void FtpMainWindow::slotDownload()
{
    QValueList<QUrlInfo> files = rightView->selectedItems();
    if ( files.isEmpty() )
	return;

    QStringList lst;
    QValueList<QUrlInfo>::Iterator it = files.begin();
    for ( ; it != files.end(); ++it )
	lst << QUrl( remoteOperator, ( *it ).name() );
    localOperator.copy( lst, localOperator, FALSE );
}

void FtpMainWindow::slotLocalStart( QNetworkOperation *op )
{
    if ( !op )
	return;

    if ( op->operation() == QNetworkProtocol::OpListChildren )
	leftView->clear();
    else if ( op->operation() == QNetworkProtocol::OpGet ) {
	progressBar1->setTotalSteps( 0 );
	progressBar1->reset();
    }
}

void FtpMainWindow::slotLocalFinished( QNetworkOperation *op )
{
    if ( !op )
	return;

    if ( op && op->state() == QNetworkProtocol::StFailed ) {
	QMessageBox::critical( this, tr( "ERROR" ), op->protocolDetail() );

	int ecode = op->errorCode();
	if ( ecode == QNetworkProtocol::ErrListChlidren || ecode == QNetworkProtocol::ErrParse ||
	     ecode == QNetworkProtocol::ErrUnknownProtocol || ecode == QNetworkProtocol::ErrLoginIncorrect ||
	     ecode == QNetworkProtocol::ErrValid || ecode == QNetworkProtocol::ErrHostNotFound ||
	     ecode == QNetworkProtocol::ErrFileNotExisting ) {
	    localOperator = oldLocal;
	    localCombo->setEditText( localOperator.path() );
	    localOperator.listChildren();
	}
    } else if ( op->operation() == QNetworkProtocol::OpPut ) {
	localOperator.listChildren();
	progressLabel1->hide();
	progressBar1->hide();
    } else if ( op->operation() == QNetworkProtocol::OpGet ) {
	progressBar1->setTotalSteps( 0 );
	progressBar1->reset();
    }

}

void FtpMainWindow::slotRemoteStart( QNetworkOperation *op )
{
    if ( !op )
	return;

    if ( op->operation() == QNetworkProtocol::OpListChildren )
	rightView->clear();
    else if ( op->operation() == QNetworkProtocol::OpGet ) {
	progressBar2->setTotalSteps( 0 );
	progressBar2->reset();
    }
}

void FtpMainWindow::slotRemoteFinished( QNetworkOperation *op )
{
    if ( !op )
	return;

    if ( op && op->state() == QNetworkProtocol::StFailed ) {
	QMessageBox::critical( this, tr( "ERROR" ), op->protocolDetail() );

	int ecode = op->errorCode();
	if ( ecode == QNetworkProtocol::ErrListChlidren || ecode == QNetworkProtocol::ErrParse ||
	     ecode == QNetworkProtocol::ErrUnknownProtocol || ecode == QNetworkProtocol::ErrLoginIncorrect ||
	     ecode == QNetworkProtocol::ErrValid || ecode == QNetworkProtocol::ErrHostNotFound ||
	     ecode == QNetworkProtocol::ErrFileNotExisting ) {
	    remoteOperator = oldRemote;
	    remoteHostCombo->setEditText( remoteOperator.host() );
	    remotePathCombo->setEditText( remoteOperator.path() );
	    passLined->setText( remoteOperator.pass() );
	    userCombo->setEditText( remoteOperator.user() );
	    portSpin->setValue( remoteOperator.port() );
	    remoteOperator.listChildren();
	}
    } else if ( op->operation() == QNetworkProtocol::OpListChildren ) {
	remotePathCombo->setEditText( remoteOperator.path() );
    } else if ( op->operation() == QNetworkProtocol::OpPut ) {
	remoteOperator.listChildren();
	progressLabel2->hide();
	progressBar2->hide();
    } else if ( op->operation() == QNetworkProtocol::OpGet ) {
	progressBar2->setTotalSteps( 0 );
	progressBar2->reset();
    }
}

void FtpMainWindow::slotLocalDataTransferProgress( int bytesDone, int bytesTotal,
						   QNetworkOperation *op )
{
    if ( !op )
	return;

    if ( !progressBar1->isVisible() ) {
	if ( bytesDone < bytesTotal) {
	    progressLabel1->show();
	    progressBar1->show();
	    progressBar1->setTotalSteps( bytesTotal );
	    progressBar1->setProgress( 0 );
	    progressBar1->reset();
	} else
	    return;
    }

    if ( progressBar1->totalSteps() == bytesTotal )
	progressBar1->setTotalSteps( bytesTotal );

    if ( op->operation() == QNetworkProtocol::OpGet )
	progressLabel1->setText( tr( "Read: %1" ).arg( op->arg1() ) );
    else if ( op->operation() == QNetworkProtocol::OpPut )
	progressLabel1->setText( tr( "Write: %1" ).arg( op->arg1() ) );
    else
	return;

    progressBar1->setProgress( bytesDone );
}

void FtpMainWindow::slotRemoteDataTransferProgress( int bytesDone, int bytesTotal,
						    QNetworkOperation *op )
{
    if ( !op )
	return;

    if ( !progressBar2->isVisible() ) {
	if ( bytesDone < bytesTotal) {
	    progressLabel2->show();
	    progressBar2->show();
	    progressBar2->setTotalSteps( bytesTotal );
	    progressBar2->setProgress( 0 );
	    progressBar2->reset();
	} else
	    return;
    }

    if ( progressBar2->totalSteps() != bytesTotal )
	progressBar2->setTotalSteps( bytesTotal );

    if ( op->operation() == QNetworkProtocol::OpGet )
	progressLabel2->setText( tr( "Read: %1" ).arg( op->arg1() ) );
    else if ( op->operation() == QNetworkProtocol::OpPut )
	progressLabel2->setText( tr( "Write: %1" ).arg( op->arg1() ) );
    else
	return;

    progressBar2->setProgress( bytesDone );
}

void FtpMainWindow::slotLocalMkdir()
{
    bool ok = FALSE;
    QString name = QLineDialog::getText( tr( "Directory Name:" ), QString::null, &ok, this );

    if ( !name.isEmpty() && ok )
	localOperator.mkdir( name );
}

void FtpMainWindow::slotLocalRemove()
{
}

void FtpMainWindow::slotRemoteMkdir()
{
    bool ok = FALSE;
    QString name = QLineDialog::getText( tr( "Directory Name:" ), QString::null, &ok, this );

    if ( !name.isEmpty() && ok )
	remoteOperator.mkdir( name );
}

void FtpMainWindow::slotRemoteRemove()
{
}
