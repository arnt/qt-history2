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
** The slots uploadFile(), downloadFile(), removeFile() and connectToHost() are
** connected with the resp. actions of the GUI.
**
*****************************************************************************/

#include <qftp.h>

#include "connectdialog.h"

void FtpMainWindow::init()
{
    ftp = new QFtp( this );
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
}

void FtpMainWindow::changePath( const QString & )
{

}
