/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.cpp#61 $
**
** Implementation of QFile class
**
** Created : 950628
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qglobal.h"
#include "qt_mac.h"
#include "qfile.h"
#include "qfiledefs_p.h"

#define HAS_TEXT_FILEMODE

bool qt_file_access( const QString& fn, int t )
{
    FSSpec myspec;
    char bigbuf[257];
    const char * wingle=fn.ascii();
    qstrcpy(bigbuf+1,wingle);
    bigbuf[0]=qstrlen(wingle);
    OSErr ret;
    ret=FSMakeFSSpec((short)0,(long)0,(const unsigned char *)bigbuf,&myspec);
    if(ret!=noErr) {
        qWarning("Make FS spec in qt_file_access error %d for %s %d",ret,
		 fn.ascii(),t);
        return false;
    }
    // The Mac isn't big on permissions
    return true;
}

bool QFile::remove( const QString &fileName )
{
    FSSpec myspec;
    char bigbuf[257];
    const char * wingle=fileName.ascii();
    qstrcpy(bigbuf+1,wingle);
    bigbuf[0]=qstrlen(wingle);
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
    return false;
}

bool QFile::open( int m, FILE *f )
{
    return false;
}

bool QFile::open( int m,int f )
{
    return false;
}

uint QFile::size() const
{
    const char * thename=name().ascii();
    FILE * f=fopen(thename,"r");
    if(!f) {
	qWarning("Couldn't open %s for QFile::size()\n",thename);
	return 0;
    }
    fseek(f,0,SEEK_END);
    uint ret;
    ret=(uint)ftell(f);
    fclose(f);
    return ret;
}

bool QFile::at( int pos )
{
    if ( !isOpen() ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QFile::at: File is not open" );
#endif
        return FALSE;
    }
    bool ok;
    if ( isRaw() ) {                            // raw file
        //pos = (int)LSEEK(fd, pos, SEEK_SET);
        ok = pos != -1;
    } else {                                    // buffered file
        ok = fseek(fh, pos, SEEK_SET) == 0;
    }
    if ( ok )
        ioIndex = pos;
#if defined(QT_CHECK_RANGE)
    else
        qWarning( "QFile::at: Cannot set file position %d", pos );
#endif
    return ok;
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
    if ( !isOpen() )
        return -1;
    else if ( fh )
	// Hmm
        return (int)fh;
    else
        return fd;
}

void QFile::close()
{
}
