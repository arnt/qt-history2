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

unsigned short
QFontJis0201Codec::characterFromUnicode(const QString &str, int pos) const
{
    const QChar *c = str.unicode() + pos;
    if ( c->unicode() < 0x80 )
        return c->unicode();
    if ( c->unicode() >= 0xff61 && c->unicode() <= 0xff9f )
        return c->unicode() - 0xff61 + 0xa1;
    return 0;
}

QByteArray QFontJis0201Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QByteArray rstring;
    rstring.resize( lenInOut+1 );
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

void QFontJis0201Codec::fromUnicode(const QChar *in, unsigned short *out, int length) const
{
    while (length--) {
	if ( in->unicode() < 0x80 ) {
	    *out = (uchar) in->unicode();
	} else if ( in->unicode() >= 0xff61 && in->unicode() <= 0xff9f ) {
	    *out = (uchar) (in->unicode() - 0xff61 + 0xa1);
	} else {
	    *out = 0;
	}

	++in;
	++out;
    }
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

unsigned short QFontJis0208Codec::characterFromUnicode(const QString &str, int pos) const
{
    return convJP->unicodeToJisx0208((str.unicode() + pos)->unicode());
}

QByteArray QFontJis0208Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QByteArray result;
    result.resize(lenInOut * 2 + 1);
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

void QFontJis0208Codec::fromUnicode(const QChar *in, unsigned short *out, int length) const
{
    while (length--) {
	*out++ = convJP->unicodeToJisx0208(in->unicode());
	++in;
    }
}


bool QFontJis0208Codec::canEncode( QChar ch ) const
{
    return ( convJP->unicodeToJisx0208(ch.unicode()) != 0 );
}




#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS
