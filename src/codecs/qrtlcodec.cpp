/*
 * Copyright (c) 1999 Lars Knoll <knoll@mpi-hd.mpg.de>, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted under the terms of the QPL, Version 1.0
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

#include "qrtlcodec.h"

#ifndef QT_NO_CODECS

// NOT REVISED

static const uchar unkn = '?'; // BLACK SQUARE (94) would be better

static const ushort heb_to_unicode[128] = {
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0x00A0, 0xFFFD, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x203E,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x2017,
    0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
    0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
    0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7,
    0x05E8, 0x05E9, 0x05EA, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD
};

static const uchar unicode_to_heb_00[32] = {
    0xA0, unkn, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xD7, 0xAB, 0xAC, 0xAD, 0xAE, unkn,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xF7, 0xBB, 0xBC, 0xBD, 0xBE, unkn,
};

static const uchar unicode_to_heb_05[32] = {
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, unkn, unkn, unkn, unkn, unkn
};

/* this function assuems the QString is still visually ordered.
 * Finding the basic direction of the text is not easy in this case, since
 * a string like "my friend MOLAHS" could (in logical order) mean aswell
 * "SHALOM my friend" or "my friend SHALOM", depending on the basic direction
 * one assumes for the text.
 *
 * So this function uses some heuristics to find the right answer...
 */
static QChar::Direction findBasicDirection(QString str)
{
    unsigned int pos;
    unsigned int len = str.length();
    QChar::Direction dir1 = QChar::DirON;
    QChar::Direction dir2 = QChar::DirON;

    // If the visual representation of the first line starts and ends with the same
    // directionality, we know the answer.
    pos = 0;
    while (pos < len) {
	if (str.at(pos).direction() < 2) { // DirR or DirL 
	    dir1 = str.at(pos).direction();
	    break;
	}
	pos++;
    }

    if( pos == len ) // no directional chars, assume QChar::DirL
	return QChar::DirL;

    pos = len;
    while (pos > 0) {
	if (str.at(pos).direction() < 2) { // DirR or DirL
	    dir2 = str.at(pos).direction();
	    break;
	}
	pos--;
    }

    // both are the same, so we have the direction!
    if ( dir1 == dir2 ) return dir1;

    // guess with the help of punktuation marks...
    // if the sentence ends with a punktuation, we should have a mark
    // at one side of the text...

    pos = 0;
    while (pos < len-1 && str.at(pos).direction() < 2 ) {
	if(str.at(pos).category() == QChar::Punctuation_Other) {
	    if( str.at(pos+1).direction() < 2 ) return QChar::DirR;
	    else break; // no letter next to the mark... don't know
	}
	pos++;
    }

    pos = len;
    while (pos < 1 && str.at(pos).direction() < 2 ) {
	if(str.at(pos).category() == QChar::Punctuation_Other) {
	    if( str.at(pos-1).direction() < 2 ) return QChar::DirL;
	    else break; // no letter next to the mark... don't know
	}
	pos--;
    }

    // don't know try DirR...
    return QChar::DirR;
}

int QHebrewCodec::mibEnum() const
{
    return 11;
}

const char* QHebrewCodec::name() const
{
    return "ISO 8859-8";
}

QString QHebrewCodec::toUnicode(const char* chars, int len) const
{
    return toUnicode(chars, len, heb_to_unicode);
}

static void reverse(QString &input, unsigned int a, unsigned int b)
{
    // evil cast... I know...
    QChar *chars = (QChar *)input.unicode();

    QChar temp;
    while (a < b) {
	temp = chars[a];
	chars[a] = chars[b];
	chars[b] = temp;

	a++; b--;
    }
}

static void reverse(QString &str, unsigned int a, unsigned int b,
		    QChar::Direction dir)
{
    // first reverse....
    if(a != 0 || b != str.length()-1 || dir != QChar::DirL)
	reverse(str, a, b);

    // now, go through it, to see if there is some substring
    // we need to reverse again...
    QChar::Direction opposite = (dir==QChar::DirL ? QChar::DirR : QChar::DirL);
    while(a <= b) {
	QChar::Direction d = str.at(a).direction();
	if( d == opposite ) {
	    // found something to reverse...
	    uint c = a;
	    while( c < b && str.at(c).direction() == opposite )
		c++;
	    c--;
	    if( c > a ) {
		reverse(str, a, c, opposite);
		a = c;
	    }
	} else if ( dir == QChar::DirR &&
		    d == QChar::DirEN ||
		    d == QChar::DirAN ) {
	    uint c = a;
	    while( c < b) {
		d = str.at(c).direction();
		if ( d != QChar::DirEN && d != QChar::DirES &&
		     d != QChar::DirET && d != QChar::DirCS &&
		     d != QChar::DirAN ) {
		    c--;
		    break;
		}
		c++;
	    }
	    if( c > a ) {
		reverse(str, a, c, opposite);
		a = c;
	    }
	}
	a++;
    }
}

