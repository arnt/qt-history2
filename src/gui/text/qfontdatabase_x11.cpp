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

#ifndef QT_NO_XFT
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#ifdef QFONTDATABASE_DEBUG
#  define FD_DEBUG qDebug
#else
#  define FD_DEBUG if (false) qDebug
#endif // QFONTDATABASE_DEBUG

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


// ----- begin of generated code -----

#define make_tag(c1, c2, c3, c4) \
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
#define LAST_LATIN_ENCODING 9
    { "iso8859-5", 10, 8, make_tag('i','s','o','8'), make_tag('5','9','-','5') },
    { "*-cp1251", 11, 2251, 0, make_tag('1','2','5','1') },
    { "koi8-ru", 12, 2084, make_tag('k','o','i','8'), make_tag('8','-','r','u') },
    { "koi8-u", 13, 2088, make_tag('k','o','i','8'), make_tag('i','8','-','u') },
    { "koi8-r", 14, 2084, make_tag('k','o','i','8'), make_tag('i','8','-','r') },
    { "iso8859-7", 15, 10, make_tag('i','s','o','8'), make_tag('5','9','-','7') },
    { "iso10646-1", 16, 0, make_tag('i','s','o','1'), make_tag('4','6','-','1') },
    { "iso8859-8", 17, 85, make_tag('i','s','o','8'), make_tag('5','9','-','8') },
    { "gb18030-0", 18, -114, make_tag('g','b','1','8'), make_tag('3','0','-','0') },
    { "gb18030.2000-0", 19, -113, make_tag('g','b','1','8'), make_tag('0','0','-','0') },
    { "gbk-0", 20, -113, make_tag('g','b','k','-'), make_tag('b','k','-','0') },
    { "gb2312.*-0", 21, 57, make_tag('g','b','2','3'), 0 },
    { "jisx0201*-0", 22, 15, make_tag('j','i','s','x'), 0 },
    { "jisx0208*-0", 23, 63, make_tag('j','i','s','x'), 0 },
    { "ksc5601*-*", 24, 36, make_tag('k','s','c','5'), 0 },
    { "big5hkscs-0", 25, -2101, make_tag('b','i','g','5'), make_tag('c','s','-','0') },
    { "hkscs-1", 26, -2101, make_tag('h','k','s','c'), make_tag('c','s','-','1') },
    { "big5*-*", 27, -2026, make_tag('b','i','g','5'), 0 },
    { "tscii-*", 28, 2028, make_tag('t','s','c','i'), 0 },
    { "tis620*-*", 29, 2259, make_tag('t','i','s','6'), 0 },
    { "iso8859-11", 30, 2259, make_tag('i','s','o','8'), make_tag('9','-','1','1') },
    { "mulelao-1", 31, -4242, make_tag('m','u','l','e'), make_tag('a','o','-','1') },
    { "ethiopic-unicode", 32, 0, make_tag('e','t','h','i'), make_tag('c','o','d','e') },
    { "unicode-*", 33, 0, make_tag('u','n','i','c'), 0 },
    { "*-symbol", 34, 0, 0, make_tag('m','b','o','l') },
    { "*-fontspecific", 35, 0, 0, make_tag('i','f','i','c') },
    { "fontspecific-*", 36, 0, make_tag('f','o','n','t'), 0 },
    { 0, 0, 0, 0, 0 }
};

static const char scripts_for_xlfd_encoding[37][61] = {
    // iso8859-1
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-2
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-3
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-4
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-9
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-10
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-13
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-14
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-15
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // hp-roman8
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-5
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // *-cp1251
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // koi8-ru
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // koi8-u
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // koi8-r
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-7
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso10646-1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-8
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // gb18030-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0 },
    // gb18030.2000-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0 },
    // gbk-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0 },
    // gb2312.*-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0 },
    // jisx0201*-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // jisx0208*-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0 },
    // ksc5601*-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1 },
    // big5hkscs-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0 },
    // hkscs-1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0 },
    // big5*-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0 },
    // tscii-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // tis620*-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // iso8859-11
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // mulelao-1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // ethiopic-unicode
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // unicode-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0 },
    // *-symbol
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
      0 },
    // *-fontspecific
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
      0 },
    // fontspecific-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
      0 }

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

  Returns true if the the given xlfd is valid.  The fields lbearing
  and rbearing are not given any values.
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
        fd->addStyle = QString::null;

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
        fd->pointSize = qRound(qt_pointSize(fd->pixelSize, dpi) * 10.);
    } else if (fd->pixelSize == 0 && fd->pointSize) {
        // calculate pixel size from pointsize/dpi
        fd->pixelSize = qRound(qt_pixelSize(fd->pointSize / 10., dpi));
    }

    return true;
}

