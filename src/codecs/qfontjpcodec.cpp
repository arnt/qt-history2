/****************************************************************************
** $Id$
**
** Japanese Font utilities for X11
**
** Created : 20010130
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

#include "private/qfontcodecs_p.h"

#ifndef QT_NO_CODECS
#ifndef QT_NO_BIG_CODECS
#include "qjpunicode.h"


// JIS X 0201

QFontJis0201Codec::QFontJis0201Codec()
{
}

const char *QFontJis0201Codec::name() const
{
    return "jisx0201*-0";
}

int QFontJis0201Codec::mibEnum() const
{
    return 15;
}



QCString QFontJis0201Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString rstring( lenInOut+1 );
    uchar *rdata = (uchar *) rstring.data();
    const QChar *sdata = uc.unicode();
    int i = 0;
    for ( ; i < lenInOut; ++i, ++sdata, ++rdata ) {
	if ( sdata->unicode() < 0x80 ) {
	    *rdata = (uchar) sdata->unicode();
	} else if ( sdata->unicode() >= 0xff61 && sdata->unicode() <= 0xff9f ) {
	    *rdata = (uchar) (sdata->unicode() - 0xff61 + 0xa1);
	} else {
	    *rdata = '?';
	}
    }
    *rdata = 0u;
    return rstring;
}

int QFontJis0201Codec::heuristicNameMatch(const char* hint) const
{
    if ( qstrncmp( hint, "jisx0201", 8 ) == 0 )
	return 20;
    return -1;

}

bool QFontJis0201Codec::canEncode( QChar ch ) const
{
    return ( ch.unicode() < 0x80 || ( ch.unicode() >= 0xff61 &&
				      ch.unicode() <= 0xff9f ) );
}

int QFontJis0201Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


// JIS X 0208

int QFontJis0208Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


int QFontJis0208Codec::heuristicNameMatch(const char *hint) const
{
    if ( qstrncmp( hint, "jisx0208.", 9 ) == 0 )
	return 20;
    return -1;
}

QFontJis0208Codec::QFontJis0208Codec()
{
    convJP = QJpUnicodeConv::newConverter(QJpUnicodeConv::Default);
}


QFontJis0208Codec::~QFontJis0208Codec()
{
    delete convJP;
    convJP = 0;
}


const char* QFontJis0208Codec::name() const
{
    return "jisx0208*-0";
}


int QFontJis0208Codec::mibEnum() const
{
    return 63;
}


QString QFontJis0208Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString::null;
}


QCString QFontJis0208Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();

    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	ch = convJP->unicodeToJisx0208(ch.unicode());

	if ( ! ch.isNull() ) {
	    *rdata++ = ch.row();
	    *rdata++ = ch.cell();
	} else {
	    //white square
	    *rdata++ = 0x22;
	    *rdata++ = 0x22;
	}
    }

    lenInOut *= 2;

    return result;
}


bool QFontJis0208Codec::canEncode( QChar ch ) const
{
    return ( convJP->unicodeToJisx0208(ch.unicode()) != 0 );
}




#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS
