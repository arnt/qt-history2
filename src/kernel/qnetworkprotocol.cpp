/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qnetworkprotocol.cpp#3 $
**
** Implementation of QFileDialog class
**
** Created : 950429
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qnetworkprotocol.h"

QMap< QString, QNetworkProtocol* > *qNetworkProtocolRegister = 0;

/*!
  \class QNetworkProtocol qnetworkprotocol.h

  This is a baseclass which should be used for implementations
  of network protocols which can then be used in Qt (e.g.
  in the filedialog).
*/

/*!
  \fn void QNetworkProtocol::finished( int action )

  This signal is emitted when a data transfer of some sort finished.
  \a action gives more information about it, this can be one of
	ActListDirectory
	ActCopyFile
	ActMoveFiles
	ActPut
*/

/*!
  \fn void QNetworkProtocol::start( int action )

  This signal is emitted when a data transfer of some sort started.
  \a action gives more information about it, this can be one of
	ActListDirectory
	ActCopyFile
	ActMoveFiles
	ActPut
*/

/*!
  \fn void QNetworkProtocol::error( int ecode, const QString &msg )

  This signal is emitted whenever an error occures. \a ecode
  is the error code, and \a msg an error message which can be
  e.g. displayed to the user.

  \a ecode is one of
	ErrDeleteFile
	ErrRenameFile
	ErrCopyFile
	ErrReadDir
	ErrCreateDir
	ErrUnknownProtocol
	ErrParseError
*/

/*!
  \fn void QNetworkProtocol::data( const QCString &data )

  This signal is emitted when new \a data has been received.
*/

/*!
  \fn void QUrl::putSuccessful( const QCString &data )

  This signal is emitted after successfully calling put(). \a data is the data
  which has been put.
*/

/*!
  \fn void QNetworkProtocol::connectionStateChanged( int state, const QString &data )

  #### todo
*/

/*!
  #### todo
*/

QNetworkProtocol::QNetworkProtocol()
    : url( 0 )
{
}

/*!
  #### todo
*/

QNetworkProtocol::~QNetworkProtocol()
{
    url = 0;
}

/*!
  Open connection.
*/

void QNetworkProtocol::openConnection( QUrl *u )
{
    setUrl( u );
}

/*!
  #### todo
*/

bool QNetworkProtocol::isOpen()
{
    return FALSE;
}

/*!
  #### todo
*/

void QNetworkProtocol::close()
{
}

/*!
  #### todo
*/

void QNetworkProtocol::setUrl( QUrl *u )
{
    url = u;
}

/*!
  #### todo
*/

void QNetworkProtocol::put( const QCString & )
{
}

/*!
  #### todo
*/

QNetworkProtocol *QNetworkProtocol::copy() const
{
    return new QNetworkProtocol();
}

/*!
  #### todo
*/

void QNetworkProtocol::registerNetworkProtocol( const QString &protocol, QNetworkProtocol *nprotocol )
{
    if ( !qNetworkProtocolRegister )
	qNetworkProtocolRegister = new QMap< QString, QNetworkProtocol* >;

    qNetworkProtocolRegister->insert( protocol, nprotocol );
}

/*!
  #### todo
*/

QNetworkProtocol *QNetworkProtocol::getNetworkProtocol( const QString &protocol )
{
    if ( !qNetworkProtocolRegister )
	qNetworkProtocolRegister = new QMap< QString, QNetworkProtocol* >;

    QMap< QString, QNetworkProtocol*>::Iterator it = qNetworkProtocolRegister->find( protocol );
    if ( it == qNetworkProtocolRegister->end() )
	return 0;

    return ( *it )->copy();
}







/*!
  \class QNetworkFileAccess qnetworkprotocol.h

*/

/*!
  \fn void QNetworkFileAccess::entry( const QUrlInfo &i )

  This signal is emitted after listEntries() was called and
  a new entry (file) has been read from the list of files. \a i
  holds the information about the new etry.
*/

/*!
  \fn void QNetworkFileAccess::createdDirectory( const QUrlInfo &i )

  This signal is emitted when mkdir() has been succesful
  and the directory has been created. \a i holds the information
  about the new directory.
*/

/*!
  \fn void QNetworkFileAccess::removed( const QString &name )

  This signal is emitted when remove() has been succesful
  and the file has been removed. \a name is the filename
  of the removed file.
*/

/*!
  \fn void QNetworkFileAccess::itemChanged( const QString &oldname, const QString &newname )

  This signal is emitted whenever a file, which is a child of this URL,
  has been changed e.g. by successfully calling rename(). \a oldname is
  the original name of the file and \a newname is the name which the file
  go now.
*/

/*!
  \fn void QNetworkFileAccess::urlIsDir()

  When calling isFile() or isDir() and the URL is a dir, this signal
  is emitted.
*/

/*!
  \fn void QNetworkFileAccess::urlIsFile()

  When calling isFile() or isDir() and the URL is a file, this signal
  is emitted.
*/

/*!
  \fn void QNetworkFileAccess::copyProgress( const QString &from, const QString &to, int step, int total )

  When copying a file this signal is emitted. \a from is the file which
  is copied, \a to the destination. \a step is the progress
  (always <= \a total) or -1, if copying just started. \a total is the
  number of steps needed to copy the file.

  This signal can be used to show the progress when copying files.
*/

/*!
  #### todo
*/

QNetworkFileAccess::QNetworkFileAccess()
    : QNetworkProtocol()
{
}


/*!
  #### todo
*/

QNetworkFileAccess::~QNetworkFileAccess()
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::listEntries( const QString &, int, int)
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::mkdir(const QString & )
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::remove( const QString & )
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::rename( const QString &, const QString & )
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::copy( const QStringList &, const QString &, bool )
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::isUrlDir()
{
}

/*!
  #### todo
*/

void QNetworkFileAccess::isUrlFile()
{
}

/*!
  #### todo
*/

QNetworkProtocol *QNetworkFileAccess::copy() const
{
    return new QNetworkFileAccess();
}
