/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurloperator.h#2 $
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
#include "qlist.h"

struct QUrlOperatorPrivate;
class QUrlInfo;
class QNetworkOperation;

class QUrlOperator : public QObject,
		     public QUrl
{
    Q_OBJECT

public:
    QUrlOperator();
    QUrlOperator( const QString &urL );
    QUrlOperator( const QUrlOperator& url );
    QUrlOperator( const QUrlOperator& url, const QString& relUrl_ );
    virtual ~QUrlOperator();

    virtual void setPath( const QString& path );
    virtual bool cdUp();

    virtual QNetworkOperation *listChildren();
    virtual QNetworkOperation *mkdir( const QString &dirname );
    virtual QNetworkOperation *remove( const QString &filename );
    virtual QNetworkOperation *rename( const QString &oldname, const QString &newname );
    virtual QNetworkOperation *copy( const QString &from, const QString &to, bool move );
    virtual QList<QNetworkOperation> copy( const QStringList &files, const QString &dest, bool move );
    virtual bool isDir();

    virtual QNetworkOperation *get( const QCString &data );

    virtual void setNameFilter( const QString &nameFilter );
    QString nameFilter() const;

    virtual QUrlInfo info( const QString &entry ) const;

    void emitNewChild( const QUrlInfo &, QNetworkOperation *res );
    void emitFinished( QNetworkOperation *res );
    void emitStart( QNetworkOperation *res );
    void emitCreatedDirectory( const QUrlInfo &, QNetworkOperation *res );
    void emitRemoved( QNetworkOperation *res );
    void emitItemChanged( QNetworkOperation *res );
    void emitData( const QCString &, QNetworkOperation *res );
    void emitCopyProgress( int step, int total, QNetworkOperation *res );

    QUrlOperator& operator=( const QUrlOperator &url );

signals:
    void newChild( const QUrlInfo &, QNetworkOperation *res );
    void finished( QNetworkOperation *res );
    void start( QNetworkOperation *res );
    void createdDirectory( const QUrlInfo &, QNetworkOperation *res );
    void removed( QNetworkOperation *res );
    void itemChanged( QNetworkOperation *res );
    void data( const QCString &, QNetworkOperation *res );
    void copyProgress( int step, int total, QNetworkOperation *res );
    
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

inline void QUrlOperator::emitNewChild( const QUrlInfo &i, QNetworkOperation *res )
{
    addEntry( i );
    emit newChild( i, res );
}

inline void QUrlOperator::emitFinished( QNetworkOperation *res )
{
    emit finished( res );
    deleteNetworkProtocol();
    getNetworkProtocol();
}

inline void QUrlOperator::emitStart( QNetworkOperation *res )
{
    emit start( res );
}

inline void QUrlOperator::emitCreatedDirectory( const QUrlInfo &i, QNetworkOperation *res )
{
    emit createdDirectory( i, res );
}

inline void QUrlOperator::emitRemoved( QNetworkOperation *res )
{
    emit removed( res );
}

inline void QUrlOperator::emitItemChanged( QNetworkOperation *res )
{
    emit itemChanged( res );
}

inline void QUrlOperator::emitData( const QCString &d, QNetworkOperation *res )
{
    emit data( d, res );
}

inline void QUrlOperator::emitCopyProgress( int step, int total, QNetworkOperation *res )
{
    emit copyProgress( step, total, res );
}

#endif
