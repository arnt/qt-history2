/****************************************************************************
** $Id: //depot/qt/main/src/tools/qutfcodec.cpp#6 $
**
** Implementation of QEucCodec class
**
** Created : 981015
**
** Copyright (C)1998-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qutfcodec.h"

int QUtf8Codec::mib() const
{
    return 106;
}

QCString QUtf8Codec::fromUnicode(const QString& uc, int& len_in_out) const
{
    int l = QMIN((int)uc.length(),len_in_out);
    int rlen = l*2+1;
    QCString rstr(rlen);
    uchar* cursor = (uchar*)rstr.data();
    for (int i=0; i<l; i++) {
	QChar ch = uc[i];
	if ( !ch.row && ch.cell < 0x80 ) {
	    *cursor++ = ch.cell;
	} else {
	    uchar b = (ch.row << 2) | (ch.cell >> 6);
	    if ( ch.row < 0x08 ) {
		*cursor++ = 0xc0 | b;
	    } else {
		*cursor++ = 0xe0 | (ch.row >> 4);
		*cursor++ = 0x80 | (b&0x3f);
	    }
	    *cursor++ = 0x80 | (ch.cell&0x3f);
	}
    }
    *cursor = 0x00; // NUL terminator
    len_in_out = cursor - (uchar*)rstr.data();
    return rstr;
}

const char* QUtf8Codec::name() const
{
    return "utf8";
}

int QUtf8Codec::heuristicContentMatch(const char* chars, int len) const
{
    int score = 0;
    for (int i=0; i<len; i++) {
	uchar ch = chars[i];
	// No nulls allowed.
	if ( !ch )
	    return -1;
	if ( ch < 128 ) {
	    // Inconclusive
	} else if ( (ch&0xe0) == 0xc0 ) {
	    if ( i < len-1 ) {
		uchar c2 = chars[++i];
		if ( (c2&0xc0) != 0x80 )
		    return -1;
		score+=2;
	    }
	} else if ( (ch&0xf0) == 0xe0 ) {
	    if ( i < len-1 ) {
		uchar c2 = chars[++i];
		if ( (c2&0xc0) != 0x80 ) {
		    return -1;
		    if ( i < len-1 ) {
			uchar c3 = chars[++i];
			if ( (c3&0xc0) != 0x80 )
			    return -1;
			score+=3;
		    }
		}
	    }
	}
    }
    return score;
}




class QUtf8Decoder : public QTextDecoder {
    ushort uc;
    int need;
public:
    QUtf8Decoder() : need(0)
    {
    }

    QString toUnicode(const char* chars, int len)
    {
	QString result;
	for (int i=0; i<len; i++) {
	    uchar ch = chars[i];
	    if (need) {
		if ( (ch&0xc0) == 0x80 ) {
		    uc = (uc << 6) | (ch & 0x3f);
		    need--;
		    if ( !need ) {
			result += QChar(uc);
		    }
		} else {
		    // error
		    result += QChar::replacement;
		    need = 0;
		}
	    } else {
		if ( ch < 128 ) {
		    result += QChar(ch);
		} else if ( (ch&0xe0) == 0xc0 ) {
		    uc = ch &0x1f;
		    need = 1;
		} else if ( (ch&0xf0) == 0xe0 ) {
		    uc = ch &0x0f;
		    need = 2;
		}
	    }
	}
	return result;
    }
};

QTextDecoder* QUtf8Codec::makeDecoder() const
{
    return new QUtf8Decoder;
}






int QUtf16Codec::mib() const
{
    return 1000;
}

const char* QUtf16Codec::name() const
{
    return "utf16";
}

int QUtf16Codec::heuristicContentMatch(const char* chars, int len) const
{
    uchar* uchars = (uchar*)chars;
    if ( len >= 2 && (uchars[0] == 0xff && uchars[1] == 0xfe ||
		      uchars[1] == 0xff && uchars[0] == 0xfe) )
	return 2;
    else
	return 0;
}




class QUtf16Encoder : public QTextEncoder {
    bool headerdone;
public:
    QUtf16Encoder() : headerdone(FALSE)
    {
    }

    QCString fromUnicode(const QString& uc, int& len_in_out)
    {
	if ( headerdone ) {
	    len_in_out = uc.length()*sizeof(QChar);
	    QCString d(len_in_out+1);
	    memcpy(d.data(),uc.unicode(),len_in_out);
	    return d;
	} else {
	    headerdone = TRUE;
	    len_in_out = (1+uc.length())*sizeof(QChar);
	    QCString d(len_in_out+1);
	    memcpy(d.data(),&QChar::byteOrderMark,sizeof(QChar));
	    memcpy(d.data()+sizeof(QChar),uc.unicode(),uc.length()*sizeof(QChar));
	    return d;
	}
    }
};

class QUtf16Decoder : public QTextDecoder {
    uchar buf;
    bool half;
    bool swap;
    bool headerdone;

public:
    QUtf16Decoder() : half(FALSE), swap(FALSE), headerdone(FALSE)
    {
    }

    QString toUnicode(const char* chars, int len)
    {
	QString r;

	while ( len-- ) {
	    if ( half ) {
		QChar ch;
		if ( swap ) {
		    ch.row = *chars++;
		    ch.cell = buf;
		} else {
		    ch.row = buf;
		    ch.cell = *chars++;
		}
		if ( !headerdone ) {
		    if ( ch == QChar::byteOrderSwapped )
			swap = !swap;
		    else if ( ch == QChar::byteOrderMark )
			; // Ignore ZWNBSP
		    else
			r += ch;
		    headerdone = TRUE;
		} else
		    r += ch;
		half = FALSE;
	    } else {
		buf = *chars++;
		half = TRUE;
	    }
	}

	return r;
    }
};

QTextDecoder* QUtf16Codec::makeDecoder() const
{
    return new QUtf16Decoder;
}

QTextEncoder* QUtf16Codec::makeEncoder() const
{
    return new QUtf16Encoder;
}