/*!
  Since hebrew (aswell as arabic) are written from left to right,
  but iso8859-8 assumes visual ordering (as opposed to the
  logical ordering of Unicode, we have to reverse the order of the
  input string to get it into logical order.

  One problem is, that the basic text direction is unknown. So this
  function uses some heuristics to find it, and if it can't guess the
  right one, it assumes, the basic text direction is right to left.

  This behaviour can be overwritten, by putting a control char
  at the beginning of the text telling the function which basic text
  direction to use. If the basic text direction is left-to-right, the
  control char is (uchar) 0xfe, for right-to-left it is 0xff. Both chars
  are undefined in the iso 8859-6/8 charsets.

  Example: A visually ordered string "english WERBEH english2" would
  be recognizes as having a basic left to right direction. so the logically
  ordered QString would be "english HEBREW english2".

  By prepending a (char)0xff before the string, QHebrewCodec::toUnicode would
  use a basic text direction of left-to-right, and the string would thus
  become "english2 HEBREW english".
  */
QString QHebrewCodec::toUnicode(const char* chars, int len,
			     const ushort *table) const
{
    QString r;
    const unsigned char * c = (const unsigned char *)chars;
    QChar::Direction basicDir = QChar::DirON; // neutral, we don't know

    if( len == 0 ) return QString::null;

    // Test, if the user gives us a directionality.
    // We use 0xFE and 0xFF in ISO8859-8 for that.
    // These chars are undefined in the charset, and are mapped to
    // RTL overwrite
    if( c[0] == 0xfe ) {
	basicDir = QChar::DirL;
	c++; // skip directionality hint
    }
    if( c[0] == 0xff ) {
	basicDir = QChar::DirR;
	c++; // skip directionality hint
    }

    for( int i=0; i<len; i++ ) {
	if ( c[i] > 127 )
	    r[i] = table[c[i]-128];
	else
	    r[i] = c[i];
    }

    // do transformation from visual byte ordering to logical byte
    // ordering
    if( basicDir == QChar::DirON )
	basicDir = findBasicDirection(r);

    reverse(r, 0, r.length()-1, basicDir);

    return r;
}

QCString QHebrewCodec::fromUnicode(const QString& uc, int& len_in_out) const
{
    // process only len chars...
    int l;
    if( len_in_out > 0 )
	l = QMIN((int)uc.length(),len_in_out);
    else
	l = (int)uc.length();

    QCString rstr;
    if( l == 1 ) {
	if( !to8bit( uc[0], &rstr ) )
	    rstr += unkn;
    } else {
	QString tmp = uc;
	tmp.truncate(l);
	QString vis = tmp; // ########   .visual();

	for (int i=0; i<l; i++) {
	    const QChar ch = vis[i];

	    if( !to8bit( ch, &rstr ) )
		rstr += unkn;
	}
	// len_in_out = cursor - result;
    }
    if( l > 0 && !rstr.length() )
	rstr += unkn;

    return rstr;
}

bool QHebrewCodec::to8bit(const QChar ch, QCString *rstr) const
{
    bool converted = FALSE;

    if( ch.isMark() ) return TRUE; // ignore marks for conversion

    if ( ch.row() ) {
	if ( ch.row() == 0x05 ) {
	    if ( ch.cell() > 0x91 )
		converted = TRUE;
	    // 0x0591 - 0x05cf: hebrew punktuation... dropped
	    if ( ch.cell() >= 0xD0 )
		*rstr += unicode_to_heb_05[ch.cell()- 0xD0];
	} else if ( ch.row() == 0x20 ) {
	    if ( ch.cell() == 0x3E ) {
		*rstr += (char)0xAF;
		converted = TRUE;
	    } else if ( ch.cell() == 0x17 ) {
		*rstr += (char)0xCF;
		converted = TRUE;
	    }
	} else {
	    converted = FALSE;
	}
    } else {
	if ( ch.cell() < 0x80 ) {
	    *rstr += ch.cell();
	    converted = TRUE;
	} else if( ch.cell() < 0xA0 ) {
	    *rstr += unicode_to_heb_00[ch.cell() - 0xA0];
	    converted = TRUE;
	}
    }

    if(converted) return TRUE;

    // couldn't convert the char... lets try its decomposition
    QString d = ch.decomposition();
    if(d.isNull())
	return FALSE;

    int l = d.length();
    for (int i=0; i<l; i++) {
	const QChar ch = d[i];

	if(to8bit(ch, rstr))
	    converted = TRUE;
    }

    return converted;
}

int QHebrewCodec::heuristicContentMatch(const char* chars, int len) const
{
    const unsigned char * c = (const unsigned char *)chars;

    int score = 0;
    for (int i=0; i<len; i++) {
	if(c[i] > 0x80 && heb_to_unicode[c[i] - 0x80] != 0xFFFD)
	    score++;
	else
	    return -1;
    }
    return score;
}

#endif
