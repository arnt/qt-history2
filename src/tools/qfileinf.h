/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinf.h#1 $
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
class QFile;
class QDir;


class QFileInfo                            // file information class
{
public:
    enum PermissionSpec
           { ReadUser  = 0x001, WriteUser  = 0x002, ExeUser  = 0x004,
             ReadGroup = 0x008, WriteGroup = 0x010, ExeGroup = 0x020,
             ReadOther = 0x040, WriteOther = 0x080, ExeOther = 0x100 };


    QFileInfo();
    QFileInfo( const QFileInfo & );
    QFileInfo( const QFile & );
    QFileInfo( const QDir &, const char *fileName );
    QFileInfo( const char *fullPathfileName );
   ~QFileInfo();

    void  setFile( const char *fullPathfileName );
    void  setFile( const QDir &, const char *fileName );
    const char *fullPathFileName() const;

    bool  exists()	  const;
    bool  isReadable()	  const;
    bool  isWritable()	  const;
    bool  isExecutable()  const;

    bool  isFile()        const;
    bool  isDir()	  const;
    bool  isSymLink()	  const;

    const char *owner()   const;
    uint        ownerId() const;
    const char *group()   const;
    uint        groupId() const;

    bool permission( int permissionSpec ) const;

    long  size() const;			        // get file size

    QDateTime lastModified() const;
    QDateTime lastRead()     const;

private:
    void  init();
    QFile f;
};


#endif // QFILEINF_H
