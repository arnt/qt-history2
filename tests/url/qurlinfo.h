/****************************************************************************
** $Id: //depot/qt/main/tests/url/qurlinfo.h#7 $
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

#ifndef QURLINFO_H
#define QURLINFO_H

#include "qurl.h"
#include <qdatetime.h>
#include <qstring.h>

struct QUrlInfoPrivate;

class QUrlInfo
{
public:
    QUrlInfo();
    QUrlInfo( const QString &name, int permissions, const QString &owner,
	      const QString &group, uint size, const QDateTime &lastModified,
	      const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
	      bool isWritable, bool isReadable, bool isExecutable );
    QUrlInfo( const QUrl &url, int permissions, const QString &owner,
	      const QString &group, uint size, const QDateTime &lastModified,
	      const QDateTime &lastRead, bool isDir, bool isFile, bool isSymLink,
	      bool isWritable, bool isReadable, bool isExecutable );
    QUrlInfo( const QUrl &path, const QString &file );
    QUrlInfo( const QUrlInfo &ui );
    QUrlInfo &operator=( const QUrlInfo &ui );
    ~QUrlInfo();

    void setName( const QString &name );
    void setDir( bool b );
    void setFile( bool b );
    void setOwner( const QString &s );
    void setGroup( const QString &s );
    void setSize( uint s );

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

    QString makeUrl( const QUrl &path, bool withProtocolWhenLocal = FALSE ) const;

private:
    QUrlInfoPrivate *d;

};

#endif
