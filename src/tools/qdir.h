 /****************************************************************************
** $Id: //depot/qt/main/src/tools/qdir.h#1 $
**
** Definition of QDir class
**
** Author  : Eirik Eng
** Created : 950427
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QDIR_H
#define QDIR_H

#include "qstrlist.h"

class QDir
{
public:
    QDir();
    QDir( const char *path );
    QDir( const QDir & );
   ~QDir();

    QString     path()         const;
    QString     absolutePath() const;
    void        setPath( const char *path );

    QString     dirName() const;
    QString     fullName( const char *fileName ) const;

    bool        readFiles( QStrList *, const char *filter = 0,
                           bool includeSymLinks = TRUE ) const;
    bool        readDirs(  QStrList *, const char *filter = 0,
                           bool includeSymLinks = TRUE ) const;
    bool        readAll(   QStrList *, const char *filter = 0,
                           bool includeSymLinks = TRUE ) const;

    bool        cd( const char *dirName);
    bool        cdUp();

    bool        readable() const;
    bool        exists()   const;
    bool	isRoot()   const;

    QDir       &operator=( const QDir & );
    QDir       &operator=( const char * );
    bool        operator==( const QDir & );
    bool        operator!=( const QDir & );

    static char separator();
    static QDir current();
    static QDir home();
    static QDir root();
    static bool match( const char *filter, const char *fileName );
    static QString cleanPathName( const char *pathName );
private:
    bool readDirEntries( QStrList *, bool files, bool dirs,
                      const char *filter, bool includeSymLinks = TRUE ) const;

    QString dPath;
};

inline bool QDir::operator!=( const QDir &d )
{
    return !(*this == d);
}

#if defined( UNIX )
    #define Q_SEPARATOR "/"
#elif defined (_OS_MSDOS_) || defined(_OS_OS2_) || defined(_OS_WINNT_)
    #define Q_SEPARATOR "\\"
#elif defined (_OS_MAC_)
    #define Q_SEPARATOR ":"
#else
    #error "Unknown operating system"
#endif


#endif // QDIR_H


