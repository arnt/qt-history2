/****************************************************************************
** $Id: //depot/qt/main/src/tools/qsjiscodec.cpp#6 $
**
** Implementation of QSjisCodec class
**
** Created : 990225
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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

// Most of the code here was originally written by Serika Kurusugawa
// a.k.a. Junji Takagi, and is include in Qt with the author's permission,
// and the grateful thanks of the Troll Tech team.

/*
 * Copyright (c) 1999 Serika Kurusugawa, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "qsjiscodec.h"

static const uchar Esc = 0x1b;

#define	IsKana(c)	(((c) >= 0xa1) && ((c) <= 0xdf))
#define	IsSjisChar1(c)	((((c) >= 0x81) && ((c) <= 0x9f)) ||	\
			 (((c) >= 0xe0) && ((c) <= 0xfc)))
#define	IsSjisChar2(c)	(((c) >= 0x40) && ((c) != 0x7f) && ((c) <= 0xfc))
#define	IsUserDefinedChar1(c)	(((c) >= 0xf0) && ((c) <= 0xfc))

#define	QValidChar(u)	((u) ? QChar((ushort)(u)) : QChar::replacement)

QSjisCodec::QSjisCodec() : conv(QJpUnicodeConv::newConverter(JU_Default))
{
}

int QSjisCodec::mibEnum() const
{
    /*
    Name: Shift_JIS  (preferred MIME name)
    MIBenum: 17
    Source: A Microsoft code that extends csHalfWidthKatakana to include 
	    kanji by adding a second byte when the value of the first 
	    byte is in the ranges 81-9F or E0-EF.
    Alias: MS_Kanji 
    Alias: csShiftJIS
    */
    return 17;
}

QCString QSjisCodec::fromUnicode(const QString& uc, int& len_in_out) const
{
    int l = QMIN((int)uc.length(),len_in_out);
    int rlen = l*2+1;
    QCString rstr(rlen);
    uchar* cursor = (uchar*)rstr.data();
    for (int i=0; i<l; i++) {
	QChar ch = uc[i];
	uint j;
	if ( ch.row == 0x00 && ch.cell < 0x80 ) {
	    // ASCII
	    *cursor++ = ch.cell;
	} else if ((j = conv->UnicodeToJisx0201(ch.row, ch.cell)) != 0) {
	    // JIS X 0201 Latin or JIS X 0201 Kana
	    *cursor++ = j;
	} else if ((j = conv->UnicodeToSjis(ch.row, ch.cell)) != 0) {
	    // JIS X 0208
	    *cursor++ = (j >> 8);
	    *cursor++ = (j & 0xff);
	} else if ((j = conv->UnicodeToJisx0212(ch.row, ch.cell)) != 0) {
	    // JIS X 0212 (can't be encoded in ShiftJIS !)
	    *cursor++ = 0x81;	// white square
	    *cursor++ = 0xa0;	// white square
	} else {
	    // Error
	    *cursor++ = '?';	// unknown char
	}
    }
    len_in_out = cursor - (uchar*)rstr.data();
    *cursor = 0;
    return rstr;
}

QString QSjisCodec::toUnicode(const char* chars, int len) const
{
    QString result;
    for (int i=0; i<len; i++) {
	uchar ch = chars[i];
	if ( ch < 0x80 || IsKana(ch) ) {
	    // JIS X 0201 Latin or JIS X 0201 Kana
	    uint u = conv->Jisx0201ToUnicode(ch);
	    result += QValidChar(u);
	} else if ( IsSjisChar1(ch) ) {
	    // JIS X 0208
	    if ( i < len-1 ) {
		uchar c2 = chars[++i];
		if ( IsSjisChar2(c2) ) {
		    if ( IsUserDefinedChar1(ch) ) {
			result += QChar::replacement;
		    } else {
			uint u = conv->SjisToUnicode(ch, c2);
			result += QValidChar(u);
		    }
		} else {
		    i--;
		    result += QChar::replacement;
		}
	    } else {
		result += QChar::replacement;
	    }
	} else {
	    result += QChar::replacement;
	}
    }
    return result;
}

const char* QSjisCodec::name() const
{
    return "SJIS";
}

int QSjisCodec::heuristicNameMatch(const char* hint) const
{
    int score = 0;
    bool ja = FALSE;
    if (strnicmp(hint, "ja_JP", 5) == 0 || strnicmp(hint, "japan", 5) == 0) {
	score += 3; 
	ja = TRUE;
    } else if (strnicmp(hint, "ja", 2) == 0) {
	score += 2; 
	ja = TRUE;
    }
    const char *p = 0;
    if (ja) {
	p = strchr(hint, '.');
	if (p == 0) {
	    return score - 1;
	}
	p++;
    } else {
	p = hint;
    }
    if (p) {
	if ((stricmp(p, "mscode") == 0) ||
	    (stricmp(p, "PCK") == 0) ||
	    (stricmp(p, "SJIS") == 0) ||
	    (stricmp(p, "ShiftJIS") == 0)) {
	    return score + 4;
	}
    }
    return QTextCodec::heuristicNameMatch(hint);
}

int QSjisCodec::heuristicContentMatch(const char* chars, int len) const
{
    int score = 0;
    for (int i=0; i<len; i++) {
	uchar ch = chars[i];
	// No nulls allowed.
	if ( !ch || ch == Esc )
	    return -1;
	if ( ch < 32 && ch != '\t' && ch != '\n' && ch != '\r' ) {
	    // Suspicious
	    if ( score )
		score--;
	} else if ( ch < 0x80 ) {
	    // Inconclusive
	} else if ( IsKana(ch) ) {
	    // JIS X 0201 Kana
	    score++;
	} else if ( IsSjisChar1(ch) ) {
	    // JIS X 0208-1990
	    if ( i < len-1 ) {
		uchar c2 = chars[++i];
		if ( !IsSjisChar2(c2) )
		    return -1;
		score++;
	    }
	    score++;
	} else {
	    // Invalid
	    return -1;
	}
    }
    return score;
}

class QSjisDecoder : public QTextDecoder {
    uchar buf[1];
    int nbuf;
    const QJpUnicodeConv * const conv;
public:
    QSjisDecoder(const QJpUnicodeConv *c) : nbuf(0), conv(c)
    {
    }

    QString toUnicode(const char* chars, int len)
    {
	QString result;
	for (int i=0; i<len; i++) {
	    uchar ch = chars[i];
	    switch (nbuf) {
	      case 0:
		if ( ch < 0x80 || IsKana(ch) ) {
		    // JIS X 0201 Latin or JIS X 0201 Kana
		    uint u = conv->Jisx0201ToUnicode(ch);
		    result += QValidChar(u);
		} else if ( IsSjisChar1(ch) ) {
		    // JIS X 0208
		    buf[0] = ch;
		    nbuf = 1;
		} else {
		    // Invalid
		    result += QChar::replacement;
		}
		break;
	      case 1:
		// JIS X 0208
		if ( IsSjisChar2(ch) ) {
		    if ( IsUserDefinedChar1(buf[0]) ) {
			result += QChar::replacement;
		    } else {
			uint u = conv->SjisToUnicode(buf[0], ch);
			result += QValidChar(u);
		    }
		} else {
		    // Invalid
		    result += QChar::replacement;
		}
		nbuf = 0;
		break;
	    }
	}
	return result;
    }
};

QTextDecoder* QSjisCodec::makeDecoder() const
{
    return new QSjisDecoder(conv);
}

