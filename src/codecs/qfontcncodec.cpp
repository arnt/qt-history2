/****************************************************************************
** $Id$
**
** Japanese Font utilities for X11
**
** Created : 20010130
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include "private/qfontcodecs_p.h"

#ifndef QT_NO_CODECS
#ifndef QT_NO_BIG_CODECS


extern int qt_UnicodeToGb18030(uint unicode, uchar *gbchar);


int QFontGB2312Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


QFontGB2312Codec::QFontGB2312Codec()
{
}


const char* QFontGB2312Codec::name() const
{
    return "gb2312.1980-0";
}


int QFontGB2312Codec::mibEnum() const
{
    return 57;
}


QString QFontGB2312Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString();
}


QCString QFontGB2312Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();

    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	uchar buf[8];

	int len = qt_UnicodeToGb18030( ch.unicode(), buf );

	if ( len == 2 && buf[0] > 0xa0 && buf[1] > 0xa0  ) {
	    *rdata++ = buf[0] & 0x7f ;
	    *rdata++ = buf[1] & 0x7f;
	} else {
	    //white square
	    *rdata++ = 0x21;
	    *rdata++ = 0x75;
	}
    }

    lenInOut *= 2;

    return result;
}


bool QFontGB2312Codec::canEncode( QChar ch ) const
{
    uchar buf[4];
    int len = qt_UnicodeToGb18030( ch.unicode(), buf );
    return ( len == 2 && buf[0] > 0xa0 && buf[1] > 0xa0  );
}


#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS
