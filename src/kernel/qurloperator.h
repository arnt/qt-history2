/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurloperator.h#1 $
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

#ifndef QURLOPERATOR_H
#define QURLOPERATOR_H

#include "qobject.h"
#include "qurl.h"

struct QUrlOperatorPrivate;
class QUrlInfo;

class QUrlOperator : public QObject,
		     public QUrl
{
    Q_OBJECT
    
public:
    enum Error {
	ErrDeleteFile = -1,
	ErrRenameFile = -2,
	ErrCopyFile = -3,
	ErrReadDir = -4,
	ErrCreateDir = -5,
	ErrUnknownProtocol = -6,
	ErrParse = -7,
	ErrLoginIncorrect = -8,
	ErrHostNotFound = -9,
	ErrValid = -10
    };

    enum Action {
	ActListDirectory = 0,
	ActCopyFiles,
	ActMoveFiles,
	ActGet
    };
    
    QUrlOperator();
    QUrlOperator( const QString &urL );
    QUrlOperator( const QUrlOperator& url );
    QUrlOperator( const QUrlOperator& url, const QString& relUrl_ );
    virtual ~QUrlOperator();
    
    virtual void setPath( const QString& path );
    virtual bool cdUp();

    virtual void listEntries();
    virtual void mkdir( const QString &dirname );
    virtual void remove( const QString &filename );
    virtual void rename( const QString &oldname, const QString &newname );
    virtual void copy( const QString &from, const QString &to );
    virtual void copy( const QStringList &files, const QString &dest, bool move );
    virtual bool isDir();

    virtual void get( const QCString &data );

    virtual void setNameFilter( const QString &nameFilter );
    QString nameFilter() const;

    virtual QUrlInfo info( const QString &entry ) const;

    void emitEntry( const QUrlInfo & );
    void emitFinished( int action );
    void emitStart( int action );
    void emitCreatedDirectory( const QUrlInfo & );
    void emitRemoved( const QString & );
    void emitItemChanged( const QString &oldname, const QString &newname );
    void emitError( int ecode, const QString &msg );
    void emitData( const QCString &d );
    void emitCopyProgress( const QString &from, const QString &to,
			   int step, int total );

    QUrlOperator& operator=( const QUrlOperator &url );

signals:
    void entry( const QUrlInfo & );
    void finished( int );
    void start( int );
    void createdDirectory( const QUrlInfo & );
    void removed( const QString & );
    void itemChanged( const QString &oldname, const QString &newname );
    void error( int ecode, const QString &msg );
    void data( const QCString & );
    void copyProgress( const QString &, const QString &,
		       int step, int total );
protected:
    virtual void reset();
    virtual bool parse( const QString& url );
    virtual bool checkValid();
    virtual void addEntry( const QUrlInfo &i );
    virtual void clearEntries();
    void getNetworkProtocol();
    void deleteNetworkProtocol();

private:
    QUrlOperatorPrivate *d;
    
};

inline void QUrlOperator::emitEntry( const QUrlInfo &i )
{
    addEntry( i );
    emit entry( i );
}

inline void QUrlOperator::emitFinished( int action )
{
    emit finished( action );
    deleteNetworkProtocol();
    getNetworkProtocol();
}

inline void QUrlOperator::emitStart( int action )
{
    emit start( action );
}

inline void QUrlOperator::emitCreatedDirectory( const QUrlInfo &i )
{
    emit createdDirectory( i );
}

inline void QUrlOperator::emitRemoved( const QString &s )
{
    emit removed( s );
}

inline void QUrlOperator::emitItemChanged( const QString &oldname, const QString &newname )
{
    emit itemChanged( oldname, newname );
}

inline void QUrlOperator::emitError( int ecode, const QString &msg )
{
    emit error( ecode, msg );
}

inline void QUrlOperator::emitData( const QCString &d )
{
    emit data( d );
}

inline void QUrlOperator::emitCopyProgress( const QString &from, const QString &to,
				    int step, int total )
{
    emit copyProgress( from, to, step, total );
}

#endif
