/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.h#22 $
**
** Definition of QFileInfo class
**
** Created : 950628
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFILEINFO_H
#define QFILEINFO_H

#ifndef QT_H
#include "qfile.h"
#include "qdatetime.h"
#endif // QT_H

class QDir;
struct QFileInfoCache;


class Q_EXPORT QFileInfo				   // file information class
{
public:
    enum PermissionSpec {
	ReadUser  = 0400, WriteUser  = 0200, ExeUser  = 0100,
	ReadGroup = 0040, WriteGroup = 0020, ExeGroup = 0010,
	ReadOther = 0004, WriteOther = 0002, ExeOther = 0001 };

    QFileInfo();
    QFileInfo( const QString &file );
    QFileInfo( const QFile & );
    QFileInfo( const QDir &, const QString &fileName );
    QFileInfo( const QFileInfo & );
   ~QFileInfo();

    QFileInfo  &operator=( const QFileInfo & );

    void	setFile( const QString &file );
    void	setFile( const QFile & );
    void	setFile( const QDir &, const QString &fileName );

    bool	exists()	const;
    void	refresh()	const;
    bool	caching()	const;
    void	setCaching( bool );

    QString	filePath()	const;
    QString	fileName()	const;
    QString	absFilePath()	const;
    QString	baseName()	const;
    QString	extension( bool complete = TRUE ) const;

    QString	dirPath( bool absPath = FALSE ) const;
    QDir	dir( bool absPath = FALSE )	const;

    bool	isReadable()	const;
    bool	isWritable()	const;
    bool	isExecutable()	const;

    bool	isRelative()	const;
    bool	convertToAbs();

    bool	isFile()	const;
    bool	isDir()		const;
    bool	isSymLink()	const;

    QString	readLink()	const;

    QString	owner()		const;
    uint	ownerId()	const;
    QString	group()		const;
    uint	groupId()	const;

    bool	permission( int permissionSpec ) const;

    uint	size()		const;

    QDateTime	lastModified()	const;
    QDateTime	lastRead()	const;

private:
    void	doStat() const;
    QString	fn;
    QFileInfoCache *fic;
    bool	cache;
};


inline bool QFileInfo::caching() const
{
    return cache;
}


#endif // QFILEINFO_H
