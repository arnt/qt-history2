/****************************************************************************
**
** Definition of QUrl class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QURL_H
#define QURL_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_URL

class QUrlPrivate;

class Q_KERNEL_EXPORT QUrl
{
public:
    QUrl();
    QUrl( const QString& url );
    QUrl( const QUrl& url );
    QUrl( const QUrl& url, const QString& relUrl, bool checkSlash = FALSE );
    virtual ~QUrl();

    QString protocol() const;
    virtual void setProtocol( const QString& protocol );

    QString user() const;
    virtual void setUser( const QString& user );
    bool hasUser() const;

    QString password() const;
    virtual void setPassword( const QString& pass );
    bool hasPassword() const;

    QString host() const;
    virtual void setHost( const QString& user );
    bool hasHost() const;

    int port() const;
    virtual void setPort( int port );
    bool hasPort() const;

    QString path( bool correct = TRUE ) const;
    virtual void setPath( const QString& path );
    bool hasPath() const;

    virtual void setEncodedPathAndQuery( const QString& enc );
    QString encodedPathAndQuery();

    virtual void setQuery( const QString& txt );
    QString query() const;

    QString ref() const;
    virtual void setRef( const QString& txt );
    bool hasRef() const;

    bool isValid() const;
    bool isLocalFile() const;

    virtual void addPath( const QString& path );
    virtual void setFileName( const QString& txt );

    QString fileName() const;
    QString dirPath() const;

    QUrl& operator=( const QUrl& url );
    QUrl& operator=( const QString& url );

    bool operator==( const QUrl& url ) const;
    bool operator==( const QString& url ) const;

    static void decode( QString& url );
    static void encode( QString& url );

    operator QString() const;
    virtual QString toString( bool encodedPath = FALSE, bool forcePrependProtocol = TRUE ) const;

    virtual bool cdUp();

    static bool isRelativeUrl( const QString &url );

protected:
    virtual void reset();
    virtual bool parse( const QString& url );

private:
    QUrlPrivate *d;

};

#endif //QT_NO_URL

#endif
