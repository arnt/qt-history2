 /****************************************************************************
** $Id: //depot/qt/main/src/tools/qdir.h#2 $
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
#include "qfileinf.h"

class QDir
{
public:
    enum FilterSpec { Dirs       = 0x001,
                      Files      = 0x002,
                      Drives     = 0x004
                      NoSymLinks = 0x008,
                      All        = 0x007,
                      TypeMask   = 0x00F,

                      Executable = 0x010,
                      Readable   = 0x020, 
                      Writable   = 0x040, 
                      Modified   = 0x080,
                      Hidden     = 0x100, 
                      System     = 0x200,
                      AccessMask = 0x3F0 };

    enum SortSpec   { Name      = 0x01,
                      Time      = 0x02,
                      Size      = 0x03,
                      Unsorted  = 0x04,
                      DirsFirst = 0x08,
                      Reversed  = 0x10 };

    QDir();
    QDir( const char *path );
    QDir( const char *path, const char *nameFilt, SortSpec sort = Name,
         FilterSpec filt = All );
    QDir( const QDir & );
   ~QDir();

    QString     absolutePath() const;
    QString     path()         const;
    void        setPath( const char *path );

    const char *nameFilter() const;
    void        setNameFilter( const char *nameFilter );
    QDir::FilterSpec filter() const;
    void        setFilter( FilterSpec );
    QDir::SortSpec sorting() const;
    void        setSorting( SortSpec );

    QString     dirName() const;
    QString     fullName( const char *fileName ) const;

    const QStrList *list();

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
    static bool mkdir( const QDir &path, const char *dirName );
    static bool mkdir( const char *fullPathDirName );
    static bool rmdir( const QDir &path, const char *dirName );
    static bool rmdir( const char *fullPathDirName );
private:
    bool readDirEntries() const;

    QString dPath;
    QStrList *l;
    uint dirty   : 1;
    uint filt    : 10;
    uint sort    : 5;
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


