/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "private/qfontcodecs_p.h"

#ifndef QT_NO_CODECS
#ifndef QT_NO_BIG_CODECS

extern unsigned int qt_UnicodeToKsc5601(unsigned int unicode);


int QFontKsc5601Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


QFontKsc5601Codec::QFontKsc5601Codec()
{
}


const char* QFontKsc5601Codec::name() const
{
    return "ksc5601.1987-0";
}


int QFontKsc5601Codec::mibEnum() const
{
    return 36;
}


QString QFontKsc5601Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString(); //###
}

unsigned short QFontKsc5601Codec::characterFromUnicode(const QString &str, int pos) const
{
    return qt_UnicodeToKsc5601((str.unicode() + pos)->unicode());
}

QByteArray QFontKsc5601Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QByteArray result;
    result.resize(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();

    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	ch = qt_UnicodeToKsc5601(ch.unicode());

	if ( ! ch.isNull() ) {
	    *rdata++ = ch.row() & 0x7f ;
	    *rdata++ = ch.cell() & 0x7f;
	} else {
	    //white square
	    *rdata++ = 0x21;
	    *rdata++ = 0x60;
	}
    }

    lenInOut *= 2;

    return result;
}

void QFontKsc5601Codec::fromUnicode(const QChar *in, unsigned short *out, int length) const
{
    while (length--) {
	*out++ = (qt_UnicodeToKsc5601(in->unicode()) & 0x7f7f);
	++in;
    }
}

bool QFontKsc5601Codec::canEncode( QChar ch ) const
{
    return (qt_UnicodeToKsc5601(ch.unicode()) != 0);
}

#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS
