/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Palmtop Environment.
**
** Licensees holding valid Qt Palmtop Developer license may use this 
** file in accordance with the Qt Palmtop Developer License Agreement 
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
** PURPOSE.
**
** email sales@trolltech.com for information about Qt Palmtop License 
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/


#ifndef QRSYNC_H
#define QRSYNC_H

#include <qstring.h>
#include <qglobal.h>
#include <string.h>

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
// #  if defined(QT_NODLL)
//#    undef RSYNC_MAKEDLL
//#    undef RSYNC_DLL
#  if defined(RSYNC_MAKEDLL)	/* create a Qt DLL library */
#    if defined(RSYNC_DLL)
#      undef RSYNC_DLL
#    endif
#    define RSYNC_EXPORT  __declspec(dllexport)
#    define RSYNC_TEMPLATEDLL
#    undef  RSYNC_DISABLE_COPY	/* avoid unresolved externals */
#  elif defined(RSYNC_DLL)		/* use a Qt DLL library */
#    define RSYNC_EXPORT  __declspec(dllimport)
#    define RSYNC_TEMPLATEDLL
#    undef  RSYNC_DISABLE_COPY	/* avoid unresolved externals */
#  endif
#else
#  undef RSYNC_MAKEDLL		/* ignore these for other platforms */
#  undef RSYNC_DLL
#endif

#ifndef RSYNC_EXPORT
#  define RSYNC_EXPORT
#endif

class RSYNC_EXPORT QMd4Sum {
public:
    QMd4Sum() { memset( md4sum.uchars, 0, 16 ); }
    QMd4Sum( QString string );

    const QMd4Sum &operator = ( const QString &s );
public:
    bool operator < ( const QMd4Sum &other ) const {
	bool result = ( md4sum.uints[0] < other.md4sum.uints[0] ? FALSE :
			md4sum.uints[0] != other.md4sum.uints[0] ? TRUE :
		md4sum.uints[1] > other.md4sum.uints[1] ? FALSE :
			md4sum.uints[1] != other.md4sum.uints[1] ? TRUE :
		md4sum.uints[2] > other.md4sum.uints[2] ? FALSE :
			md4sum.uints[2] != other.md4sum.uints[2] ? TRUE :
		md4sum.uints[3] > other.md4sum.uints[3] ? FALSE :
			md4sum.uints[3] != other.md4sum.uints[3] ? TRUE : FALSE );
	    return result;
    }

public:
    QString scrambled() const {
	QString tmp;
	tmp.sprintf( "%08x%08x%08x%08x", md4sum.uints[0]^0x9482, md4sum.uints[1]^0x5c1e, md4sum.uints[2]^0x0ae7, md4sum.uints[3]^2315 );
	return tmp;
    }
    void dump() const {
	qDebug("chksum = %x %x %x %x", md4sum.uints[0], md4sum.uints[1], md4sum.uints[2], md4sum.uints[3] );
    }
private:    
    union {
	unsigned char uchars[16];
	Q_UINT32 uints[4];
    } md4sum;
    friend bool operator == ( const QMd4Sum &s1, const QMd4Sum &s2 );
    friend bool operator != ( const QMd4Sum &s1, const QMd4Sum &s2 );
};

inline bool operator == ( const QMd4Sum &s1, const QMd4Sum &s2 ) {
    return( s1.md4sum.uints[0] == s2.md4sum.uints[0] &&
	    s1.md4sum.uints[1] == s2.md4sum.uints[1] &&
	    s1.md4sum.uints[2] == s2.md4sum.uints[2] &&
	    s1.md4sum.uints[3] == s2.md4sum.uints[3] );	
}
inline bool operator != ( const QMd4Sum &s1, const QMd4Sum &s2 ) {
    return( s1.md4sum.uints[0] != s2.md4sum.uints[0] ||
	    s1.md4sum.uints[1] != s2.md4sum.uints[1] ||
	    s1.md4sum.uints[2] != s2.md4sum.uints[2] ||
	    s1.md4sum.uints[3] != s2.md4sum.uints[3] );	
}


class RSYNC_EXPORT QRsync
{
public:
    static void generateSignature( QString baseFile, QString sigFile );
    static void generateDiff( QString baseFile, QString sigFile, QString diffFile );
    static void applyDiff( QString baseFile, QString diffFile );
};


#endif
