/****************************************************************************
** $Id: //depot/qt/main/src/tools/qeucjpcodec.cpp#5 $
**
** Implementation of QEucJpCodec class
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

#include "qeucjpcodec.h"

static const uchar Esc = 0x1b;
static const uchar Ss2 = 0x8e;	// Single Shift 2
static const uchar Ss3 = 0x8f;	// Single Shift 3

#define	IsKana(c)	(((c) >= 0xa1) && ((c) <= 0xdf))
#define	IsEucChar(c)	(((c) >= 0xa1) && ((c) <= 0xfe))

#define	QValidChar(u)	((u) ? QChar((ushort)(u)) : QChar::replacement)

QEucJpCodec::QEucJpCodec() : conv(QJpUnicodeConv::newConverter(JU_Default))
{
}

int QEucJpCodec::mibEnum() const
{
    /*
    Name: Extended_UNIX_Code_Packed_Format_for_Japanese
    MIBenum: 18
    Source: Standardized by OSF, UNIX International, and UNIX Systems
	    Laboratories Pacific.  Uses ISO 2022 rules to select
		   code set 0: US-ASCII (a single 7-bit byte set)
		   code set 1: JIS X0208-1990 (a double 8-bit byte set)
			       restricted to A0-FF in both bytes
		   code set 2: Half Width Katakana (a single 7-bit byte set)
			       requiring SS2 as the character prefix
		   code set 3: JIS X0212-1990 (a double 7-bit byte set)
			       restricted to A0-FF in both bytes
			       requiring SS3 as the character prefix
    Alias: csEUCPkdFmtJapanese
    Alias: EUC-JP  (preferred MIME name)
    */
    return 18;
}

QCString QEucJpCodec::fromUnicode(const QString& uc, int& len_in_out) const
{
    int l = QMIN((int)uc.length(),len_in_out);
    int rlen = l*3+1;
    QCString rstr(rlen);
    uchar* cursor = (uchar*)rstr.data();
    for (int i=0; i<l; i++) {
	QChar ch = uc[i];
	uint j;
	if ( ch.row == 0x00 && ch.cell < 0x80 ) {
	    // ASCII
	    *cursor++ = ch.cell;
	} else if ((j = conv->UnicodeToJisx0201(ch.row, ch.cell)) != 0) {
	    if (j < 0x80) {
		// JIS X 0201 Latin ?
		*cursor++ = j;
	    } else {
		// JIS X 0201 Kana
		*cursor++ = Ss2;
		*cursor++ = j;
	    }
	} else if ((j = conv->UnicodeToJisx0208(ch.row, ch.cell)) != 0) {
	    // JIS X 0208
	    *cursor++ = (j >> 8)   | 0x80;
	    *cursor++ = (j & 0xff) | 0x80;
	} else if ((j = conv->UnicodeToJisx0212(ch.row, ch.cell)) != 0) {
	    // JIS X 0212
	    *cursor++ = Ss3;
	    *cursor++ = (j >> 8)   | 0x80;
	    *cursor++ = (j & 0xff) | 0x80;
	} else {
	    // Error
	    *cursor++ = '?';	// unknown char
	}
    }
    len_in_out = cursor - (uchar*)rstr.data();
    *cursor = 0;
    return rstr;
}

QString QEucJpCodec::toUnicode(const char* chars, int len) const
{
    QString result;
    for (int i=0; i<len; i++) {
	uchar ch = chars[i];
	if ( ch < 0x80 ) {
	    // ASCII
	    result += QChar(ch);
	} else if ( ch == Ss2 ) {
	    // JIS X 0201 Kana
	    if ( i < len-1 ) {
		uchar c2 = chars[++i];
		if ( IsKana(c2) ) {
		    uint u = conv->Jisx0201ToUnicode(c2);
		    result += QValidChar(u);
		} else {
		    i--;
		    result += QChar::replacement;
		}
	    }
	} else if ( ch == Ss3 ) {
	    // JIS X 0212
	    if ( i < len-1 ) {
		uchar c2 = chars[++i];
		if ( IsEucChar(c2) ) {
		    if ( i < len-1 ) {
			uchar c3 = chars[++i];
			if ( IsEucChar(c3) ) {
			    uint u = conv->Jisx0212ToUnicode(c2 & 0x7f, c3 & 0x7f);
			    result += QValidChar(u);
			} else {
			    i--;
			    result += QChar::replacement;
			}
		    } else {
			result += QChar::replacement;
		    }
		} else {
		    i--;
		    result += QChar::replacement;
		}
	    } else {
		result += QChar::replacement;
	    }
	} else if ( IsEucChar(ch) ) {
	    // JIS X 0208
	    if ( i < len-1 ) {
		uchar c2 = chars[++i];
		if ( IsEucChar(c2) ) {
		    uint u = conv->Jisx0208ToUnicode(ch & 0x7f, c2 & 0x7f);
		    result += QValidChar(u);
		} else {
		    i--;
		    result += QChar::replacement;
		}
	    } else {
		result += QChar::replacement;
	    }
	} else {
	    // Invalid
	    result += QChar::replacement;
	}
    }
    return result;
}

