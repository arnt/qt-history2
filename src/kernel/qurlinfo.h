/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurlinfo.h#12 $
**
** Implementation of QUrlInfo class
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QURLINFO_H
#define QURLINFO_H

#ifndef QT_H
#include "qdatetime.h"
#include "qstring.h"
#endif // QT_H

class QUrlOperator;
struct QUrlInfoPrivate;
class QUrl;

class Q_EXPORT QUrlInfo
{
public:
    QUrlInfo();
    QUrlInfo( const QUrlOperator &path, const QString &file );
    QUrlInfo( const QUrlInfo &ui );
    QUrlInfo( const QString &name, int permissions, const QString &owner,
	      const QString &group, uint size, const QDateTime &lastModified,
	      const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
	      bool isWritable, bool isReadable, bool isExecutable );
    QUrlInfo( const QUrl &url, int permissions, const QString &owner,
	      const QString &group, uint size, const QDateTime &lastModified,
	      const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
	      bool isWritable, bool isReadable, bool isExecutable );
    QUrlInfo &operator=( const QUrlInfo &ui );
    virtual ~QUrlInfo();

    virtual void setName( const QString &name );
    virtual void setDir( bool b );
    virtual void setFile( bool b );
    virtual void setSymLink( bool b );
    virtual void setOwner( const QString &s );
    virtual void setGroup( const QString &s );
    virtual void setSize( uint s );
    virtual void setWritable( bool b );
    virtual void setReadable( bool b );
    virtual void setPermissions( int p );
    virtual void setLastModified( const QDateTime &dt );

    QString name() const;
    int permissions() const;
    QString owner() const;
    QString group() const;
    uint size() const;
    QDateTime lastModified() const;
    QDateTime lastRead() const;
    bool isDir() const;
    bool isFile() const;
    bool isSymLink() const;
    bool isWritable() const;
    bool isReadable() const;
    bool isExecutable() const;

    static bool greaterThan( const QUrlInfo &i1, const QUrlInfo &i2,
			     int sortBy );
    static bool lessThan( const QUrlInfo &i1, const QUrlInfo &i2,
			  int sortBy );
    static bool equal( const QUrlInfo &i1, const QUrlInfo &i2,
		       int sortBy );

private:
    QUrlInfoPrivate *d;

};

#endif