/*
  Fills in a font definition (QFontDef) from the font properties in an
  XFontStruct.

  Returns true if the QFontDef could be filled with properties from
  the XFontStruct.  The fields lbearing and rbearing are not given any
  values.
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
    QtFontFamily *fontFamily = reqFamily ? db->family(reqFamily) : 0;

    // make sure we don't load twice
    if ((encoding_id == -1 && xlfdsFullyLoaded) || (encoding_id != -1 && encodingLoaded[encoding_id]))
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
        if (! parseXFontName(fontList[i], tokens)) continue;

        // get the encoding_id for this xlfd.  we need to do this
        // here, since we can pass -1 to this function to do full
        // database population
        *(tokens[CharsetEncoding]-1) = '-';
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

        for (int script = 0; script < QFont::LastPrivateScript; ++script) {
            if (scripts_for_xlfd_encoding[encoding_id][script])
                family->scripts[script] = QtFontFamily::Supported;
            else
                family->scripts[script] |= QtFontFamily::UnSupported_Xlfd;
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


#ifndef QT_NO_XFT
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

static void loadXft()
{
    if (!X11->has_xft)
        return;

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

    fonts =
        XftListFonts(QX11Info::display(),
                     QX11Info::appScreen(),
                     (const char *)0,
                     FC_FAMILY, FC_WEIGHT, FC_SLANT,
                     FC_SPACING, FC_FILE, FC_INDEX,
                     FC_CHARSET, FC_FOUNDRY, FC_SCALABLE, FC_PIXEL_SIZE, FC_WEIGHT,
#if FC_VERSION >= 20193
                     FC_WIDTH,
#endif
                     (const char *)0);

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
        family->hasXft = true;

        FcCharSet *charset = 0;
        FcResult res = FcPatternGetCharSet(fonts->fonts[i], FC_CHARSET, 0, &charset);
	if (res == FcResultMatch && FcCharSetCount(charset) > 1) {
            for (int i = 0; i < QFont::LastPrivateScript; ++i) {
                QChar ch = sampleCharacter((QFont::Script) i);

                if (ch.unicode() != 0 &&
                    FcCharSetHasChar(charset, ch.unicode())) {
                    family->scripts[i] = QtFontFamily::Supported;
                } else {
                    family->scripts[i] |= QtFontFamily::UnSupported_Xft;
                }
            }
            family->xftScriptCheck = true;
 	} else {
 	    // we set UnknownScript to supported for symbol fonts. It makes no sense to merge these
 	    // with other ones, as they are special in a way.
 	    for ( int i = 0; i < QFont::LastPrivateScript; ++i )
 		family->scripts[i] |= QtFontFamily::UnSupported_Xft;
 	    family->scripts[QFont::UnknownScript] = QtFontFamily::Supported;
 	}

        QByteArray file((const char *)file_value);
        family->fontFilename = file;
        family->fontFileIndex = index_value;

        QtFontStyle::Key styleKey;
        styleKey.style = (slant_value == FC_SLANT_ITALIC) ? QFont::StyleItalic
                         : ((slant_value == FC_SLANT_OBLIQUE) ? QFont::StyleOblique : QFont::StyleNormal);
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
        family->hasXft = true;
        family->synthetic = true;
        QtFontFoundry *foundry = family->foundry(QString::null,  true);

        for (int i = 0; i < QFont::LastPrivateScript; ++i) {
            if (i == QFont::UnknownScript)
                continue;
            family->scripts[i] = QtFontFamily::Supported;
        }

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
#endif // QT_NO_XFT

static void load(const QString &family = QString::null, int script = -1)
{
    if (X11->has_xft)
        return;

#ifdef QFONTDATABASE_DEBUG
    QTime t;
    t.start();
#endif

    if (family.isNull()) {
        if (script == -1)
            loadXlfds(0, -1);
        else {
            for (int i = 0; i < numEncodings; i++) {
                if (scripts_for_xlfd_encoding[i][script])
                    loadXlfds(0, i);
            }
        }
    } else {
        QtFontFamily *f = db->family(family, true);
        if (!f->fullyLoaded) {
            // could reduce this further with some more magic:
            // would need to remember the encodings loaded for the family.
            if ((script == -1 && !f->xlfdLoaded) ||
                 (!f->hasXft && !(f->scripts[script] & QtFontFamily::Supported) &&
                   !(f->scripts[script] & QtFontFamily::UnSupported_Xlfd))) {
                loadXlfds(family.latin1(), -1);
                f->fullyLoaded = true;
            }
        }
    }

#ifdef QFONTDATABASE_DEBUG
    FD_DEBUG("QFontDatabase: load(%s, %d) took %d ms", family.latin1(), script, t.elapsed());
#endif
}


static void initializeDb()
{
    if (db) return;
    db = new QFontDatabasePrivate;
    qfontdatabase_cleanup.set(&db);

    QTime t;
    t.start();

#ifndef QT_NO_XFT
    loadXft();
    FD_DEBUG("QFontDatabase: loaded Xft: %d ms",  t.elapsed());
#endif

    t.start();

#ifndef QT_NO_XFT
    for (int i = 0; i < db->count; i++) {
#ifdef XFT_MATRIX
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
#endif // XFT_MATRIX
    }
#endif


#ifdef QFONTDATABASE_DEBUG
#ifndef QT_NO_XFT
    if (!X11->has_xft)
#endif
        // load everything at startup in debug mode.
        loadXlfds(0,  -1);

    // print the database
    for (int f = 0; f < db->count; f++) {
        QtFontFamily *family = db->families[f];
        FD_DEBUG("'%s' %s  hasXft=%s", family->name.latin1(), (family->fixedPitch ? "fixed" : ""),
                 (family->hasXft ? "yes" : "no"));
        for (int i = 0; i < QFont::LastPrivateScript; ++i) {
            FD_DEBUG("\t%s: %s", QFontDatabase::scriptName((QFont::Script) i).latin1(),
                     ((family->scripts[i] & QtFontFamily::Supported) ? "Supported" :
                      (family->scripts[i] & QtFontFamily::UnSupported) == QtFontFamily::UnSupported ?
                      "UnSupported" : "Unknown"));
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
#define MAXFONTSIZE_XFT 256
#define MAXFONTSIZE_XLFD 128
#ifndef QT_NO_XFT
static double addPatternProps(FcPattern *pattern, const QtFontStyle::Key &key, bool fakeOblique, bool smoothScalable,
                              const QFontPrivate *fp, const QFontDef &request, QFont::Script script)
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

    /*
      ##### do we still need this as we ignore Xft1 now?
      Xft1 doesn't obey user settings for turning off anti-aliasing using
      the following:

      match any size > 6 size < 12 edit antialias = false;

      ... if we request pixel sizes.  so, work around this limitiation and
      convert the pixel size to a point size and request that.
    */
    double size_value = request.pixelSize;
    double scale = 1.;
    if (size_value > MAXFONTSIZE_XFT) {
        scale = (double)size_value/(double)MAXFONTSIZE_XFT;
        size_value = MAXFONTSIZE_XFT;
    }

    size_value = size_value*72./QX11Info::appDpiY(fp->screen);
    FcPatternAddDouble(pattern, FC_SIZE, size_value);

    if (!smoothScalable) {
#if FC_VERSION >= 20193
        int stretch = request.stretch;
        if (!stretch)
            stretch = 100;
	FcPatternAddInteger(pattern, FC_WIDTH, stretch);
#endif
    } else if ((request.stretch > 0 && request.stretch != 100) ||
               (key.style == QFont::StyleOblique && fakeOblique)) {
        FcMatrix matrix;
        FcMatrixInit(&matrix);

        if (request.stretch > 0 && request.stretch != 100)
            FcMatrixScale(&matrix, double(request.stretch) / 100.0, 1.0);
        if (key.style == QFont::StyleOblique && fakeOblique)
            FcMatrixShear(&matrix, 0.20, 0.0);

        FcPatternAddMatrix(pattern, FC_MATRIX, &matrix);
    }

    if (QX11Info::appDepth(fp->screen) <= 8) {
        // Xft can't do antialiasing on 8bpp
        FcPatternAddBool(pattern, FC_ANTIALIAS, false);
    } else if (request.styleStrategy & (QFont::PreferAntialias|QFont::NoAntialias)) {
        FcPatternAddBool(pattern, FC_ANTIALIAS,
                         !(request.styleStrategy & QFont::NoAntialias));
    }

    if (script != QFont::Unicode) {
        FcCharSet *cs = FcCharSetCreate();
        QChar sample = sampleCharacter(script);
        FcCharSetAddChar(cs, sample.unicode());
        if (script == QFont::Latin)
            // add Euro character
            FcCharSetAddChar(cs, 0x20ac);
        FcPatternAddCharSet(pattern, FC_CHARSET, cs);
        FcCharSetDestroy(cs);
    }
    return scale;
}
#endif // QT_NO_XFT

static
QFontEngine *loadEngine(QFont::Script script,
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

#ifndef QT_NO_XFT
    if (X11->has_xft && encoding->encoding == -1) {

        FM_DEBUG("    using Xft");

        FcPattern *pattern = FcPatternCreate();
        if (!pattern) return 0;

        bool symbol = (family->scripts[QFont::UnknownScript] == QtFontFamily::Supported);

        if (!foundry->name.isEmpty())
            FcPatternAddString(pattern, FC_FOUNDRY, (const FcChar8 *)foundry->name.utf8());

        if (!family->rawName.isEmpty())
            FcPatternAddString(pattern, FC_FAMILY, (const FcChar8 *)family->rawName.utf8());

        char pitch_value = (encoding->pitch == 'c' ? FC_CHARCELL :
                             (encoding->pitch == 'm' ? FC_MONO : FC_PROPORTIONAL));
        FcPatternAddInteger(pattern, FC_SPACING, pitch_value);

        double scale = addPatternProps(pattern, style->key, style->fakeOblique, style->smoothScalable, fp, request, script);

 	if (!symbol) {
 	    FcCharSet *cs = FcCharSetCreate();
	    QChar sample = sampleCharacter(script);
 	    FcCharSetAddChar(cs, sample.unicode());
 	    if (script == QFont::Latin)
 		// add Euro character
 		FcCharSetAddChar(cs, 0x20ac);
 	    FcPatternAddCharSet(pattern, FC_CHARSET, cs);
 	    FcCharSetDestroy(cs);
 	}

        FcResult res;
        FcPattern *result =
            XftFontMatch(QX11Info::display(), fp->screen, pattern, &res);

 	if (result && script == QFont::Latin) {
 	    // since we added the Euro char on top, check we actually got the family
 	    // we requested. If we didn't get it correctly, remove the Euro from the pattern
	    // and try again.
 	    FcChar8 *f;
 	    res = FcPatternGetString(result, FC_FAMILY, 0, &f);
 	    if (res == FcResultMatch && QString::fromUtf8((char *)f) != family->rawName) {
 		FcPatternDel(pattern, FC_CHARSET);
 		FcCharSet *cs = FcCharSetCreate();
 		QChar sample = sampleCharacter(script);
 		FcCharSetAddChar(cs, sample.unicode());
 		FcPatternAddCharSet(pattern, FC_CHARSET, cs);
 		FcCharSetDestroy(cs);
 		result = XftFontMatch(QX11Info::display(), fp->screen, pattern, &res);
 	    }
 	}
	FcPatternDestroy(pattern);
	if (!result)
	    return 0;

        // We pass a duplicate to XftFontOpenPattern because either xft font
        // will own the pattern after the call or the pattern will be
        // destroyed.
        FcPattern *dup = FcPatternDuplicate(result);
        XftFont *xftfs = XftFontOpenPattern(QX11Info::display(), dup);

        if (! xftfs) // Xft couldn't find a font?
            return 0;

        QFontEngine *fe = new QFontEngineXft(xftfs, result, symbol ? 1 : 0);
        if (fp->dpi != QX11Info::appDpiY(fp->screen)) {
            double px;
            FcPatternGetDouble(result, FC_PIXEL_SIZE, 0, &px);
            scale = (double)request.pixelSize/px;
        }
        fe->setScale(scale);
        return fe;
    }
#endif // QT_NO_XFT

    FM_DEBUG("    using XLFD");

    QByteArray xlfd("-");
    xlfd += foundry->name.isEmpty() ? "*" : foundry->name.latin1();
    xlfd += "-";
    xlfd += family->name.isEmpty() ? "*" : family->name.latin1();

    xlfd += "-";
    xlfd += style->weightName ? style->weightName : "*";
    xlfd += "-";
    xlfd += (style->key.style == QFont::StyleItalic ? "i" : (style->key.style == QFont::StyleOblique ? "o" : "r"));

    xlfd += "-";
    xlfd += style->setwidthName ? style->setwidthName : "*";
    // ### handle add-style
    xlfd += "-*-";

    int px = size->pixelSize;
    if (style->smoothScalable && px == SMOOTH_SCALABLE)
        px = request.pixelSize;
    else if (style->bitmapScalable && px == 0)
        px = request.pixelSize;
    double scale = 1.;
    if (px > MAXFONTSIZE_XLFD) {
        scale = (double)px/(double)MAXFONTSIZE_XLFD;
        px = MAXFONTSIZE_XLFD;
    }
    if (fp && fp->dpi != QX11Info::appDpiY())
        scale = (double)request.pixelSize/(double)px;

    xlfd += QByteArray::number(px);
    xlfd += "-";
    xlfd += QByteArray::number(encoding->xpoint);
    xlfd += "-";
    xlfd += QByteArray::number(encoding->xres);
    xlfd += "-";
    xlfd += QByteArray::number(encoding->yres);
    xlfd += "-";

    // ### handle cell spaced fonts
    xlfd += encoding->pitch;
    xlfd += "-";
    xlfd += QByteArray::number(encoding->avgwidth);
    xlfd += "-";
    xlfd += xlfd_for_id(encoding->encoding);

    FM_DEBUG("    xlfd: '%s'", xlfd.data());

    XFontStruct *xfs;
    if (! (xfs = XLoadQueryFont(QX11Info::display(), xlfd)))
        return 0;

    QFontEngine *fe = 0;
    const int mib = xlfd_encoding[encoding->encoding].mib;
    if (encoding->encoding <= LAST_LATIN_ENCODING && !forced_encoding) {
        fe = new QFontEngineLatinXLFD(xfs, xlfd, mib);
    } else {
        fe = new QFontEngineXLFD(xfs, xlfd, mib);
    }

    fe->setScale(scale);

    return fe;
}