const char* QEucJpCodec::name() const
{
    return "eucJP";
}

int QEucJpCodec::heuristicNameMatch(const char* hint) const
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
	    return score;
	}
	p++;
    } else {
	p = hint;
    }
    if (p) {
	if ((stricmp(p, "AJEC") == 0) ||
	    (stricmp(p, "eucJP") == 0) ||
	    (stricmp(p, "ujis") == 0)) {
	    return score + 4;
	}
	// there exists ja_JP.EUC, ko_KR.EUC, zh_CN.EUC and zh_TW.EUC
	// so "euc" may or may not be Japanese EUC.
	if (stricmp(p, "euc") == 0) {
	    return ja ? score + 4 : 1;
	}
    }
    return QTextCodec::heuristicNameMatch(hint);
}

int QEucJpCodec::heuristicContentMatch(const char* chars, int len) const
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
	} else if ( ch == Ss2 ) {
	    // JIS X 0201 Kana
	    if ( i < len-1 ) {
		uchar c2 = chars[++i];
		if ( !IsKana(c2) )
		    return -1;
		score++;
	    }
	    score++;
	} else if ( ch == Ss3 ) {
	    // JIS X 0212
	    if ( i < len-1 ) {
		uchar c2 = chars[++i];
		if ( !IsEucChar(c2) )
		    return -1;
		if ( i < len-1 ) {
		    uchar c3 = chars[++i];
		    if ( !IsEucChar(c3) )
			return -1;
		    score++;
		}
		score++;
	    }
	    score++;
	} else if ( IsEucChar(ch) ) {
	    // JIS X 0208-1990
	    if ( i < len-1 ) {
		uchar c2 = chars[++i];
		if ( !IsEucChar(c2) )
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

class QEucJpDecoder : public QTextDecoder {
    uchar buf[2];
    int nbuf;
    const QJpUnicodeConv * const conv;
public:
    QEucJpDecoder(const QJpUnicodeConv *c) : nbuf(0), conv(c)
    {
    }

    QString toUnicode(const char* chars, int len)
    {
	QString result;
	for (int i=0; i<len; i++) {
	    uchar ch = chars[i];
	    switch (nbuf) {
	      case 0:
		if ( ch < 0x80 ) {
		    // ASCII
		    result += QChar(ch);
		} else if ( ch == Ss2 || ch == Ss3 ) {
		    // JIS X 0201 Kana or JIS X 0212
		    buf[0] = ch;
		    nbuf = 1;
		} else if ( IsEucChar(ch) ) {
		    // JIS X 0208
		    buf[0] = ch;
		    nbuf = 1;
		} else {
		    // Invalid
		    result += QChar::replacement;
		}
		break;
	      case 1:
		if ( buf[0] == Ss2 ) {
		    // JIS X 0201 Kana
		    if ( IsKana(ch) ) {
			uint u = conv->Jisx0201ToUnicode(ch);
			result += QValidChar(u);
		    } else {
			result += QChar::replacement;
		    }
		    nbuf = 0;
		} else if ( buf[0] == Ss3 ) {
		    // JIS X 0212-1990
		    if ( IsEucChar(ch) ) {
			buf[1] = ch;
			nbuf = 2;
		    } else {
			// Error
			result += QChar::replacement;
			nbuf = 0;
		    }
		} else {
		    // JIS X 0208-1990
		    if ( IsEucChar(ch) ) {
			uint u = conv->Jisx0208ToUnicode(buf[0] & 0x7f, ch & 0x7f);
			result += QValidChar(u);
		    } else {
			// Error
			result += QChar::replacement;
		    }
		    nbuf = 0;
		}
		break;
	    case 2:
		// JIS X 0212
		if ( IsEucChar(ch) ) {
		    uint u = conv->Jisx0212ToUnicode(buf[1] & 0x7f, ch & 0x7f);
		    result += QValidChar(u);
		} else {
		    result += QChar::replacement;
		}
		nbuf = 0;
	    }
	}
	return result;
    }
};

QTextDecoder* QEucJpCodec::makeDecoder() const
{
    return new QEucJpDecoder(conv);
}

