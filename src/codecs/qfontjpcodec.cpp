/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontjpcodec.cpp#1 $
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

#include "qfontcodecs_p.h"

#ifndef QT_NO_CODECS
#include <qjpunicode.h>


int QFontJis0208Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
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
    return "jisx0208.1983-0";
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






int QFontJis0212Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


QFontJis0212Codec::QFontJis0212Codec()
{
    convJP = QJpUnicodeConv::newConverter(QJpUnicodeConv::Default);
}


QFontJis0212Codec::~QFontJis0212Codec()
{
    delete convJP;
    convJP = 0;
}


const char* QFontJis0212Codec::name() const
{
    return "jisx0212.1990-0";
}


int QFontJis0212Codec::mibEnum() const
{
    return 98;
}


QString QFontJis0212Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString::null;
}


QCString QFontJis0212Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();

    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	ch = convJP->unicodeToJisx0212(ch.unicode());

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


bool QFontJis0212Codec::canEncode( QChar ch ) const
{
    return ( convJP->unicodeToJisx0212(ch.unicode()) != 0 );
}


#endif // QT_NO_CODECS
