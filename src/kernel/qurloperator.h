/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurloperator.h#21 $
**
** Definition of QUrlOperator class
**
** Created : 950429
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QURLOPERATOR_H
#define QURLOPERATOR_H

#ifndef QT_H
#include "qobject.h"
#include "qurl.h"
#include "qlist.h"
#include "qnetworkprotocol.h"
#endif // QT_H

#ifdef QT_FEATURE_NETWORKPROTOCOL

struct QUrlOperatorPrivate;
class QUrlInfo;

class Q_EXPORT QUrlOperator : public QObject, public QUrl
{
    friend class QNetworkProtocol;

    Q_OBJECT

public:
    QUrlOperator();
    QUrlOperator( const QString &urL );
    QUrlOperator( const QUrlOperator& url );
    QUrlOperator( const QUrlOperator& url, const QString& relUrl, bool checkSlash = FALSE );
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
    virtual void copy( const QStringList &files, const QString &dest, bool move = FALSE );
    virtual bool isDir( bool *ok = 0 );

    virtual void setNameFilter( const QString &nameFilter );
    QString nameFilter() const;

    virtual QUrlInfo info( const QString &entry ) const;

    QUrlOperator& operator=( const QUrlOperator &url );
    QUrlOperator& operator=( const QString &url );

    virtual void stop();

signals:
    void newChildren( const QValueList<QUrlInfo> &, QNetworkOperation *res );
    void finished( QNetworkOperation *res );
    void start( QNetworkOperation *res );
    void createdDirectory( const QUrlInfo &, QNetworkOperation *res );
    void removed( QNetworkOperation *res );
    void itemChanged( QNetworkOperation *res );
    void data( const QByteArray &, QNetworkOperation *res );
    void dataTransferProgress( int bytesDone, int bytesTotal, QNetworkOperation *res );
    void startedNextCopy( const QList<QNetworkOperation> &lst );
    void connectionStateChanged( int state, const QString &data );

protected:
    virtual void reset();
    virtual bool parse( const QString& url );
    virtual bool checkValid();
    virtual void clearEntries();
    void getNetworkProtocol();
    void deleteNetworkProtocol();

private slots:
    void copyGotData( const QByteArray &data, QNetworkOperation *op );
    void continueCopy( QNetworkOperation *op );
    void finishedCopy();
    void addEntry( const QValueList<QUrlInfo> &i );

private:
    void deleteOperation( QNetworkOperation *op );

    QUrlOperatorPrivate *d;

};

#endif // QT_FEATURE_NETWORKPROTOCOL

#endif // QURLOPERATOR_H
