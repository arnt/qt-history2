/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// Most of the code here was originally written by Serika Kurusugawa
// a.k.a. Junji Takagi, and is included in Qt with the author's permission,
// and the grateful thanks of the Trolltech team.

/*! \class QSjisCodec qsjiscodec.h
    \reentrant
    \ingroup i18n
  \brief The QSjisCodec class provides conversion to and from Shift-JIS.

  More precisely, the QSjisCodec class subclasses QTextCodec to
  provide support for Shift-JIS, an encoding of JIS X 0201 Latin, JIS
  X 0201 Kana or JIS X 0208.

  The environment variable \c UNICODEMAP_JP can be used to fine-tune
  QJisCodec, QSjisCodec and QEucJpCodec. The \l QJisCodec
  documentation describes how to use this variable.

  Most of the code here was written by Serika Kurusugawa,
  a.k.a. Junji Takagi, and is included in Qt with the author's
  permission and the grateful thanks of the Trolltech team.
  Here is the copyright statement for the code as it was at the
  point of contribution. Trolltech's subsequent modifications
  are covered by the usual copyright for Qt.

  \legalese

  Copyright (C) 1999 Serika Kurusugawa. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  \list 1
  \i Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  \i Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  \endlist

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS".
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.
*/

#include "qsjiscodec_p.h"

#ifndef QT_NO_BIG_CODECS

enum {
    Esc = 0x1b
};

#define        IsKana(c)        (((c) >= 0xa1) && ((c) <= 0xdf))
#define        IsSjisChar1(c)        ((((c) >= 0x81) && ((c) <= 0x9f)) ||        \
                         (((c) >= 0xe0) && ((c) <= 0xfc)))
#define        IsSjisChar2(c)        (((c) >= 0x40) && ((c) != 0x7f) && ((c) <= 0xfc))
#define        IsUserDefinedChar1(c)        (((c) >= 0xf0) && ((c) <= 0xfc))

#define        QValidChar(u)        ((u) ? QChar((ushort)(u)) : QChar(QChar::ReplacementCharacter))

/*!
  Creates a Shift-JIS codec. Note that this is done automatically by
  the QApplication, you do not need construct your own.
*/
QSjisCodec::QSjisCodec() : conv(QJpUnicodeConv::newConverter(QJpUnicodeConv::Default))
{
}


/*!
  Destroys the Shift-JIS codec.
*/
QSjisCodec::~QSjisCodec()
{
    delete (QJpUnicodeConv*)conv;
    conv = 0;
}


/*!
  \reimp
*/
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

/*!
  \reimp
*/
QByteArray QSjisCodec::fromUnicode(const QString& uc, int& lenInOut) const
{
    int l = qMin((int)uc.length(),lenInOut);
    int rlen = l*2+1;
    QByteArray rstr;
    rstr.resize(rlen);
    uchar* cursor = (uchar*)rstr.data();
    for (int i=0; i<l; i++) {
        QChar ch = uc[i];
        uint j;
        if (ch.row() == 0x00 && ch.cell() < 0x80) {
            // ASCII
            *cursor++ = ch.cell();
        } else if ((j = conv->unicodeToJisx0201(ch.row(), ch.cell())) != 0) {
            // JIS X 0201 Latin or JIS X 0201 Kana
            *cursor++ = j;
        } else if ((j = conv->unicodeToSjis(ch.row(), ch.cell())) != 0) {
            // JIS X 0208
            *cursor++ = (j >> 8);
            *cursor++ = (j & 0xff);
        } else if ((j = conv->unicodeToJisx0212(ch.row(), ch.cell())) != 0) {
            // JIS X 0212 (can't be encoded in ShiftJIS !)
            *cursor++ = 0x81;        // white square
            *cursor++ = 0xa0;        // white square
        } else {
            // Error
            *cursor++ = '?';        // unknown char
        }
    }
    lenInOut = cursor - (const uchar*)rstr.constData();
    rstr.resize(lenInOut);
    return rstr;
}

/*!
  \reimp
*/
QString QSjisCodec::toUnicode(const char* chars, int len) const
{
    QString result;
    for (int i=0; i<len; i++) {
        uchar ch = chars[i];
        if (ch < 0x80 || IsKana(ch)) {
            // JIS X 0201 Latin or JIS X 0201 Kana
            uint u = conv->jisx0201ToUnicode(ch);
            result += QValidChar(u);
        } else if (IsSjisChar1(ch)) {
            // JIS X 0208
            if (i < len-1) {
                uchar c2 = chars[++i];
                if (IsSjisChar2(c2)) {
                    if (IsUserDefinedChar1(ch)) {
                        result += QChar::ReplacementCharacter;
                    } else {
                        uint u = conv->sjisToUnicode(ch, c2);
                        result += QValidChar(u);
                    }
                } else {
                    i--;
                    result += QChar::ReplacementCharacter;
                }
            } else {
                result += QChar::ReplacementCharacter;
            }
        } else {
            result += QChar::ReplacementCharacter;
        }
    }
    return result;
}

/*!
  \reimp
*/
const char* QSjisCodec::name() const
{
    return "SJIS";
}

/*!
    Returns the codec's mime name.
*/
const char* QSjisCodec::mimeName() const
{
    return "Shift_JIS";
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
                if (ch < 0x80 || IsKana(ch)) {
                    // JIS X 0201 Latin or JIS X 0201 Kana
                    uint u = conv->jisx0201ToUnicode(ch);
                    result += QValidChar(u);
                } else if (IsSjisChar1(ch)) {
                    // JIS X 0208
                    buf[0] = ch;
                    nbuf = 1;
                } else {
                    // Invalid
                    result += QChar::ReplacementCharacter;
                }
                break;
              case 1:
                // JIS X 0208
                if (IsSjisChar2(ch)) {
                    if (IsUserDefinedChar1(buf[0])) {
                        result += QChar::ReplacementCharacter;
                    } else {
                        uint u = conv->sjisToUnicode(buf[0], ch);
                        result += QValidChar(u);
                    }
                } else {
                    // Invalid
                    result += QChar::ReplacementCharacter;
                }
                nbuf = 0;
                break;
            }
        }
        return result;
    }
};

/*!
  \reimp
*/
QTextDecoder* QSjisCodec::makeDecoder() const
{
    return new QSjisDecoder(conv);
}

#endif
