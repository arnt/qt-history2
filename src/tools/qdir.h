/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdir.h#4 $
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

typedef declare(QListM,QFileInfo) QFileInfoList;
typedef declare(QListIteratorM,QFileInfo) QFileInfoIterator;

class QDir
{
public:
    enum FilterSpec { Dirs          = 0x001,
                      Files         = 0x002,
                      Drives        = 0x004,
                      NoSymLinks    = 0x008,
                      All           = 0x007,
                      TypeMask      = 0x00F,

                      Readable      = 0x010, 
                      Writable      = 0x020, 
                      Executable    = 0x040,
                      RWEMask       = 0x070,
                      Modified      = 0x080,
                      Hidden        = 0x100, 
                      System        = 0x200,
                      AccessMask    = 0x3F0,
                      DefaultFilter = -1 };

    enum SortSpec   { Name        = 0x00,
                      Time        = 0x01,
                      Size        = 0x02,
                      Unsorted    = 0x03,
                      SortByMask  = 0x03,
                      DirsFirst   = 0x04,
                      Reversed    = 0x08,
                      IgnoreCase  = 0x10,
                      DefaultSort = -1 };

    QDir();
    QDir( const char *path );
    QDir( const char *path, const char *nameFilt, int sortSpec = Name,
         int filterSpec = All );
    QDir( const QDir & );
   ~QDir();

    void        setPath( const char *relativeOrAbsolutePath );
    const char *path()         const;
    QString     fullPath()     const;
    QString     absolutePath() const;

    QString     dirName() const;
    QString     pathName( const char *fileName ) const;
    QString     fullPathName( const char *fileName ) const;

    bool        cd( const char *dirName);
    bool        cdUp();

    const char *nameFilter() const;
    void        setNameFilter( const char *nameFilter );
    QDir::FilterSpec filter() const;
    void        setFilter( int filterSpec );
    QDir::SortSpec sorting() const;
    void        setSorting( int sortSpec );

    bool        matchAllDirs() const;
    void        setMatchAllDirs( bool );

    const QStrList *entries( int filterSpec = DefaultFilter, 
                             int sortSpec   = DefaultSort  ) const;
    const QStrList *entries( const char *nameFilter,
                             int filterSpec = DefaultFilter, 
                             int sortSpec   = DefaultSort   ) const;

    const QFileInfoList *entryInfos( int filterSpec = DefaultFilter, 
                             int sortSpec   = DefaultSort  ) const;
    const QFileInfoList *entryInfos( const char *nameFilter,
                             int filterSpec = DefaultFilter, 
                             int sortSpec   = DefaultSort   ) const;

    bool        mkdir( const char *dirName );
    bool        rmdir( const char *dirName );

    bool        isReadable() const;
    bool        exists()   const;
    bool	isRoot()   const;

    QDir       &operator=( const QDir & );
    QDir       &operator=( const char * );
    bool        operator==( const QDir & );
    bool        operator!=( const QDir & );

    bool        setToCurrent() const;

    bool        remove( const char *fileName );
    bool        rename( const char *name, const char *newName  );
    bool        exists( const char *name );

    static char separator();

    static bool setCurrent( const char *path );
    static QDir current();
    static QDir home();
    static QDir root();
    static QString currentDirString();
    static QString homeDirString();
    static QString rootDirString();

    static bool match( const char *filter, const char *fileName );
    static QString cleanPathName( const char *pathName );
    static bool isRelativePath( const char *path );

/*    static bool remove( const QDir &, const char *fileName );
    static bool rename( const QDir &,const char *name, const char *newName  );
    static bool exists( const Dir &,const char *name );
*/
private:
    void	init();
    bool        readDirEntries( const QString &nameFilter,
                                int FilterSpec = DefaultFilter, 
                                int SortSpec   = DefaultSort  );

    QString	   dPath;
    QStrList      *fList;
    QFileInfoList *fiList;
    QString        nameFilt;
    uint 	   dirty   : 1;
    uint 	   allDirs : 1;
    uint 	   filtS   : 10;
    uint 	   sortS   : 5;
};

inline const char *QDir::path() const
{
    return dPath.data();
}

inline const char *QDir::nameFilter() const
{
    return (const char *) nameFilt;
}

inline QDir::FilterSpec QDir::filter() const
{
    return (FilterSpec) sortS;
}

inline QDir::SortSpec QDir::sorting() const
{
    return (SortSpec) sortS;
}

inline bool QDir::matchAllDirs() const
{
    return allDirs;
}

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
