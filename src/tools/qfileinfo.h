/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.h#2 $
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
    QFileInfo( const char *relativeOrAbsoluteFileName );
   ~QFileInfo();

    bool    exists()         const;
    void    refresh()        const;

    void setFile( const QFile & );
    void setFile( const QDir &, const char *fileName );
    void setFile( const char *relativeOrAbsoluteFileName );

    QString name()           const;
    QString fileName()       const;
    QString fullPathName()   const;
    QString baseName()       const;
    QString extension()      const;

    QString dirName( bool fullPath = FALSE )  const;
    QDir    dir( bool fullPath = FALSE )      const;

    bool    isReadable()     const;
    bool    isWritable()     const;
    bool    isExecutable()   const;

    bool    isRelative()     const;

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
    void  init();
    QString fn;
    QFileInfoCache *fic;
};


#endif // QFILEINF_H
