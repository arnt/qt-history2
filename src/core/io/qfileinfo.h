/****************************************************************************
**
** Definition of QFileInfo class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFILEINFO_H
#define QFILEINFO_H

#ifndef QT_H
#include "qfile.h"
#include "qdatetime.h"
#endif // QT_H


class QDir;
class QFileInfoPrivate;

class Q_CORE_EXPORT QFileInfo
{
public:
    enum PermissionSpec {
	ReadOwner = 04000, WriteOwner = 02000, ExeOwner = 01000,
	ReadUser  = 00400, WriteUser  = 00200, ExeUser  = 00100,
	ReadGroup = 00040, WriteGroup = 00020, ExeGroup = 00010,
	ReadOther = 00004, WriteOther = 00002, ExeOther = 00001 };

    QFileInfo();
    QFileInfo( const QString &file );
    QFileInfo( const QFile & );
#ifndef QT_NO_DIR
    QFileInfo( const QDir &, const QString &fileName );
#endif
    QFileInfo( const QFileInfo & );
    ~QFileInfo();

    QFileInfo  &operator=( const QFileInfo & );

    void	setFile( const QString &file );
    void	setFile( const QFile & );
#ifndef QT_NO_DIR
    void	setFile( const QDir &, const QString &fileName );
#endif
    bool	exists()	const;
    void	refresh()	const;
#ifdef QT_COMPAT
    inline QT_COMPAT bool caching()	const { return true; }
    inline QT_COMPAT void setCaching( bool ) {}
#endif

    QString	filePath()	const;
    QString	fileName()	const;
#ifndef QT_NO_DIR //###
    QString	absFilePath()	const;
#endif
    QString	baseName( bool complete = FALSE ) const;
    QString	extension( bool complete = TRUE ) const;

#ifndef QT_NO_DIR //###
    QString	dirPath( bool absPath = FALSE ) const;
#endif
#ifndef QT_NO_DIR
    QDir	dir( bool absPath = FALSE )	const;
#endif
    bool	isReadable()	const;
    bool	isWritable()	const;
    bool	isExecutable()	const;
    bool 	isHidden()      const;

#ifndef QT_NO_DIR //###
    bool	isRelative()	const;
    bool	convertToAbs();
#endif

    bool	isFile()	const;
    bool	isDir()		const;
    bool	isSymLink()	const;

    QString	readLink()	const;

    QString	owner()		const;
    uint	ownerId()	const;
    QString	group()		const;
    uint	groupId()	const;

    bool	permission( int permissionSpec ) const;

    QIODevice::Offset size()	const;

    QDateTime	created()	const;
    QDateTime	lastModified()	const;
    QDateTime	lastRead()	const;

    void detach();

private:
    QFileInfoPrivate *d;
};

#endif // QFILEINFO_H
