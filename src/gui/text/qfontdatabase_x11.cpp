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

#include <qplatformdefs.h>

#include <qdatetime.h>
#include <qpaintdevice.h>

#include <private/qt_x11_p.h>
#include "qx11info_x11.h"

#include <ctype.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#ifndef QT_NO_FONTCONFIG
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

// from qfont_x11.cpp
extern double qt_pointSize(double pixelSize, int dpi);
extern double qt_pixelSize(double pointSize, int dpi);

static inline void capitalize (char *s)
{
    bool space = true;
    while(*s) {
        if (space)
            *s = toupper(*s);
        space = (*s == ' ');
        ++s;
    }
}


/*
  To regenerate the writingSystems_for_xlfd_encoding table, run
  'util/unicode/x11/makeencodings' and paste the generated
  'encodings.c' here.
*/
// ----- begin of generated code -----

#define make_tag( c1, c2, c3, c4 )                         \
    ((((unsigned int)c1)<<24) | (((unsigned int)c2)<<16) | \
    (((unsigned int)c3)<<8) | ((unsigned int)c4))

struct XlfdEncoding {
    const char *name;
    int id;
    int mib;
    unsigned int hash1;
    unsigned int hash2;
};

static const XlfdEncoding xlfd_encoding[] = {
    { "iso8859-1", 0, 4, make_tag('i','s','o','8'), make_tag('5','9','-','1') },
    { "iso8859-2", 1, 5, make_tag('i','s','o','8'), make_tag('5','9','-','2') },
    { "iso8859-3", 2, 6, make_tag('i','s','o','8'), make_tag('5','9','-','3') },
    { "iso8859-4", 3, 7, make_tag('i','s','o','8'), make_tag('5','9','-','4') },
    { "iso8859-9", 4, 12, make_tag('i','s','o','8'), make_tag('5','9','-','9') },
    { "iso8859-10", 5, 13, make_tag('i','s','o','8'), make_tag('9','-','1','0') },
    { "iso8859-13", 6, 109, make_tag('i','s','o','8'), make_tag('9','-','1','3') },
    { "iso8859-14", 7, 110, make_tag('i','s','o','8'), make_tag('9','-','1','4') },
    { "iso8859-15", 8, 111, make_tag('i','s','o','8'), make_tag('9','-','1','5') },
    { "hp-roman8", 9, 2004, make_tag('h','p','-','r'), make_tag('m','a','n','8') },
    { "iso8859-5", 10, 8, make_tag('i','s','o','8'), make_tag('5','9','-','5') },
    { "*-cp1251", 11, 2251, 0, make_tag('1','2','5','1') },
    { "koi8-ru", 12, 2084, make_tag('k','o','i','8'), make_tag('8','-','r','u') },
    { "koi8-u", 13, 2088, make_tag('k','o','i','8'), make_tag('i','8','-','u') },
    { "koi8-r", 14, 2084, make_tag('k','o','i','8'), make_tag('i','8','-','r') },
    { "iso8859-7", 15, 10, make_tag('i','s','o','8'), make_tag('5','9','-','7') },
    { "iso8859-8", 16, 85, make_tag('i','s','o','8'), make_tag('5','9','-','8') },
    { "gb18030-0", 17, -114, make_tag('g','b','1','8'), make_tag('3','0','-','0') },
    { "gb18030.2000-0", 18, -113, make_tag('g','b','1','8'), make_tag('0','0','-','0') },
    { "gbk-0", 19, -113, make_tag('g','b','k','-'), make_tag('b','k','-','0') },
    { "gb2312.*-0", 20, 57, make_tag('g','b','2','3'), 0 },
    { "jisx0201*-0", 21, 15, make_tag('j','i','s','x'), 0 },
    { "jisx0208*-0", 22, 63, make_tag('j','i','s','x'), 0 },
    { "ksc5601*-*", 23, 36, make_tag('k','s','c','5'), 0 },
    { "big5hkscs-0", 24, -2101, make_tag('b','i','g','5'), make_tag('c','s','-','0') },
    { "hkscs-1", 25, -2101, make_tag('h','k','s','c'), make_tag('c','s','-','1') },
    { "big5*-*", 26, -2026, make_tag('b','i','g','5'), 0 },
    { "tscii-*", 27, 2028, make_tag('t','s','c','i'), 0 },
    { "tis620*-*", 28, 2259, make_tag('t','i','s','6'), 0 },
    { "iso8859-11", 29, 2259, make_tag('i','s','o','8'), make_tag('9','-','1','1') },
    { "mulelao-1", 30, -4242, make_tag('m','u','l','e'), make_tag('a','o','-','1') },
    { "ethiopic-unicode", 31, 0, make_tag('e','t','h','i'), make_tag('c','o','d','e') },
    { "iso10646-1", 32, 0, make_tag('i','s','o','1'), make_tag('4','6','-','1') },
    { "unicode-*", 33, 0, make_tag('u','n','i','c'), 0 },
    { "*-symbol", 34, 0, 0, make_tag('m','b','o','l') },
    { "*-fontspecific", 35, 0, 0, make_tag('i','f','i','c') },
    { "fontspecific-*", 36, 0, make_tag('f','o','n','t'), 0 },
    { 0, 0, 0, 0, 0 }
};

static const char writingSystems_for_xlfd_encoding[37][39] = {
    // iso8859-1
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-2
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-3
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-4
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-9
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-10
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-13
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-14
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-15
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // hp-roman8
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-5
    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // *-cp1251
    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // koi8-ru
    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // koi8-u
    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // koi8-r
    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-7
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-8
    { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // gb18030-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // gb18030.2000-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // gbk-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // gb2312.*-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // jisx0201*-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // jisx0208*-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // ksc5601*-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // big5hkscs-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // hkscs-1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // big5*-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // tscii-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // tis620*-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-11
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // mulelao-1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // ethiopic-unicode
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1 },
    // iso10646-1
    { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      1, 1, 0, 1, 0, 1, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // unicode-*
    { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      1, 1, 0, 1, 0, 1, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // *-symbol
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1 },
    // *-fontspecific
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1 },
    // fontspecific-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1 }

};

// ----- end of generated code -----


const int numEncodings = sizeof(xlfd_encoding) / sizeof(XlfdEncoding) - 1;

int qt_xlfd_encoding_id(const char *encoding)
{
    // qDebug("looking for encoding id for '%s'", encoding);
    int len = strlen(encoding);
    if (len < 4)
        return -1;
    unsigned int hash1 = make_tag(encoding[0], encoding[1], encoding[2], encoding[3]);
    const char *ch = encoding + len - 4;
    unsigned int hash2 = make_tag(ch[0], ch[1], ch[2], ch[3]);

    const XlfdEncoding *enc = xlfd_encoding;
    for (; enc->name; ++enc) {
        if ((enc->hash1 && enc->hash1 != hash1) ||
             (enc->hash2 && enc->hash2 != hash2))
            continue;
        // hashes match, do a compare if strings match
        // the enc->name can contain '*'s we have to interpret correctly
        const char *n = enc->name;
        const char *e = encoding;
        while (1) {
            // qDebug("bol: *e='%c', *n='%c'", *e,  *n);
            if (*e == '\0') {
                if (*n)
                    break;
                // qDebug("found encoding id %d", enc->id);
                return enc->id;
            }
            if (*e == *n) {
                ++e;
                ++n;
                continue;
            }
            if (*n != '*')
                break;
            ++n;
            // qDebug("skip: *e='%c', *n='%c'", *e,  *n);
            while (*e && *e != *n)
                ++e;
        }
    }
    // qDebug("couldn't find encoding %s", encoding);
    return -1;
}

int qt_mib_for_xlfd_encoding(const char *encoding)
{
    int id = qt_xlfd_encoding_id(encoding);
    if (id != -1) return xlfd_encoding[id].mib;
    return 0;
};

int qt_encoding_id_for_mib(int mib)
{
    const XlfdEncoding *enc = xlfd_encoding;
    for (; enc->name; ++enc) {
        if (enc->mib == mib)
            return enc->id;
    }
    return -1;
}

static const char * xlfd_for_id(int id)
{
    // special case: -1 returns the "*-*" encoding, allowing us to do full
    // database population in a single X server round trip.
    if (id < 0 || id > numEncodings)
        return "*-*";
    return xlfd_encoding[id].name;
}

enum XLFDFieldNames {
    Foundry,
    Family,
    Weight,
    Slant,
    Width,
    AddStyle,
    PixelSize,
    PointSize,
    ResolutionX,
    ResolutionY,
    Spacing,
    AverageWidth,
    CharsetRegistry,
    CharsetEncoding,
    NFontFields
};

// Splits an X font name into fields separated by '-'
static bool parseXFontName(char *fontName, char **tokens)
{
    if (! fontName || fontName[0] == '0' || fontName[0] != '-') {
        tokens[0] = 0;
        return false;
    }

    int          i;
    ++fontName;
    for (i = 0; i < NFontFields && fontName && fontName[0]; ++i) {
        tokens[i] = fontName;
        for (;; ++fontName) {
            if (*fontName == '-')
                break;
            if (! *fontName) {
                fontName = 0;
                break;
            }
        }

        if (fontName) *fontName++ = '\0';
    }

    if (i < NFontFields) {
        for (int j = i ; j < NFontFields; ++j)
            tokens[j] = 0;
        return false;
    }

    return true;
}

static inline bool isZero(char *x)
{
    return (x[0] == '0' && x[1] == 0);
}

static inline bool isScalable(char **tokens)
{
    return (isZero(tokens[PixelSize]) &&
            isZero(tokens[PointSize]) &&
            isZero(tokens[AverageWidth]));
}

static inline bool isSmoothlyScalable(char **tokens)
{
    return (isZero(tokens[ResolutionX]) &&
            isZero(tokens[ResolutionY]));
}

static inline bool isFixedPitch(char **tokens)
{
    return (tokens[Spacing][0] == 'm' ||
            tokens[Spacing][0] == 'c' ||
            tokens[Spacing][0] == 'M' ||
            tokens[Spacing][0] == 'C');
}

/*
  Fills in a font definition (QFontDef) from an XLFD (X Logical Font
  Description).

  Returns true if the the given xlfd is valid.
*/
bool qt_fillFontDef(const QByteArray &xlfd, QFontDef *fd, int dpi)
{
    char *tokens[NFontFields];
    QByteArray buffer = xlfd;
    if (! parseXFontName(buffer.data(), tokens))
        return false;

    capitalize(tokens[Family]);
    capitalize(tokens[Foundry]);

    fd->family = QString::fromLatin1(tokens[Family]);
    QString foundry = QString::fromLatin1(tokens[Foundry]);
    if (! foundry.isEmpty() && foundry != QString::fromLatin1("*"))
        fd->family +=
            QString::fromLatin1(" [") + foundry + QString::fromLatin1("]");

    if (qstrlen(tokens[AddStyle]) > 0)
        fd->addStyle = QString::fromLatin1(tokens[AddStyle]);
    else
        fd->addStyle.clear();

    fd->pointSize = atoi(tokens[PointSize]);
    fd->styleHint = QFont::AnyStyle;        // ### any until we match families

    char slant = tolower((uchar) tokens[Slant][0]);
    fd->style = (slant == 'o' ? QFont::StyleOblique : (slant == 'i' ? QFont::StyleItalic : QFont::StyleNormal));
    char fixed = tolower((uchar) tokens[Spacing][0]);
    fd->fixedPitch = (fixed == 'm' || fixed == 'c');
    fd->weight = getFontWeight(tokens[Weight]);

    int r = atoi(tokens[ResolutionY]);
    fd->pixelSize = atoi(tokens[PixelSize]);
    // not "0" or "*", or required DPI
    if (r && fd->pixelSize && r != dpi) {
        // calculate actual pointsize for display DPI
        fd->pointSize = qt_pointSize(fd->pixelSize, dpi);
    } else if (fd->pixelSize == 0 && fd->pointSize) {
        // calculate pixel size from pointsize/dpi
        fd->pixelSize = qRound(qt_pixelSize(fd->pointSize, dpi));
    }

    return true;
}

/*
  Fills in a font definition (QFontDef) from the font properties in an
  XFontStruct.

  Returns true if the QFontDef could be filled with properties from
  the XFontStruct.
*/
static bool qt_fillFontDef(XFontStruct *fs, QFontDef *fd, int dpi)
{
    unsigned long value;
    if (fs && !XGetFontProperty(fs, XA_FONT, &value))
        return false;

    char *n = XGetAtomName(QX11Info::display(), value);
    QByteArray xlfd(n);
    if (n)
        XFree(n);
    return qt_fillFontDef(xlfd.toLower(), fd, dpi);
}


static QtFontStyle::Key getStyle(char ** tokens)
{
    QtFontStyle::Key key;

    char slant0 = tolower((uchar) tokens[Slant][0]);

    if (slant0 == 'r') {
        if (tokens[Slant][1]) {
            char slant1 = tolower((uchar) tokens[Slant][1]);

            if (slant1 == 'o')
                key.style = QFont::StyleOblique;
            else if (slant1 == 'i')
                key.style = QFont::StyleItalic;
        }
    } else if (slant0 == 'o')
        key.style = QFont::StyleOblique;
    else if (slant0 == 'i')
        key.style = QFont::StyleItalic;

    key.weight = getFontWeight(tokens[Weight]);

    if (qstrcmp(tokens[Width], "normal") == 0) {
        key.stretch = 100;
    } else if (qstrcmp(tokens[Width], "semi condensed") == 0 ||
                qstrcmp(tokens[Width], "semicondensed") == 0) {
        key.stretch = 90;
    } else if (qstrcmp(tokens[Width], "condensed") == 0) {
        key.stretch = 80;
    } else if (qstrcmp(tokens[Width], "narrow") == 0) {
        key.stretch = 60;
    }

    return key;
}


static bool xlfdsFullyLoaded = false;
static unsigned char encodingLoaded[numEncodings];

static void loadXlfds(const char *reqFamily, int encoding_id)
{
    QFontDatabasePrivate *db = privateDb();
    QtFontFamily *fontFamily = reqFamily ? db->family(reqFamily) : 0;

    // make sure we don't load twice
    if ((encoding_id == -1 && xlfdsFullyLoaded)
        || (encoding_id != -1 && encodingLoaded[encoding_id]))
        return;
    if (fontFamily && fontFamily->xlfdLoaded)
        return;

    int fontCount;
    // force the X server to give us XLFDs
    QByteArray xlfd_pattern("-*-");
    xlfd_pattern += reqFamily ? reqFamily : "*";
    xlfd_pattern += "-*-*-*-*-*-*-*-*-*-*-";
    xlfd_pattern += xlfd_for_id(encoding_id);

    char **fontList = XListFonts(QX11Info::display(),
                                 xlfd_pattern,
                                 0xffff, &fontCount);
    // qDebug("requesting xlfd='%s', got %d fonts", xlfd_pattern.data(), fontCount);


    char *tokens[NFontFields];

    for(int i = 0 ; i < fontCount ; i++) {
        if (! parseXFontName(fontList[i], tokens))
            continue;

        // get the encoding_id for this xlfd.  we need to do this
        // here, since we can pass -1 to this function to do full
        // database population
        *(tokens[CharsetEncoding] - 1) = '-';
        int encoding_id = qt_xlfd_encoding_id(tokens[CharsetRegistry]);
        if (encoding_id == -1)
            continue;

        char *familyName = tokens[Family];
        capitalize(familyName);
        char *foundryName = tokens[Foundry];
        capitalize(foundryName);
        QtFontStyle::Key styleKey = getStyle(tokens);

        bool smooth_scalable = false;
        bool bitmap_scalable = false;
        if (isScalable(tokens)) {
            if (isSmoothlyScalable(tokens))
                smooth_scalable = true;
            else
                bitmap_scalable = true;
        }
        uint pixelSize = atoi(tokens[PixelSize]);
        uint xpointSize = atoi(tokens[PointSize]);
        uint xres = atoi(tokens[ResolutionX]);
        uint yres = atoi(tokens[ResolutionY]);
        uint avgwidth = atoi(tokens[AverageWidth]);
        bool fixedPitch = isFixedPitch(tokens);

        if (avgwidth == 0 && pixelSize != 0) {
            /*
              Ignore bitmap scalable fonts that are automatically
              generated by some X servers.  We know they are bitmap
              scalable because even though they have a specified pixel
              size, the average width is zero.
            */
            continue;
        }

        QtFontFamily *family = fontFamily ? fontFamily : db->family(familyName, true);
        family->fontFileIndex = -1;
        QtFontFoundry *foundry = family->foundry(foundryName, true);
        QtFontStyle *style = foundry->style(styleKey, true);

        delete [] style->weightName;
        style->weightName = qstrdup(tokens[Weight]);
        delete [] style->setwidthName;
        style->setwidthName = qstrdup(tokens[Width]);

        if (smooth_scalable) {
            style->smoothScalable = true;
            style->bitmapScalable = false;
            pixelSize = SMOOTH_SCALABLE;
        }
        if (!style->smoothScalable && bitmap_scalable)
            style->bitmapScalable = true;
        if (!fixedPitch)
            family->fixedPitch = false;

        QtFontSize *size = style->pixelSize(pixelSize, true);
        QtFontEncoding *enc =
            size->encodingID(encoding_id, xpointSize, xres, yres, avgwidth, true);
        enc->pitch = *tokens[Spacing];
        if (!enc->pitch) enc->pitch = '*';

        for (int i = 0; i < QFontDatabase::WritingSystemsCount; ++i) {
            if (writingSystems_for_xlfd_encoding[encoding_id][i])
                family->writingSystems[i] = QtFontFamily::Supported;
            else
                family->writingSystems[i] |= QtFontFamily::UnsupportedXLFD;
        }
        if (encoding_id == -1)
            family->xlfdLoaded = true;
    }
    if (!reqFamily) {
        // mark encoding as loaded
        if (encoding_id == -1)
            xlfdsFullyLoaded = true;
        else
            encodingLoaded[encoding_id] = true;
    }

    XFreeFontNames(fontList);
}


#ifndef QT_NO_FONTCONFIG
static int getFCWeight(int fc_weight)
{
    int qtweight = QFont::Black;
    if (fc_weight <= (FC_WEIGHT_LIGHT + FC_WEIGHT_MEDIUM) / 2)
        qtweight = QFont::Light;
    else if (fc_weight <= (FC_WEIGHT_MEDIUM + FC_WEIGHT_DEMIBOLD) / 2)
        qtweight = QFont::Normal;
    else if (fc_weight <= (FC_WEIGHT_DEMIBOLD + FC_WEIGHT_BOLD) / 2)
        qtweight = QFont::DemiBold;
    else if (fc_weight <= (FC_WEIGHT_BOLD + FC_WEIGHT_BLACK) / 2)
        qtweight = QFont::Bold;

    return qtweight;
}

QFontDef FcPatternToQFontDef(FcPattern *pattern)
{
    QFontDef fontDef;

    FcChar8 *value = 0;
    if (FcPatternGetString(pattern, FC_FAMILY, 0, &value) == FcResultMatch) {
        fontDef.family = QString::fromUtf8(reinterpret_cast<const char *>(value));
        fontDef.family.replace('-', ' ');
        fontDef.family.replace("/", "");
    }

    double dpi;
    if (FcPatternGetDouble(pattern, FC_DPI, 0, &dpi) != FcResultMatch)
        dpi = QX11Info::appDpiY();

    double size;
    if (FcPatternGetDouble(pattern, FC_PIXEL_SIZE, 0, &size) == FcResultMatch)
        fontDef.pixelSize = qRound(size);
    else
        fontDef.pixelSize = 12;

    fontDef.pointSize = qt_pointSize(fontDef.pixelSize, qRound(dpi));

    /* ###
       fontDef.styleStrategy
       fontDef.styleHint
    */

    int weight;
    if (FcPatternGetInteger(pattern, FC_WEIGHT, 0, &weight) != FcResultMatch)
        weight = FC_WEIGHT_MEDIUM;
    fontDef.weight = getFCWeight(weight);

    int slant;
    if (FcPatternGetInteger(pattern, FC_SLANT, 0, &slant) != FcResultMatch)
        slant = FC_SLANT_ROMAN;
    fontDef.style = (slant == FC_SLANT_ITALIC)
                    ? QFont::StyleItalic
                    : ((slant == FC_SLANT_OBLIQUE)
                       ? QFont::StyleOblique
                       : QFont::StyleNormal);

    FcBool scalable;
    if (FcPatternGetBool(pattern, FC_SCALABLE, 0, &scalable) != FcResultMatch)
        scalable = false;
    if (scalable) {
        FcMatrix identity;
        FcMatrix *matrix;
        if (FcPatternGetMatrix(pattern, FC_MATRIX, 0, &matrix) != FcResultMatch) {
            FcMatrixInit(&identity);
            matrix = &identity;
        }
        if (matrix->yx == 0.0 && matrix->yy == 1.0) {
            // get the stretch
            fontDef.stretch = qRound(matrix->xx * 100.0);
            // shear?
            if (matrix->xy != 0.0) {
                // yup... we're using our "fake oblique" trick
                fontDef.style = QFont::StyleOblique;
            }
        } else {
            // ###
            fontDef.stretch = 100;
        }
    } else {
#if FC_VERSION >= 20193
        int width;
        if (FcPatternGetInteger(pattern, FC_WIDTH, 0, &width) == FcResultMatch)
            fontDef.stretch = width;
        else
#endif
            fontDef.stretch = 100;
    }

    int spacing;
    if (FcPatternGetInteger(pattern, FC_SPACING, 0, &spacing) == FcResultMatch) {
        fontDef.fixedPitch = (spacing >= FC_MONO);
        fontDef.ignorePitch = false;
    } else {
        fontDef.ignorePitch = true;
    }

    return fontDef;
}

static const char *specialLanguages[] = {
    "en",
    "he",
    "ar",
    "syr",
    "div",
    "hi",
    "bn",
    "pa",
    "gu",
    "or",
    "ta",
    "te",
    "kn",
    "ml",
    "si",
    "th",
    "lo",
    "bo",
    "my",
    "ko",
    "km"
};
enum { SpecialLanguageCount = sizeof(specialLanguages) / sizeof(const char *) };

// this could become a list of all languages used for each writing
// system, instead of using the single most common language.
static const char *languageForWritingSystem[] = {
    0,     // Any
    "en",  // Latin
    "el",  // Greek
    "ru",  // Cyrillic
    "hy",  // Armenian
    "he",  // Hebrew
    "ar",  // Arabic
    "syr", // Syriac
    "div", // Thaana
    "hi",  // Devanagari
    "bn",  // Bengali
    "pa",  // Gurmukhi
    "gu",  // Gujarati
    "or",  // Oriya
    "ta",  // Tamil
    "te",  // Telugu
    "kn",  // Kannada
    "ml",  // Malayalam
    "si",  // Sinhala
    "th",  // Thai
    "lo",  // Lao
    "bo",  // Tibetan
    "my",  // Myanmar
    "ka",  // Georgian
    "km",  // Khmer
    "zh-cn", // SimplifiedChinese
    "zh-tw", // TraditionalChinese
    "ja",  // Japanese
    "ko",  // Korean
    "vi",  // Vietnamese
};
enum { LanguageCount = sizeof(languageForWritingSystem) / sizeof(const char *) };

static void loadFontConfig()
{
    if (!X11->has_fontconfig)
        return;

    Q_ASSERT_X(int(QUnicodeTables::ScriptCount) == SpecialLanguageCount,
               "QFontDatabase", "New scripts have been added.");
    Q_ASSERT_X((QFontDatabase::WritingSystemsCount - 1) == LanguageCount,
               "QFontDatabase", "New writing systems have been added.");

    QFontDatabasePrivate *db = privateDb();
    FcFontSet  *fonts;

    QString familyName;
    QString rawName;
    FcChar8 *value = 0;
    int weight_value;
    int slant_value;
    int spacing_value;
    FcChar8 *file_value;
    int index_value;
    FcChar8 *foundry_value;
    FcBool scalable;

    {
        FcObjectSet *os = FcObjectSetCreate();
        FcPattern *pattern = FcPatternCreate();
        const char *properties [] = {
            FC_FAMILY, FC_WEIGHT, FC_SLANT,
            FC_SPACING, FC_FILE, FC_INDEX,
            FC_LANG, FC_FOUNDRY, FC_SCALABLE, FC_PIXEL_SIZE, FC_WEIGHT,
#if FC_VERSION >= 20193
            FC_WIDTH,
#endif
            (const char *)0
        };
        const char **p = properties;
        while (*p) {
            FcObjectSetAdd(os, *p);
            ++p;
        }
        fonts = FcFontList(0, pattern, os);
        FcObjectSetDestroy(os);
        FcPatternDestroy(pattern);
    }

    for (int i = 0; i < fonts->nfont; i++) {
        if (FcPatternGetString(fonts->fonts[i], FC_FAMILY, 0, &value) != FcResultMatch)
            continue;
        //         capitalize(value);
        rawName = familyName = QString::fromUtf8((const char *)value);
        familyName.replace('-', ' ');
        familyName.replace("/", "");
        slant_value = FC_SLANT_ROMAN;
        weight_value = FC_WEIGHT_MEDIUM;
        spacing_value = FC_PROPORTIONAL;
	file_value = 0;
	index_value = 0;
	scalable = FcTrue;

        if (FcPatternGetInteger (fonts->fonts[i], FC_SLANT, 0, &slant_value) != FcResultMatch)
	    slant_value = FC_SLANT_ROMAN;
        if (FcPatternGetInteger (fonts->fonts[i], FC_WEIGHT, 0, &weight_value) != FcResultMatch)
	    weight_value = FC_WEIGHT_MEDIUM;
        if (FcPatternGetInteger (fonts->fonts[i], FC_SPACING, 0, &spacing_value) != FcResultMatch)
	    spacing_value = FC_PROPORTIONAL;
        if (FcPatternGetString (fonts->fonts[i], FC_FILE, 0, &file_value) != FcResultMatch)
	    file_value = 0;
        if (FcPatternGetInteger (fonts->fonts[i], FC_INDEX, 0, &index_value) != FcResultMatch)
	    index_value = 0;
        if (FcPatternGetBool(fonts->fonts[i], FC_SCALABLE, 0, &scalable) != FcResultMatch)
	    scalable = FcTrue;
        if (FcPatternGetString(fonts->fonts[i], FC_FOUNDRY, 0, &foundry_value) != FcResultMatch)
	    foundry_value = 0;
        QtFontFamily *family = db->family(familyName, true);
        family->rawName = rawName;
        family->hasFT = true;

        FcLangSet *langset = 0;
        FcResult res = FcPatternGetLangSet(fonts->fonts[i], FC_LANG, 0, &langset);
        if (res == FcResultMatch) {
            for (int i = 1; i < LanguageCount; ++i) {
                const FcChar8 *lang = (const FcChar8*) languageForWritingSystem[i];
                if (!lang) {
                    family->writingSystems[i] |= QtFontFamily::UnsupportedFT;
                } else {
                    FcLangResult langRes = FcLangSetHasLang(langset, lang);
                    if (langRes != FcLangDifferentLang)
                        family->writingSystems[i] = QtFontFamily::Supported;
                    else
                        family->writingSystems[i] |= QtFontFamily::UnsupportedFT;
                }
            }
            family->writingSystems[QFontDatabase::Other] = QtFontFamily::UnsupportedFT;
            family->ftWritingSystemCheck = true;
        } else {
            // we set Other to supported for symbol fonts. It makes no
            // sense to merge these with other ones, as they are
            // special in a way.
            for (int i = 1; i < LanguageCount; ++i)
                family->writingSystems[i] |= QtFontFamily::UnsupportedFT;
            family->writingSystems[QFontDatabase::Other] = QtFontFamily::Supported;
        }

        QByteArray file((const char *)file_value);
        family->fontFilename = file;
        family->fontFileIndex = index_value;

        QtFontStyle::Key styleKey;
        styleKey.style = (slant_value == FC_SLANT_ITALIC)
                         ? QFont::StyleItalic
                         : ((slant_value == FC_SLANT_OBLIQUE)
                            ? QFont::StyleOblique
                            : QFont::StyleNormal);
        styleKey.weight = getFCWeight(weight_value);
        if (!scalable) {
            int width = 100;
#if FC_VERSION >= 20193
            FcPatternGetInteger (fonts->fonts[i], FC_WIDTH, 0, &width);
#endif
            styleKey.stretch = width;
        }

        QtFontFoundry *foundry
            = family->foundry(foundry_value ? QString::fromUtf8((const char *)foundry_value) : QString::null,  true);
        QtFontStyle *style = foundry->style(styleKey,  true);

        if (spacing_value < FC_MONO)
            family->fixedPitch = false;

        QtFontSize *size;
        if (scalable) {
            style->smoothScalable = true;
            size = style->pixelSize(SMOOTH_SCALABLE, true);
        } else {
            double pixel_size = 0;
            FcPatternGetDouble (fonts->fonts[i], FC_PIXEL_SIZE, 0, &pixel_size);
            size = style->pixelSize((int)pixel_size, true);
        }
        QtFontEncoding *enc = size->encodingID(-1, 0, 0, 0, 0, true);
        enc->pitch = (spacing_value >= FC_CHARCELL ? 'c' :
                      (spacing_value >= FC_MONO ? 'm' : 'p'));
    }

    FcFontSetDestroy (fonts);

    struct FcDefaultFont {
        const char *qtname;
        const char *rawname;
        bool fixed;
    };
    const FcDefaultFont defaults[] = {
        { "Serif", "serif", false },
        { "Sans Serif", "sans-serif", false },
        { "Monospace", "monospace", true },
        { 0, 0, false }
    };
    const FcDefaultFont *f = defaults;
    while (f->qtname) {
        QtFontFamily *family = db->family(f->qtname, true);
        family->rawName = f->rawname;
        family->hasFT = true;
        family->synthetic = true;
        QtFontFoundry *foundry = family->foundry(QString::null,  true);

        // aliases only make sense for 'common', not for any of the specials
        for (int i = 1; i < LanguageCount; ++i) {
            if (requiresOpenType(i))
                family->writingSystems[i] = QtFontFamily::UnsupportedFT;
            else
                family->writingSystems[i] = QtFontFamily::Supported;
        }
        family->writingSystems[QFontDatabase::Other] = QtFontFamily::UnsupportedFT;

        QtFontStyle::Key styleKey;
        for (int i = 0; i < 4; ++i) {
            styleKey.style = (i%2) ? QFont::StyleNormal : QFont::StyleItalic;
            styleKey.weight = (i > 1) ? QFont::Bold : QFont::Normal;
            QtFontStyle *style = foundry->style(styleKey,  true);
            style->smoothScalable = true;
            QtFontSize *size = style->pixelSize(SMOOTH_SCALABLE, true);
            QtFontEncoding *enc = size->encodingID(-1, 0, 0, 0, 0, true);
            enc->pitch = (f->fixed ? 'm' : 'p');
        }
        ++f;
    }
}
#endif // QT_NO_FONTCONFIG

static void load(const QString &family = QString::null, int script = -1)
{
    if (X11->has_fontconfig)
        return;

#ifdef QFONTDATABASE_DEBUG
    QTime t;
    t.start();
#endif

    if (family.isNull() && script == -1) {
        loadXlfds(0, -1);
    } else {
        if (family.isNull()) {
            // load all families in all writing systems that match \a script
            for (int ws = 1; ws < QFontDatabase::WritingSystemsCount; ++ws) {
                if (scriptForWritingSystem[ws] != script)
                    continue;
                for (int i = 0; i < numEncodings; ++i) {
                    if (writingSystems_for_xlfd_encoding[i][ws])
                        loadXlfds(0, i);
                }
            }
        } else {
            QtFontFamily *f = privateDb()->family(family, true);
            // could reduce this further with some more magic:
            // would need to remember the encodings loaded for the family.
            if (!f->xlfdLoaded)
                loadXlfds(family.toLatin1(), -1);
        }
    }

#ifdef QFONTDATABASE_DEBUG
    FD_DEBUG("QFontDatabase: load(%s, %d) took %d ms",
             family.toLatin1().constData(), script, t.elapsed());
#endif
}

static void initializeDb()
{
    QFontDatabasePrivate *db = privateDb();
    if (!db || db->count)
        return;

    QTime t;
    t.start();

#ifndef QT_NO_FONTCONFIG
    loadFontConfig();
    FD_DEBUG("QFontDatabase: loaded FontConfig: %d ms",  t.elapsed());
#endif

    t.start();

#ifndef QT_NO_FONTCONFIG
    for (int i = 0; i < db->count; i++) {
        for (int j = 0; j < db->families[i]->count; ++j) {        // each foundry
            QtFontFoundry *foundry = db->families[i]->foundries[j];
            for (int k = 0; k < foundry->count; ++k) {
                QtFontStyle *style = foundry->styles[k];
                if (style->key.style != QFont::StyleNormal) continue;

                QtFontSize *size = style->pixelSize(SMOOTH_SCALABLE);
                if (! size) continue; // should not happen
                QtFontEncoding *enc = size->encodingID(-1, 0, 0, 0, 0, true);
                if (! enc) continue; // should not happen either

                QtFontStyle::Key key = style->key;

                // does this style have an italic equivalent?
                key.style = QFont::StyleItalic;
                QtFontStyle *equiv = foundry->style(key);
                if (equiv) continue;

                // does this style have an oblique equivalent?
                key.style = QFont::StyleOblique;
                equiv = foundry->style(key);
                if (equiv) continue;

                // let's fake one...
                equiv = foundry->style(key, true);
                equiv->smoothScalable = true;
		equiv->fakeOblique = true;

                QtFontSize *equiv_size = equiv->pixelSize(SMOOTH_SCALABLE, true);
                QtFontEncoding *equiv_enc = equiv_size->encodingID(-1, 0, 0, 0, 0, true);

                // keep the same pitch
                equiv_enc->pitch = enc->pitch;
            }
        }
    }
#endif


#ifdef QFONTDATABASE_DEBUG
#ifndef QT_NO_FONTCONFIG
    if (!X11->has_fontconfig)
#endif
        // load everything at startup in debug mode.
        loadXlfds(0,  -1);

    // print the database
    for (int f = 0; f < db->count; f++) {
        QtFontFamily *family = db->families[f];
        FD_DEBUG("'%s' %s  hasFT=%s", family->name.latin1(), (family->fixedPitch ? "fixed" : ""),
                 (family->hasFT ? "yes" : "no"));
        for (int i = 0; i < QFontDatabase::WritingSystemsCount; ++i) {
            QFontDatabase::WritingSystem ws = QFontDatabase::WritingSystem(i);
            FD_DEBUG("\t%s: %s", QFontDatabase::writingSystemName(ws).toLatin1().constData(),
                     ((family->writingSystems[i] & QtFontFamily::Supported) ? "Supported" :
                      (family->writingSystems[i] & QtFontFamily::Unsupported) == QtFontFamily::Unsupported ?
                      "Unsupported" : "Unknown"));
        }

        for (int fd = 0; fd < family->count; fd++) {
            QtFontFoundry *foundry = family->foundries[fd];
            FD_DEBUG("\t\t'%s'", foundry->name.latin1());
            for (int s = 0; s < foundry->count; s++) {
                QtFontStyle *style = foundry->styles[s];
                FD_DEBUG("\t\t\tstyle: style=%d weight=%d (%s)\n"
                         "\t\t\tstretch=%d (%s)",
                         style->key.style, style->key.weight,
                         style->weightName, style->key.stretch,
                         style->setwidthName ? style->setwidthName : "nil");
                if (style->smoothScalable)
                    FD_DEBUG("\t\t\t\tsmooth scalable");
                else if (style->bitmapScalable)
                    FD_DEBUG("\t\t\t\tbitmap scalable");
                if (style->pixelSizes) {
                    qDebug("\t\t\t\t%d pixel sizes",  style->count);
                    for (int z = 0; z < style->count; ++z) {
                        QtFontSize *size = style->pixelSizes + z;
                        for (int e = 0; e < size->count; ++e) {
                            FD_DEBUG("\t\t\t\t  size %5d pitch %c encoding %s",
                                      size->pixelSize,
                                      size->encodings[e].pitch,
                                      xlfd_for_id(size->encodings[e].encoding));
                        }
                    }
                }
            }
        }
    }
#endif // QFONTDATABASE_DEBUG
}


// --------------------------------------------------------------------------------------
// font loader
// --------------------------------------------------------------------------------------

#ifndef QT_NO_FONTCONFIG

static void addPatternProps(FcPattern *pattern, const QtFontStyle::Key &key,
                            bool fakeOblique, bool smoothScalable,
                            const QFontPrivate *fp, const QFontDef &request, int script)
{
    int weight_value = FC_WEIGHT_BLACK;
    if (key.weight == 0)
        weight_value = FC_WEIGHT_MEDIUM;
    else if (key.weight < (QFont::Light + QFont::Normal) / 2)
        weight_value = FC_WEIGHT_LIGHT;
    else if (key.weight < (QFont::Normal + QFont::DemiBold) / 2)
        weight_value = FC_WEIGHT_MEDIUM;
    else if (key.weight < (QFont::DemiBold + QFont::Bold) / 2)
        weight_value = FC_WEIGHT_DEMIBOLD;
    else if (key.weight < (QFont::Bold + QFont::Black) / 2)
        weight_value = FC_WEIGHT_BOLD;
    FcPatternAddInteger(pattern, FC_WEIGHT, weight_value);

    int slant_value = FC_SLANT_ROMAN;
    if (key.style == QFont::StyleItalic)
        slant_value = FC_SLANT_ITALIC;
    else if (key.style == QFont::StyleOblique && !fakeOblique)
        slant_value = FC_SLANT_OBLIQUE;
    FcPatternAddInteger(pattern, FC_SLANT, slant_value);

    double size_value = request.pixelSize;
    FcPatternAddDouble(pattern, FC_PIXEL_SIZE, size_value);

    if (!smoothScalable) {
#if FC_VERSION >= 20193
        int stretch = request.stretch;
        if (!stretch)
            stretch = 100;
	FcPatternAddInteger(pattern, FC_WIDTH, stretch);
#endif
    } else {
        FcPatternAddBool(pattern, FC_SCALABLE, FcTrue);

        if ((request.stretch > 0 && request.stretch != 100) ||
            (key.style == QFont::StyleOblique && fakeOblique)) {

            FcMatrix matrix;
            FcMatrixInit(&matrix);

            if (request.stretch > 0 && request.stretch != 100)
                FcMatrixScale(&matrix, double(request.stretch) / 100.0, 1.0);
            if (key.style == QFont::StyleOblique && fakeOblique)
                FcMatrixShear(&matrix, 0.20, 0.0);

            FcPatternAddMatrix(pattern, FC_MATRIX, &matrix);
        }
    }

    if (QX11Info::appDepth(fp->screen) <= 8) {
        // can't do antialiasing on 8bpp
        FcPatternAddBool(pattern, FC_ANTIALIAS, false);
    } else if (request.styleStrategy & (QFont::PreferAntialias|QFont::NoAntialias)) {
        FcPatternAddBool(pattern, FC_ANTIALIAS,
                         !(request.styleStrategy & QFont::NoAntialias));
    }

    if (script != QUnicodeTables::Common) {
        Q_ASSERT(script < QUnicodeTables::ScriptCount);
        FcLangSet *ls = FcLangSetCreate();
        FcLangSetAdd(ls, (const FcChar8*)specialLanguages[script]);
        FcPatternAddLangSet(pattern, FC_LANG, ls);
        FcLangSetDestroy(ls);
    }
}

static void FcFontSetRemove(FcFontSet *fs, int at)
{
    Q_ASSERT(at < fs->nfont);
    FcPatternDestroy(fs->fonts[at]);
    int len = (--fs->nfont - at) * sizeof(FcPattern *);;
    if (len > 0)
        memmove(fs->fonts + at, fs->fonts + at + 1, len);
}

static QFontEngine *loadFcEngineFromPattern(FcPattern *pattern, const QFontPrivate *fp,
                                            const QFontDef &request, int script)
{
    FcBool forceScalable = true;
    FcPatternGetBool(pattern, FC_SCALABLE, 0, &forceScalable);

    FcDefaultSubstitute(pattern);
    FcConfigSubstitute(0, pattern, FcMatchPattern);
    FcConfigSubstitute(0, pattern, FcMatchFont);

#ifdef FONT_MATCH_DEBUG
    printf("forceScalable=%d", forceScalable);
    printf("final FcPattern contains:\n");
    FcPatternPrint(pattern);
#endif

    FcResult result;
    FcFontSet *fs = FcFontSort(0, pattern, FcTrue, 0, &result);
    FcPatternDestroy(pattern);
    if (!fs)
        return 0;

    double size_value = request.pixelSize;

    // remove fonts if their size isn't close enough, or if they are
    // not scalable (and should be)
    for (int i = 0; i < fs->nfont; ++i) {
        FcPattern *font = fs->fonts[i];
        FcResult res;
        FcBool scalable;
        res = FcPatternGetBool(font, FC_SCALABLE, 0, &scalable);
        if (res != FcResultMatch || !scalable) {
            if (forceScalable) {
                FcFontSetRemove(fs, i);
#ifdef FONT_MATCH_DEBUG
                printf("removing pattern:");
                FcPatternPrint(font);
#endif
                --i; // go back one
            } else {
                double pixelSize;
                res = FcPatternGetDouble(font, FC_PIXEL_SIZE, 0, &pixelSize);
                if (res != FcResultMatch || qAbs((size_value-pixelSize)/size_value) > 0.2) {
                    FcFontSetRemove(fs, i);
#ifdef FONT_MATCH_DEBUG
                    printf("removing pattern:");
                    FcPatternPrint(font);
#endif
                    --i; // go back one
                }
            }
        }
    }

    QtFontStyle::Key key;
    key.style = request.style;
    key.weight = request.weight;
    key.stretch = request.stretch;

    QFontEngine *fe = 0;
    if (script != QUnicodeTables::Common) {
        // load a single font for the script

        for (int i = 0; !fe && i < fs->nfont; ++i) {
            // skip font if it doesn't support the language we want
            FcLangSet *langSet = 0;
            if (FcPatternGetLangSet(fs->fonts[i], FC_LANG, 0, &langSet) != FcResultMatch)
                continue;
            if (FcLangSetHasLang(langSet, (const FcChar8*)specialLanguages[script]) != FcLangEqual)
                continue;

            FcPattern *pattern = FcPatternDuplicate(fs->fonts[i]);
            // add properties back in as the font selected from the
            // list doesn't contain them.
            addPatternProps(pattern, key, false, true, fp, request, script);

            FcConfigSubstitute(0, pattern, FcMatchPattern);
            FcDefaultSubstitute(pattern);
            FcResult res;
            FcPattern *match = FcFontMatch(0, pattern, &res);
            QFontEngineFT *engine = new QFontEngineFT(match, request, fp->screen);
            if (engine->invalid())
                delete engine;
            else
                fe = engine;
        }
    } else {
        // create a multi engine for the fontset fontconfig gave us
        FcFontSet *fontSet = FcFontSetCreate();
        for (int i = 0; i < fs->nfont; ++i) {
            FcPattern *pattern = FcPatternDuplicate(fs->fonts[i]);
            // add properties back in as the font selected from the
            // list doesn't contain them.
            addPatternProps(pattern, key, false, true, fp, request, script);
            FcFontSetAdd(fontSet, pattern);
        }

        fe = new QFontEngineMultiFT(fontSet, fp->screen);
        fe->fontDef = request;
    }

    FcFontSetDestroy(fs);
    return fe;
}

#endif // QT_NO_FONTCONFIG

static
QFontEngine *loadEngine(int script,
                        const QFontPrivate *fp, const QFontDef &request,
                        QtFontFamily *family, QtFontFoundry *foundry,
                        QtFontStyle *style, QtFontSize *size,
                        QtFontEncoding *encoding, bool forced_encoding)
{
    Q_UNUSED(script);

    if (fp && fp->rawMode) {
        QByteArray xlfd = request.family.toLatin1();
        FM_DEBUG("Loading XLFD (rawmode) '%s'", xlfd.data());

        XFontStruct *xfs;
        if (! (xfs = XLoadQueryFont(QX11Info::display(), xlfd.data())))
            return 0;

        QFontEngine *fe = new QFontEngineXLFD(xfs, xlfd.data(), 0);
        if (! qt_fillFontDef(xfs, &fe->fontDef, fp->dpi) &&
            ! qt_fillFontDef(xlfd, &fe->fontDef, fp->dpi))
            fe->fontDef = QFontDef();

        return fe;
    }

#ifndef QT_NO_FONTCONFIG
    if (X11->has_fontconfig && encoding->encoding == -1) {

        FM_DEBUG("    using FontConfig");

        FcPattern *pattern = FcPatternCreate();
        if (!pattern)
            return 0;

        if (!foundry->name.isEmpty()) {
            FcPatternAddString(pattern, FC_FOUNDRY,
                               (const FcChar8 *)foundry->name.toUtf8().constData());
        }

        QStringList familyList;
        if (!family->rawName.isEmpty()) {
            FcPatternAddString(pattern, FC_FAMILY,
                               (const FcChar8 *)family->rawName.toUtf8().constData());
            familyList << family->name;
        }

        QString stylehint;
        switch (request.styleHint) {
        case QFont::SansSerif:
            stylehint = "sans-serif";
            break;
        case QFont::Serif:
            stylehint = "serif";
            break;
        case QFont::TypeWriter:
            stylehint = "monospace";
            break;
        default:
            if (request.fixedPitch)
                stylehint = "monospace";
            break;
        }
        if (!stylehint.isEmpty()) {
            FcPatternAddString(pattern, FC_FAMILY,
                               (const FcChar8 *)stylehint.toUtf8().constData());
            familyList << family->name;
        }

        FcValue value;
        value.type = FcTypeString;

        // these should only get added to the pattern _after_ substitution
        // append the default fallback font for the specified script
        extern QString qt_fallback_font_family(int);
        QString fallback = qt_fallback_font_family(script);
        if (!fallback.isEmpty() && !familyList.contains(fallback)) {
            QByteArray cs = fallback.toUtf8();
            value.u.s = (const FcChar8 *)cs.data();
            FcPatternAddWeak(pattern, FC_FAMILY, value, FcTrue);
        }

        // add the default family
        QString defaultFamily = QApplication::font().family();
        if (!familyList.contains(defaultFamily)) {
            QByteArray cs = defaultFamily.toUtf8();
            value.u.s = (const FcChar8 *)cs.data();
            FcPatternAddWeak(pattern, FC_FAMILY, value, FcTrue);
        }

        // add QFont::defaultFamily() to the list, for compatibility with
        // previous versions
        defaultFamily = QApplication::font().defaultFamily();
        if (!familyList.contains(defaultFamily)) {
            QByteArray cs = defaultFamily.toUtf8();
            value.u.s = (const FcChar8 *)cs.data();
            FcPatternAddWeak(pattern, FC_FAMILY, value, FcTrue);
        }

        char pitch_value = (encoding->pitch == 'c' ? FC_CHARCELL :
                            (encoding->pitch == 'm' ? FC_MONO : FC_PROPORTIONAL));
        FcPatternAddInteger(pattern, FC_SPACING, pitch_value);
        FcPatternAddBool(pattern, FC_SCALABLE, style->smoothScalable);

        addPatternProps(pattern, style->key, style->fakeOblique, style->smoothScalable,
                        fp, request, script);
        return loadFcEngineFromPattern(pattern, fp, request, script);
    }
#endif // QT_NO_FONTCONFIG

    int px = size->pixelSize;
    if (style->smoothScalable && px == SMOOTH_SCALABLE)
        px = request.pixelSize;
    else if (style->bitmapScalable && px == 0)
        px = request.pixelSize;

    QFontEngine *fe = 0;
    if (forced_encoding || script != QUnicodeTables::Common) {
        QByteArray xlfd("-");
        xlfd += foundry->name.isEmpty() ? QByteArray("*") : foundry->name.toLatin1();
        xlfd += "-";
        xlfd += family->name.isEmpty() ? QByteArray("*") : family->name.toLatin1();
        xlfd += "-";
        xlfd += style->weightName ? style->weightName : "*";
        xlfd += "-";
        xlfd += (style->key.style == QFont::StyleItalic
                 ? "i"
                 : (style->key.style == QFont::StyleOblique ? "o" : "r"));
        xlfd += "-";
        xlfd += style->setwidthName ? style->setwidthName : "*";
        // ### handle add-style
        xlfd += "-*-";
        xlfd += QByteArray::number(px);
        xlfd += "-";
        xlfd += QByteArray::number(encoding->xpoint);
        xlfd += "-";
        xlfd += QByteArray::number(encoding->xres);
        xlfd += "-";
        xlfd += QByteArray::number(encoding->yres);
        xlfd += "-";
        xlfd += encoding->pitch;
        xlfd += "-";
        xlfd += QByteArray::number(encoding->avgwidth);
        xlfd += "-";
        xlfd += xlfd_for_id(encoding->encoding);

        FM_DEBUG("    using XLFD: %s\n", xlfd.data());

        const int mib = xlfd_encoding[encoding->encoding].mib;
        XFontStruct *xfs;
        if (! (xfs = XLoadQueryFont(QX11Info::display(), xlfd)))
            return 0;
        fe = new QFontEngineXLFD(xfs, xlfd, mib);
    } else {
        QList<int> encodings;
        encodings.append(int(encoding->encoding));

        // append all other encodings for the matched font
        for (int i = 0; i < style->count; ++i) {
            QtFontEncoding *e = size->encodings + i;
            if (e == encoding)
                break;
            encodings.append(int(e->encoding));
        }
        // fill in the missing encodings
        const XlfdEncoding *enc = xlfd_encoding;
        for (; enc->name; ++enc) {
            if (!encodings.contains(enc->id))
                encodings.append(enc->id);
        }

#if defined(FONT_MATCH_DEBUG)
        FM_DEBUG("    using MultiXLFD, encodings:");
        for (int i = 0; i < encodings.size(); ++i) {
            const int id = encodings.at(i);
            FM_DEBUG("      %2d: %s", xlfd_encoding[id].id, xlfd_encoding[id].name);
        }
#endif

        fe = new QFontEngineMultiXLFD(request, encodings, fp->screen);
    }
    return fe;
}
