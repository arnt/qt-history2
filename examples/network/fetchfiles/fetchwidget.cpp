/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qtextview.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qdir.h>
#include <qfiledialog.h>

#include "fetchfiles.h"
#include "fetchwidget.h"

FetchWidget::FetchWidget( QWidget *parent, const char *name ) :
    FetchWidgetBase( parent, name ), ff( 0 )
{
    edPath->setText( "ftp://ftp.trolltech.com/" );
    connect( btnQuit, SIGNAL(clicked()), qApp, SLOT(quit()) );
    connect( btnFetch, SIGNAL(clicked()), this, SLOT(fetch()) );
    btnFetch->setEnabled( TRUE );
    btnStop->setEnabled( FALSE );
    connect( btnBrowse, SIGNAL(clicked()), this, SLOT(getFtpDir()) );
}

void FetchWidget::getFtpDir()
{
    // under Windows you must not use the native file dialog
    QString dir = getSelectedDir();
    if ( !dir.isEmpty() ) {
	edPath->setText( dir );
    }
}

QString FetchWidget::getSelectedDir()
{
    static QString workingDirectory( "ftp://ftp.trolltech.com/" );

    QFileDialog dlg( workingDirectory, QString::null, 0, 0, TRUE );
    dlg.setCaption( QFileDialog::tr( "Select Ftp Directory" ) );
    dlg.setMode( QFileDialog::Directory );
    QString result;
    if ( dlg.exec() == QDialog::Accepted ) {
	result = dlg.selectedFile();
	workingDirectory = dlg.url();
    }
    return result;
}

void FetchWidget::fetch()
{
    delete ff;
    ff = new FetchFiles( this, "fetch files" );
    connect( ff, SIGNAL(start()), this, SLOT(start()) );
    connect( ff, SIGNAL(startFile(const QString&)), this, SLOT(startFile(const QString&)) );
    connect( ff, SIGNAL(finishedFile(const QString&)), this, SLOT(finishedFile(const QString&)) );
    connect( ff, SIGNAL(finished()), this, SLOT(finished()) );
    connect( ff, SIGNAL(error()), this, SLOT(error()) );

    connect( btnStop, SIGNAL(clicked()), ff, SLOT(stop()) );
    btnFetch->setEnabled( FALSE );
    btnStop->setEnabled( TRUE );
    txtOutput->append( tr( "Trying to connect.." ) );

    ff->fetch( edPath->text(), QDir::currentDirPath() + "/download/" );
}

void FetchWidget::start()
{
    txtOutput->append( tr( "Fetching all files from: " ) + edPath->text() );
}

void FetchWidget::startFile( const QString& path )
{
    txtOutput->append( tr( "Fetching file: " ) + path );
}

void FetchWidget::finishedFile( const QString& path )
{
    txtOutput->append( tr( "File %1 copied" ).arg( path ) );
}

void FetchWidget::error()
{
    txtOutput->append( tr( "Error!!!" ) );
}

void FetchWidget::finished()
{
    txtOutput->append( tr( "Finished" ) );
    btnFetch->setEnabled( TRUE );
    btnStop->setEnabled( FALSE );
}