#ifndef QT_NO_XFT
static void parseFontName(const QString &name, QString &foundry, QString &family)
{
    if (name.contains('[') && name.contains(']')) {
        int i = name.indexOf('[');
        int li = name.lastIndexOf(']');

        if (i < li) {
            foundry = name.mid(i + 1, li - i - 1);
            if (name[i - 1] == ' ')
                i--;
            family = name.left(i);
        }
    } else {
        foundry = QString::null;
        family = name;
    }
}

static QFontEngine *loadFontConfigFont(const QFontPrivate *fp, const QFontDef &request, QFont::Script script)
{
    if (!X11->has_xft)
	return 0;

    QStringList family_list;
    if (request.family.isEmpty()) {
	family_list = fp->request.family.split(QChar(','));

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
        if (!stylehint.isEmpty())
            family_list << stylehint;
    } else {
        family_list << request.family;
    }

    FcPattern *pattern = FcPatternCreate();

    {
        QString family, foundry;
        for (QStringList::ConstIterator it = family_list.begin(); it != family_list.end(); ++it) {
            parseFontName(*it, foundry, family);
            FcPatternAddString(pattern, FC_FAMILY, (const FcChar8 *)family.utf8());
        }
    }

    QtFontStyle::Key key;
    key.style = request.style;
    key.weight = request.weight;
    key.stretch = request.stretch;

    double scale = addPatternProps(pattern, key, false, true, fp, request, script);
#ifdef FONT_MATCH_DEBUG
    qDebug("original pattern contains:");
    FcPatternPrint(pattern);
#endif

    FcConfigSubstitute(0, pattern, FcMatchPattern);
//     qDebug("1: pattern contains:");
//     FcPatternPrint(pattern);

    {
        FcValue value;
        value.type = FcTypeString;

        // these should only get added to the pattern _after_ substitution
        // append the default fallback font for the specified script
        extern QString qt_fallback_font_family(QFont::Script);
        QString fallback = qt_fallback_font_family(script);
        if (! fallback.isEmpty() && ! family_list.contains(fallback)) {
            QByteArray cs = fallback.toUtf8();
            value.u.s = (const FcChar8 *)cs.data();
            FcPatternAddWeak(pattern, FC_FAMILY, value, FcTrue);
        }

        // add the default family
        QString defaultFamily = QApplication::font().family();
        if (! family_list.contains(defaultFamily)) {
            QByteArray cs = defaultFamily.toUtf8();
            value.u.s = (const FcChar8 *)cs.data();
            FcPatternAddWeak(pattern, FC_FAMILY, value, FcTrue);
        }

        // add QFont::defaultFamily() to the list, for compatibility with
        // previous versions
        defaultFamily = QApplication::font().defaultFamily();
        if (! family_list.contains(defaultFamily)) {
            QByteArray cs = defaultFamily.toUtf8();
            value.u.s = (const FcChar8 *)cs.data();
            FcPatternAddWeak(pattern, FC_FAMILY, value, FcTrue);
        }
#ifdef FONT_MATCH_DEBUG
        printf("final pattern contains:\n");
        FcPatternPrint(pattern);
#endif
    }

    if (script != QFont::Unicode) {
        FcCharSet *cs = FcCharSetCreate();
        QChar sample = sampleCharacter(script);
        FcCharSetAddChar(cs, sample.unicode());
        if (script == QFont::Latin)
            // add Euro character
            FcCharSetAddChar(cs, 0x20ac);
        FcPatternAddCharSet(pattern, FC_CHARSET, cs);
        FcCharSetDestroy(cs);
    }

    FcDefaultSubstitute(pattern);

    FcResult result;
    FcFontSet *fs = FcFontSort(0, pattern, FcTrue, 0, &result);
    FcPatternDestroy(pattern);
    if (!fs)
	return 0;
#ifdef FONT_MATCH_DEBUG
    printf("fontset contains:\n");
    for (int i = 0; i < fs->nfont; ++i) {
        FcPattern *test = fs->fonts[i];
        FcChar8 *fam;
        FcPatternGetString(test, FC_FAMILY, 0, &fam);
        printf("    %s\n", fam);
    }
#endif

    int ch = sampleCharacter(script).unicode();
    double size_value = request.pixelSize;
    if (size_value > MAXFONTSIZE_XFT)
        size_value = MAXFONTSIZE_XFT;

    QFontEngine *fe = 0;

    for (int i = 0; i < fs->nfont; ++i) {
        FcPattern *font = fs->fonts[i];
        FcCharSet *cs;
        FcResult res = FcPatternGetCharSet(font, FC_CHARSET, 0, &cs);
        if (res != FcResultMatch)
            continue;
        if (!FcCharSetHasChar(cs, ch))
            continue;
        FcBool scalable;
        res = FcPatternGetBool(font, FC_SCALABLE, 0, &scalable);
        if (res != FcResultMatch || !scalable) {
            int pixelSize;
            res = FcPatternGetInteger(font, FC_PIXEL_SIZE, 0, &pixelSize);
            if (res != FcResultMatch || qAbs((size_value-pixelSize)/size_value) > 0.2)
                continue;
        }

        FcPattern *pattern = FcPatternDuplicate(font);
        // add properties back in as the font selected from the list doesn't contain them.
        addPatternProps(pattern, key, false, true, fp, request, script);

        FcPattern *result = XftFontMatch(QX11Info::display(), fp->screen, pattern, &res);
        FcPatternDestroy(pattern);

        // We pass a duplicate to XftFontOpenPattern because either xft font
        // will own the pattern after the call or the pattern will be
        // destroyed.
        FcPattern *dup = FcPatternDuplicate(result);
        XftFont *xftfs = XftFontOpenPattern(QX11Info::display(), dup);

        if (!xftfs) {
            // Xft couldn't find a font?
            qDebug("couldn't open fontconfigs chosen font with Xft!!!");
        } else {
            fe = new QFontEngineXft(xftfs, result, 0);
            if (fp->dpi != QX11Info::appDpiY()) {
                double px;
                FcPatternGetDouble(result, FC_PIXEL_SIZE, 0, &px);
                scale = request.pixelSize/px;
            }
            fe->setScale(scale);
            fe->fontDef = request;
            if (script != QFont::Unicode && !canRender(fe, script)) {
                FM_DEBUG("  WARN: font loaded cannot render sample 0x%04x", ch);
                delete fe;
                fe = 0;
            }
        }
        if (fe)
            break;
    }
    FcFontSetDestroy(fs);
    return fe;
}
#endif // QT_NO_XFT
