/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.h#7 $
**
** Definition of QFileInfo class
**
** Author  : Eirik Eng
** Created : 950628
** 
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFILEINF_H
#define QFILEINF_H

#include "qfile.h"
#include "qdatetm.h"
class QDir;

struct QFileInfoCache;

class QFileInfo                            // file information class
{
public:
    enum PermissionSpec
           { ReadUser  = 0400, WriteUser  = 0200, ExeUser  = 0100,
             ReadGroup = 0040, WriteGroup = 0020, ExeGroup = 0010,
             ReadOther = 0004, WriteOther = 0002, ExeOther = 0001 };


    QFileInfo();
    QFileInfo( const QFileInfo & );
    QFileInfo( const QFile & );
    QFileInfo( const QDir &, const char *fileName );
    QFileInfo( const char *relativeOrAbsFilePath );
   ~QFileInfo();

    QFileInfo &operator=( const QFileInfo & );

    bool    exists()         const;
    void    refresh()        const;
    void    setCaching( bool );
    bool    caching()	     const;

    void setFile( const QFile & );
    void setFile( const QDir &, const char *fileName );
    void setFile( const char *relativeOrAbsFilePath );

    const char *filePath()   const;
    QString fileName()       const;
    QString absFilePath()    const;
    QString baseName()       const;
    QString extension()      const;

    QString dirPath( bool absPath = FALSE )  const;
    QDir    dir( bool absPath = FALSE )      const;

    bool    isReadable()     const;
    bool    isWritable()     const;
    bool    isExecutable()   const;

    bool    isRelative()     const;
    bool    convertToAbs();

    bool    isFile()         const;
    bool    isDir()	     const;
    bool    isSymLink()	     const;

    const char *owner()      const;
    uint        ownerId()    const;
    const char *group()      const;
    uint        groupId()    const;

    bool permission( int permissionSpec ) const;

    long        size()       const;

    QDateTime lastModified() const;
    QDateTime lastRead()     const;

private:
    void doStat() const;
    QString fn;
    QFileInfoCache *fic;
    bool cache;
};

inline bool QFileInfo::caching() const
{
    return cache;
}

#endif // QFILEINF_H
