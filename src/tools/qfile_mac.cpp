/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.cpp#61 $
**
** Implementation of QFile class
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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qglobal.h"

#include "qfile.h"
#include "qfiledefs.h"

#define HAS_TEXT_FILEMODE

bool qt_file_access( const QString& fn, int t )
{
    return true;
}

bool QFile::remove( const QString &fileName )
{
    FSSpec myspec;
    char bigbuf[257];
    const char * wingle=fileName.ascii();
    strcpy(bigbuf+1,wingle);
    bigbuf[0]=strlen(wingle);
    OSErr ret;
    ret=FSMakeFSSpec((short)0,(long)0,(const unsigned char *)bigbuf,&myspec);
    if(ret!=noErr) {
        qWarning("Make FS spec in QFile::remove error %d",ret);
        return false;
    }
    ret=FSpDelete(&myspec);
    if(ret==noErr)
	return true;
    return false;
}

bool QFile::open( int m )
{
    return true;
}

bool QFile::open( int m, FILE *f )
{
    return true;
}

bool QFile::open( int m,int f )
{
    return true;
}

uint QFile::size() const
{
    return 0;
}

bool QFile::at( int pos )
{
    return true;
}

int QFile::readBlock( char * p, uint len )
{
    return 0;
}

int QFile::writeBlock( const char * p, uint len )
{
    return 0;
}

int QFile::handle() const
{
    return 0;
}

void QFile::close()
{
}
