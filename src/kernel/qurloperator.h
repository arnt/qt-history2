/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurloperator.h#21 $
**
** Implementation of QUrlOperator class
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

#ifndef QT_H
#include "qobject.h"
#include "qurl.h"
#include "qlist.h"
#include "qvaluelist.h"
#include "qnetworkprotocol.h"
#endif // QT_H

struct QUrlOperatorPrivate;
class QUrlInfo;

class Q_EXPORT QUrlOperator : public QObject, public QUrl
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

    virtual const QNetworkOperation *listChildren();
    virtual const QNetworkOperation *mkdir( const QString &dirname );
    virtual const QNetworkOperation *remove( const QString &filename );
    virtual const QNetworkOperation *rename( const QString &oldname, const QString &newname );
    virtual const QNetworkOperation *get( const QString &location = QString::null );
    virtual const QNetworkOperation *put( const QByteArray &data, const QString &location = QString::null  );
    virtual QList<QNetworkOperation> copy( const QString &from, const QString &to, bool move = FALSE );
    virtual QValueList< QList<QNetworkOperation> > copy( const QStringList &files, const QString &dest,
							 bool move = FALSE );
    virtual bool isDir( bool *ok = 0 );

    virtual void setNameFilter( const QString &nameFilter );
    QString nameFilter() const;

    virtual QUrlInfo info( const QString &entry ) const;

    QUrlOperator& operator=( const QUrlOperator &url );
    QUrlOperator& operator=( const QString &url );

public slots:
    void emitNewChild( const QUrlInfo &, QNetworkOperation *res );
    void emitFinished( QNetworkOperation *res );
    void emitStart( QNetworkOperation *res );
    void emitCreatedDirectory( const QUrlInfo &, QNetworkOperation *res );
    void emitRemoved( QNetworkOperation *res );
    void emitItemChanged( QNetworkOperation *res );
    void emitData( const QByteArray &, QNetworkOperation *res );
    void emitDataTransferProgress( int bytesDone, int bytesTotal, QNetworkOperation *res );

signals:
    void newChild( const QUrlInfo &, QNetworkOperation *res );
    void finished( QNetworkOperation *res );
    void start( QNetworkOperation *res );
    void createdDirectory( const QUrlInfo &, QNetworkOperation *res );
    void removed( QNetworkOperation *res );
    void itemChanged( QNetworkOperation *res );
    void data( const QByteArray &, QNetworkOperation *res );
    void dataTransferProgress( int bytesDone, int bytesTotal, QNetworkOperation *res );

protected:
    virtual void reset();
    virtual bool parse( const QString& url );
    virtual bool checkValid();
    virtual void addEntry( const QUrlInfo &i );
    virtual void clearEntries();
    void getNetworkProtocol();
    void deleteNetworkProtocol();

private slots:
    void getGotData( const QByteArray &data, QNetworkOperation *op );
    void finishedGet( QNetworkOperation *op );

private:
    QUrlOperatorPrivate *d;

};

inline void QUrlOperator::emitNewChild( const QUrlInfo &i, QNetworkOperation *res )
{
    addEntry( i );
    emit newChild( i, res );
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

inline void QUrlOperator::emitDataTransferProgress( int bytesDone, int bytesTotal, QNetworkOperation *res )
{
    emit dataTransferProgress( bytesDone, bytesTotal, res );
}

inline void QUrlOperator::emitFinished( QNetworkOperation *res )
{
    emit finished( res );
}


inline void QUrlOperator::emitData( const QByteArray &dat, QNetworkOperation *res )
{
    emit data( dat, res );
}

#endif
