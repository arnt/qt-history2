/****************************************************************************
**
** Implementation of platform specific QFontDatabase.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qplatformdefs.h>

#include <qdatetime.h>
#include <qpaintdevicemetrics.h>
#include <qpaintdevice.h>

#include "qt_x11_p.h"

#include <ctype.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#ifndef QT_NO_XFTFREETYPE
#include <freetype/freetype.h>
#endif

#ifdef QFONTDATABASE_DEBUG
#  define FD_DEBUG qDebug
#else
#  define FD_DEBUG if (FALSE) qDebug
#endif // QFONTDATABASE_DEBUG


// ----- begin of generated code -----

#define make_tag( c1, c2, c3, c4 ) \
( (((unsigned int)c1)<<24) | (((unsigned int)c2)<<16) | \
(((unsigned int)c3)<<8) | ((unsigned int)c4) )

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

static const char scripts_for_xlfd_encoding[37][59] = {
    // iso8859-1
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-2
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-3
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-4
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-9
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-10
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-13
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-14
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-15
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // hp-roman8
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-5
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // *-cp1251
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // koi8-ru
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // koi8-u
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // koi8-r
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-7
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso10646-1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-8
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // gb18030-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 0, 0 },
    // gb18030.2000-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 0, 0 },
    // gbk-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 0, 0 },
    // gb2312.*-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 0, 0 },
    // jisx0201*-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 1, 0, 0, 0, 0, 0, 0 },
    // jisx0208*-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 0, 0 },
    // ksc5601*-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1 },
    // big5hkscs-0
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0 },
    // hkscs-1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0 },
    // big5*-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0 },
    // tscii-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // tis620*-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // iso8859-11
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // mulelao-1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // ethiopic-unicode
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // unicode-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // *-symbol
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0, 0 },
    // *-fontspecific
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0, 0 },
    // fontspecific-*
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0, 0 }

};

// ----- end of generated code -----


const int numEncodings = sizeof( xlfd_encoding ) / sizeof( XlfdEncoding ) - 1;

int qt_xlfd_encoding_id( const char *encoding )
{
    // qDebug("looking for encoding id for '%s'", encoding );
    int len = strlen( encoding );
    if ( len < 4 )
	return -1;
    unsigned int hash1 = make_tag( encoding[0], encoding[1], encoding[2], encoding[3] );
    const char *ch = encoding + len - 4;
    unsigned int hash2 = make_tag( ch[0], ch[1], ch[2], ch[3] );

    const XlfdEncoding *enc = xlfd_encoding;
    for ( ; enc->name; ++enc ) {
	if ( (enc->hash1 && enc->hash1 != hash1) ||
	     (enc->hash2 && enc->hash2 != hash2) )
	    continue;
	// hashes match, do a compare if strings match
	// the enc->name can contain '*'s we have to interpret correctly
	const char *n = enc->name;
	const char *e = encoding;
	while ( 1 ) {
 	    // qDebug("bol: *e='%c', *n='%c'", *e,  *n );
	    if ( *e == '\0' ) {
		if ( *n )
		    break;
		// qDebug( "found encoding id %d", enc->id );
		return enc->id;
	    }
	    if ( *e == *n ) {
		++e;
		++n;
		continue;
	    }
	    if ( *n != '*' )
		break;
	    ++n;
 	    // qDebug("skip: *e='%c', *n='%c'", *e,  *n );
	    while ( *e && *e != *n )
		++e;
	}
    }
    // qDebug( "couldn't find encoding %s", encoding );
    return -1;
}

int qt_mib_for_xlfd_encoding( const char *encoding )
{
    int id = qt_xlfd_encoding_id( encoding );
    if ( id != -1 ) return xlfd_encoding[id].mib;
    return 0;
};

int qt_encoding_id_for_mib( int mib )
{
    const XlfdEncoding *enc = xlfd_encoding;
    for ( ; enc->name; ++enc ) {
	if ( enc->mib == mib )
	    return enc->id;
    }
    return -1;
}

static const char * xlfd_for_id( int id )
{
    // special case: -1 returns the "*-*" encoding, allowing us to do full
    // database population in a single X server round trip.
    if ( id < 0 || id > numEncodings )
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
static bool parseXFontName( char *fontName, char **tokens )
{
    if ( ! fontName || fontName[0] == '0' || fontName[0] != '-' ) {
	tokens[0] = 0;
	return FALSE;
    }

    int	  i;
    ++fontName;
    for ( i = 0; i < NFontFields && fontName && fontName[0]; ++i ) {
	tokens[i] = fontName;
	for ( ;; ++fontName ) {
	    if ( *fontName == '-' )
		break;
	    if ( ! *fontName ) {
		fontName = 0;
		break;
	    }
	}

	if ( fontName ) *fontName++ = '\0';
    }

    if ( i < NFontFields ) {
	for ( int j = i ; j < NFontFields; ++j )
	    tokens[j] = 0;
	return FALSE;
    }

    return TRUE;
}

static inline bool isZero(char *x)
{
    return (x[0] == '0' && x[1] == 0);
}

static inline bool isScalable( char **tokens )
{
    return (isZero(tokens[PixelSize]) &&
	    isZero(tokens[PointSize]) &&
	    isZero(tokens[AverageWidth]));
}

static inline bool isSmoothlyScalable( char **tokens )
{
    return (isZero(tokens[ResolutionX]) &&
	    isZero(tokens[ResolutionY]));
}

static inline bool isFixedPitch( char **tokens )
{
    return (tokens[Spacing][0] == 'm' ||
	    tokens[Spacing][0] == 'c' ||
	    tokens[Spacing][0] == 'M' ||
	    tokens[Spacing][0] == 'C');
}

/*
  Fills in a font definition (QFontDef) from an XLFD (X Logical Font
  Description).

  Returns TRUE if the the given xlfd is valid.  The fields lbearing
  and rbearing are not given any values.
*/
bool qt_fillFontDef( const QByteArray &xlfd, QFontDef *fd, int screen )
{
    char *tokens[NFontFields];
    QByteArray buffer = xlfd;
    if ( ! parseXFontName(buffer.data(), tokens) )
	return FALSE;

    fd->family = QString::fromLatin1(tokens[Family]);
    QString foundry = QString::fromLatin1(tokens[Foundry]);
    if ( ! foundry.isEmpty() && foundry != QString::fromLatin1("*") )
	fd->family +=
	    QString::fromLatin1(" [") + foundry + QString::fromLatin1("]");

    if ( qstrlen( tokens[AddStyle] ) > 0 )
	fd->addStyle = QString::fromLatin1(tokens[AddStyle]);
    else
	fd->addStyle = QString::null;

    fd->pointSize = atoi(tokens[PointSize]);
    fd->styleHint = QFont::AnyStyle;	// ### any until we match families

    char slant = tolower( (uchar) tokens[Slant][0] );
    fd->italic = ( slant == 'o' || slant == 'i' );
    char fixed = tolower( (uchar) tokens[Spacing][0] );
    fd->fixedPitch = ( fixed == 'm' || fixed == 'c' );
    fd->weight = getFontWeight( tokens[Weight] );

    int r = atoi(tokens[ResolutionY]);
    fd->pixelSize = atoi(tokens[PixelSize]);
    // not "0" or "*", or required DPI
    if ( r && fd->pixelSize && QPaintDevice::x11AppDpiY( screen ) &&
	 r != QPaintDevice::x11AppDpiY( screen ) ) {
	// calculate actual pointsize for display DPI
	fd->pointSize = (int) ((fd->pixelSize * 720.) /
			       QPaintDevice::x11AppDpiY( screen ) + 0.5);
    } else if ( fd->pixelSize == 0 && fd->pointSize ) {
	// calculate pixel size from pointsize/dpi
	fd->pixelSize = ( fd->pointSize *
				  QPaintDevice::x11AppDpiY( screen ) ) / 720;
    }

    return TRUE;
}

/*
  Fills in a font definition (QFontDef) from the font properties in an
  XFontStruct.

  Returns TRUE if the QFontDef could be filled with properties from
  the XFontStruct.  The fields lbearing and rbearing are not given any
  values.
*/
static bool qt_fillFontDef( XFontStruct *fs, QFontDef *fd, int screen )
{
    unsigned long value;
    if ( fs && !XGetFontProperty( fs, XA_FONT, &value ) )
	return FALSE;

    char *n = XGetAtomName( QPaintDevice::x11AppDisplay(), value );
    QByteArray xlfd( n );
    if ( n )
	XFree( n );
    return qt_fillFontDef( xlfd.lower(), fd, screen );
}


static QtFontStyle::Key getStyle( char ** tokens )
{
    QtFontStyle::Key key;

    char slant0 = tolower( (uchar) tokens[Slant][0] );

    if ( slant0 == 'r' ) {
        if ( tokens[Slant][1]) {
            char slant1 = tolower( (uchar) tokens[Slant][1] );

            if ( slant1 == 'o' )
                key.oblique = TRUE;
            else if ( slant1 == 'i' )
		key.italic = TRUE;
        }
    } else if ( slant0 == 'o' )
	key.oblique = TRUE;
    else if ( slant0 == 'i' )
	key.italic = TRUE;

    key.weight = getFontWeight( tokens[Weight] );

    if ( qstrcmp( tokens[Width], "normal" ) == 0 ) {
	key.stretch = 100;
    } else if ( qstrcmp( tokens[Width], "semi condensed" ) == 0 ||
		qstrcmp( tokens[Width], "semicondensed" ) == 0 ) {
	key.stretch = 90;
    } else if ( qstrcmp( tokens[Width], "condensed" ) == 0 ) {
	key.stretch = 80;
    } else if ( qstrcmp( tokens[Width], "narrow" ) == 0 ) {
	key.stretch = 60;
    }

    return key;
}


static inline void capitalize ( char *s )
{
    bool space = TRUE;
    while( *s ) {
	if ( space )
	    *s = toupper( *s );
	space = ( *s == ' ' );
	++s;
    }
}


extern bool qt_has_xft; // defined in qfont_x11.cpp

static bool xlfdsFullyLoaded = FALSE;
static unsigned char encodingLoaded[numEncodings];

static void loadXlfds( const char *reqFamily, int encoding_id )
{
    QtFontFamily *fontFamily = reqFamily ? db->family( reqFamily ) : 0;

    // make sure we don't load twice
    if ( (encoding_id == -1 && xlfdsFullyLoaded) || (encoding_id != -1 && encodingLoaded[encoding_id]) )
	return;
    if ( fontFamily && fontFamily->xlfdLoaded )
	return;

    int fontCount;
    // force the X server to give us XLFDs
    QByteArray xlfd_pattern("-*-");
    xlfd_pattern += reqFamily ? reqFamily : "*";
    xlfd_pattern += "-*-*-*-*-*-*-*-*-*-*-";
    xlfd_pattern += xlfd_for_id( encoding_id );

    char **fontList = XListFonts( QPaintDevice::x11AppDisplay(),
				  xlfd_pattern,
				  0xffff, &fontCount );
    // qDebug("requesting xlfd='%s', got %d fonts", xlfd_pattern.data(), fontCount );


    char *tokens[NFontFields];

    for( int i = 0 ; i < fontCount ; i++ ) {
	if ( ! parseXFontName( fontList[i], tokens ) ) continue;

	// get the encoding_id for this xlfd.  we need to do this
	// here, since we can pass -1 to this function to do full
	// database population
	*(tokens[CharsetEncoding]-1) = '-';
	int encoding_id = qt_xlfd_encoding_id( tokens[CharsetRegistry] );
	if ( encoding_id == -1 )
	    continue;

	char *familyName = tokens[Family];
	capitalize( familyName );
	char *foundryName = tokens[Foundry];
	capitalize( foundryName );
	QtFontStyle::Key styleKey = getStyle( tokens );

	bool smooth_scalable = FALSE;
	bool bitmap_scalable = FALSE;
	if ( isScalable(tokens) ) {
	    if ( isSmoothlyScalable( tokens ) )
		smooth_scalable = TRUE;
	    else
		bitmap_scalable = TRUE;
	}
	uint pixelSize = atoi( tokens[PixelSize] );
	uint xpointSize = atoi( tokens[PointSize] );
	uint xres = atoi( tokens[ResolutionX] );
	uint yres = atoi( tokens[ResolutionY] );
	uint avgwidth = atoi( tokens[AverageWidth] );
	bool fixedPitch = isFixedPitch( tokens );

	QtFontFamily *family = fontFamily ? fontFamily : db->family( familyName, TRUE );
	family->fontFileIndex = -1;
	QtFontFoundry *foundry = family->foundry( foundryName, TRUE );
	QtFontStyle *style = foundry->style( styleKey, TRUE );

	delete [] style->weightName;
	style->weightName = qstrdup( tokens[Weight] );
	delete [] style->setwidthName;
	style->setwidthName = qstrdup( tokens[Width] );

	if ( smooth_scalable ) {
	    style->smoothScalable = TRUE;
	    style->bitmapScalable = FALSE;
	    pixelSize = SMOOTH_SCALABLE;
	}
	if ( !style->smoothScalable && bitmap_scalable )
	    style->bitmapScalable = TRUE;
	if ( !fixedPitch )
	    family->fixedPitch = FALSE;

	QtFontSize *size = style->pixelSize( pixelSize, TRUE );
	QtFontEncoding *enc =
	    size->encodingID( encoding_id, xpointSize, xres, yres, avgwidth, TRUE );
	enc->pitch = *tokens[Spacing];
	if ( !enc->pitch ) enc->pitch = '*';

	for ( int script = 0; script < QFont::LastPrivateScript; ++script ) {
	    if ( scripts_for_xlfd_encoding[encoding_id][script] )
		family->scripts[script] = QtFontFamily::Supported;
	    else
		family->scripts[script] |= QtFontFamily::UnSupported_Xlfd;
	}
	if ( encoding_id == -1 )
	    family->xlfdLoaded = TRUE;
    }
    if ( !reqFamily ) {
	// mark encoding as loaded
	if ( encoding_id == -1 )
	    xlfdsFullyLoaded = TRUE;
	else
	    encodingLoaded[encoding_id] = TRUE;
    }

    XFreeFontNames( fontList );
}


#ifndef QT_NO_XFTFREETYPE
static int getXftWeight(int xftweight)
{
    int qtweight = QFont::Black;
    if (xftweight <= (XFT_WEIGHT_LIGHT + XFT_WEIGHT_MEDIUM) / 2)
	qtweight = QFont::Light;
    else if (xftweight <= (XFT_WEIGHT_MEDIUM + XFT_WEIGHT_DEMIBOLD) / 2)
	qtweight = QFont::Normal;
    else if (xftweight <= (XFT_WEIGHT_DEMIBOLD + XFT_WEIGHT_BOLD) / 2)
	qtweight = QFont::DemiBold;
    else if (xftweight <= (XFT_WEIGHT_BOLD + XFT_WEIGHT_BLACK) / 2)
	qtweight = QFont::Bold;

    return qtweight;
}

static void loadXft()
{
    if (!qt_has_xft)
	return;

    XftFontSet  *fonts;

    QString familyName;
    QString rawName;
    char *value;
    int weight_value;
    int slant_value;
    int spacing_value;
    char *file_value;
    int index_value;

    fonts =
	XftListFonts(QPaintDevice::x11AppDisplay(),
		     QPaintDevice::x11AppScreen(),
		     (const char *)0,
		     XFT_FAMILY, XFT_WEIGHT, XFT_SLANT,
		     XFT_SPACING, XFT_FILE, XFT_INDEX,
#ifdef QT_XFT2
		     FC_CHARSET,
#endif // QT_XFT2
		     (const char *)0);

    for (int i = 0; i < fonts->nfont; i++) {
	if (XftPatternGetString(fonts->fonts[i],
				XFT_FAMILY, 0, &value) != XftResultMatch )
	    continue;
	// 	capitalize( value );
	rawName = familyName = value;
	familyName.replace('-', ' ');

	slant_value = XFT_SLANT_ROMAN;
	weight_value = XFT_WEIGHT_MEDIUM;
	spacing_value = XFT_PROPORTIONAL;
	XftPatternGetInteger (fonts->fonts[i], XFT_SLANT, 0, &slant_value);
	XftPatternGetInteger (fonts->fonts[i], XFT_WEIGHT, 0, &weight_value);
	XftPatternGetInteger (fonts->fonts[i], XFT_SPACING, 0, &spacing_value);
	XftPatternGetString (fonts->fonts[i], XFT_FILE, 0, &file_value);
	XftPatternGetInteger (fonts->fonts[i], XFT_INDEX, 0, &index_value);

	QtFontFamily *family = db->family( familyName, TRUE );
	family->rawName = rawName;
	family->hasXft = TRUE;

#ifdef QT_XFT2
	if (! family->xftScriptCheck) {
	    FcCharSet *charset = 0;
	    FcResult res = FcPatternGetCharSet(fonts->fonts[i], FC_CHARSET, 0, &charset);
	    if (res == FcResultMatch) {
		for (int i = 0; i < QFont::LastPrivateScript; ++i) {
		    QChar ch = sampleCharacter((QFont::Script) i);

		    if (ch.unicode() != 0 &&
			FcCharSetHasChar(charset, ch.unicode())) {
			family->scripts[i] = QtFontFamily::Supported;
		    } else {
			family->scripts[i] |= QtFontFamily::UnSupported_Xft;
		    }
		}
		family->xftScriptCheck = TRUE;
	    }
	}
#endif // QT_XFT2

	QByteArray file(file_value);
	family->fontFilename = file;
	family->fontFileIndex = index_value;

	QtFontStyle::Key styleKey;
	styleKey.italic = (slant_value == XFT_SLANT_ITALIC);
	styleKey.oblique = (slant_value == XFT_SLANT_OBLIQUE);
	styleKey.weight = getXftWeight( weight_value );

	QtFontFoundry *foundry = family->foundry( QString::null,  TRUE );
	QtFontStyle *style = foundry->style( styleKey,  TRUE );

	style->smoothScalable = TRUE;
	family->fixedPitch = ( spacing_value >= XFT_MONO );

	QtFontSize *size = style->pixelSize( SMOOTH_SCALABLE, TRUE );
	QtFontEncoding *enc = size->encodingID( -1, 0, 0, 0, 0, TRUE );
	enc->pitch = ( spacing_value >= XFT_CHARCELL ? 'c' :
		       ( spacing_value >= XFT_MONO ? 'm' : 'p' ) );
    }

    XftFontSetDestroy (fonts);
}

#ifndef QT_XFT2

#define MAKE_TAG( _x1, _x2, _x3, _x4 ) \
          ( ( (Q_UINT32)_x1 << 24 ) |     \
            ( (Q_UINT32)_x2 << 16 ) |     \
            ( (Q_UINT32)_x3 <<  8 ) |     \
              (Q_UINT32)_x4         )

#ifdef _POSIX_MAPPED_FILES
static inline Q_UINT32 getUInt(unsigned char *p)
{
    Q_UINT32 val;
    val = *p++ << 24;
    val |= *p++ << 16;
    val |= *p++ << 8;
    val |= *p;

    return val;
}

static inline Q_UINT16 getUShort(unsigned char *p)
{
    Q_UINT16 val;
    val = *p++ << 8;
    val |= *p;

    return val;
}

static inline void tag_to_string( char *string, Q_UINT32 tag )
{
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
}

static Q_UINT16 getGlyphIndex( unsigned char *table, Q_UINT16 format, unsigned short unicode )
{
    if ( format == 0 ) {
	if ( unicode < 256 )
	    return (int) *(table+6+unicode);
    } else if ( format == 2 ) {
	qWarning("format 2 encoding table for Unicode, not implemented!");
    } else if ( format == 4 ) {
	Q_UINT16 segCountX2 = getUShort( table + 6 );
	unsigned char *ends = table + 14;
	Q_UINT16 endIndex = 0;
	int i = 0;
	for ( ; i < segCountX2/2 && (endIndex = getUShort( ends + 2*i )) < unicode; i++ );

	unsigned char *idx = ends + segCountX2 + 2 + 2*i;
	Q_UINT16 startIndex = getUShort( idx );

	if ( startIndex > unicode )
	    return 0;

	idx += segCountX2;
	Q_INT16 idDelta = (Q_INT16)getUShort( idx );
	idx += segCountX2;
	Q_UINT16 idRangeoffset_t = (Q_UINT16)getUShort( idx );

	Q_UINT16 glyphIndex;
	if ( idRangeoffset_t ) {
	    Q_UINT16 id = getUShort( idRangeoffset_t + 2*(unicode - startIndex) + idx);
	    if ( id )
		glyphIndex = ( idDelta + id ) % 0x10000;
	    else
		glyphIndex = 0;
	} else {
	    glyphIndex = (idDelta + unicode) % 0x10000;
	}
	return glyphIndex;
    }

    return 0;
}
#endif // _POSIX_MAPPED_FILES

static inline void checkXftCoverage( QtFontFamily *family )
{
#ifdef _POSIX_MAPPED_FILES
    QByteArray ext = family->fontFilename.mid( family->fontFilename.findRev( '.' ) ).lower();
    if ( family->fontFileIndex == 0 && ( ext == ".ttf" || ext == ".otf" ) ) {
	void *map;
	// qDebug("using own ttf code coverage checking of '%s'!", family->name.latin1() );
	int fd = open( family->fontFilename, O_RDONLY );
	size_t pagesize = getpagesize();
	off_t offset = 0;
	size_t length = (8192 / pagesize + 1) * pagesize;

	if ( fd == -1 )
	    goto xftCheck;
	{
	    if ( (map = mmap( 0, length, PROT_READ, MAP_SHARED, fd, offset ) ) == MAP_FAILED )
		goto error;

	    unsigned char *ttf = (unsigned char *)map;
	    Q_UINT32 version = getUInt( ttf );
	    if ( version != 0x00010000 ) {
		// qDebug("file has wrong version %x",  version );
		goto error1;
	    }
	    Q_UINT16 numTables =  getUShort( ttf+4 );

	    unsigned char *table_dir = ttf + 12;
	    Q_UINT32 cmap_offset = 0;
	    Q_UINT32 cmap_length = 0;
	    for ( int n = 0; n < numTables; n++ ) {
		Q_UINT32 tag = getUInt( table_dir + 16*n );
		if ( tag == MAKE_TAG( 'c', 'm', 'a', 'p' ) ) {
		    cmap_offset = getUInt( table_dir + 16*n + 8 );
		    cmap_length = getUInt( table_dir + 16*n + 12 );
		    break;
		}
	    }
	    if ( !cmap_offset ) {
		// qDebug("no cmap found" );
		goto error1;
	    }

	    if ( cmap_offset + cmap_length > length ) {
		munmap( map, length );
		offset = cmap_offset / pagesize * pagesize;
		cmap_offset -= offset;
		length = (cmap_offset + cmap_length);
		if ( (map = mmap( 0, length, PROT_READ, MAP_SHARED, fd, offset ) ) == MAP_FAILED )
		    goto error;
	    }

	    unsigned char *cmap = ((unsigned char *)map) + cmap_offset;

	    version = getUShort( cmap );
	    if ( version != 0 ) {
		// qDebug("wrong cmap version" );
		goto error1;
	    }
	    numTables = getUShort( cmap + 2 );
	    unsigned char *unicode_table = 0;
	    for ( int n = 0; n < numTables; n++ ) {
		Q_UINT32 version = getUInt( cmap + 4 + 8*n );
		// accept both symbol and Unicode encodings. prefer unicode.
		if ( version == 0x00030001 || version == 0x00030000 ) {
		    unicode_table = cmap + getUInt( cmap + 4 + 8*n + 4 );
		    if ( version == 0x00030001 )
			break;
		}
	    }

	    if ( !unicode_table ) {
		// qDebug("no unicode table found" );
		goto error1;
	    }

	    Q_UINT16 format = getUShort( unicode_table );
	    if ( format != 4 )
		goto error1;

	    for ( int i = 0; i < QFont::LastPrivateScript; ++i ) {
		QChar ch = sampleCharacter( (QFont::Script)i );
		if ( ch.unicode() != 0 &&
		     getGlyphIndex( unicode_table, format, ch.unicode() ) ) {
		    // qDebug("font can render script %d",  i );
		    family->scripts[i] = QtFontFamily::Supported;
		} else {
		    family->scripts[i] |= QtFontFamily::UnSupported_Xft;
		}
	    }
	    family->xftScriptCheck = TRUE;
	}
    error1:
	munmap( map, length );
    error:
	close( fd );
	if ( family->xftScriptCheck )
	    return;
    }
 xftCheck:
#endif // _POSIX_MAPPED_FILES

    FD_DEBUG("using Freetype for checking of '%s'", family->name.latin1() );

    FT_Library ft_lib;
    FT_Error error = FT_Init_FreeType( &ft_lib );
    if ( error ) return;
    FT_Face face;
    error = FT_New_Face( ft_lib, family->fontFilename, family->fontFileIndex, &face );
    if ( error ) return;

    for ( int i = 0; i < QFont::LastPrivateScript; ++i ) {
	QChar ch = sampleCharacter( (QFont::Script)i );
	if ( ch.unicode() != 0 && FT_Get_Char_Index ( face, ch.unicode() ) ) {
	    FD_DEBUG("font can render char %04x, %04x script %d '%s'",
		     ch.unicode(), FT_Get_Char_Index ( face, ch.unicode() ),
		     i, QFontDatabase::scriptName( (QFont::Script)i ).latin1() );

	    family->scripts[i] = QtFontFamily::Supported;
	} else {
	    family->scripts[i] |= QtFontFamily::UnSupported_Xft;
	}
    }
    FT_Done_Face( face );
    FT_Done_FreeType( ft_lib );
    family->xftScriptCheck = TRUE;
}
#endif // QT_XFT2
#endif // QT_NO_XFTFREETYPE

static void load( const QString &family = QString::null, int script = -1 )
{
    QTime t;
    t.start();

    if ( family.isNull() ) {
	if ( script == -1 )
	    loadXlfds( 0, -1 );
	else {
	    for ( int i = 0; i < numEncodings; i++ ) {
		if ( scripts_for_xlfd_encoding[i][script] )
		    loadXlfds( 0, i );
	    }
	}
    } else {
	QtFontFamily *f = db->family( family, TRUE );
	if ( !f->fullyLoaded ) {

#if !defined(QT_NO_XFTFREETYPE) && !defined(QT_XFT2)
	    // need to check Xft coverage
	    if ( f->hasXft && !f->xftScriptCheck ) {
		checkXftCoverage( f );
	    }
#endif
	    // could reduce this further with some more magic:
	    // would need to remember the encodings loaded for the family.
	    if ( ( script == -1 && !f->xlfdLoaded ) ||
		 ( !f->hasXft && !(f->scripts[script] & QtFontFamily::Supported) &&
		   !(f->scripts[script] & QtFontFamily::UnSupported_Xlfd) ) ) {
		loadXlfds( family, -1 );
		f->fullyLoaded = TRUE;
	    }
	}
    }

    FD_DEBUG("QFontDatabase: load( %s, %d) took %d ms", family.latin1(), script, t.elapsed() );
}


static void initializeDb()
{
    if ( db ) return;
    db = new QFontDatabasePrivate;
    qfontdatabase_cleanup.set(&db);

    memset( encodingLoaded, FALSE, sizeof( encodingLoaded ) );

    QTime t;
    t.start();

#ifndef QT_NO_XFTFREETYPE
    loadXft();
    FD_DEBUG("QFontDatabase: loaded Xft: %d ms",  t.elapsed() );
#endif

    t.start();

#ifndef QT_NO_XFTFREETYPE
    for ( int i = 0; i < db->count; i++ ) {
#ifndef QT_XFT2
	checkXftCoverage( db->families[i] );
	FD_DEBUG("QFontDatabase: xft coverage check: %d ms",  t.elapsed() );
#endif // QT_XFT2

#ifdef XFT_MATRIX
	for ( int j = 0; j < db->families[i]->count; ++j ) {	// each foundry
	    QtFontFoundry *foundry = db->families[i]->foundries[j];
	    for ( int k = 0; k < foundry->count; ++k ) {
		QtFontStyle *style = foundry->styles[k];
		if ( style->key.italic || style->key.oblique ) continue;

		QtFontSize *size = style->pixelSize( SMOOTH_SCALABLE );
		if ( ! size ) continue; // should not happen
		QtFontEncoding *enc = size->encodingID( -1, 0, 0, 0, 0, TRUE );
		if ( ! enc ) continue; // should not happen either

		QtFontStyle::Key key = style->key;

		// does this style have an italic equivalent?
		key.italic = TRUE;
		QtFontStyle *equiv = foundry->style( key );
		if ( equiv ) continue;

		// does this style have an oblique equivalent?
		key.italic = FALSE;
		key.oblique = TRUE;
		equiv = foundry->style( key );
		if ( equiv ) continue;

		// let's fake one...
		equiv = foundry->style( key, TRUE );
#ifndef QT_XFT2
		// Xft2 seems to do this automatically for us
		equiv->fakeOblique = TRUE;
#endif // !QT_XFT2
		equiv->smoothScalable = TRUE;

		QtFontSize *equiv_size = equiv->pixelSize( SMOOTH_SCALABLE, TRUE );
		QtFontEncoding *equiv_enc = equiv_size->encodingID( -1, 0, 0, 0, 0, TRUE );

		// keep the same pitch
		equiv_enc->pitch = enc->pitch;
	    }
	}
#endif // XFT_MATRIX
    }
#endif


#ifdef QFONTDATABASE_DEBUG
    // load everything at startup in debug mode.
    loadXlfds( 0,  -1 );

    // print the database
    for ( int f = 0; f < db->count; f++ ) {
	QtFontFamily *family = db->families[f];
	FD_DEBUG("'%s' %s  hasXft=%s", family->name.latin1(), (family->fixedPitch ? "fixed" : ""),
		 (family->hasXft ? "yes" : "no") );
	for ( int i = 0; i < QFont::LastPrivateScript; ++i ) {
	    FD_DEBUG("\t%s: %s", QFontDatabase::scriptName((QFont::Script) i).latin1(),
		     ((family->scripts[i] & QtFontFamily::Supported) ? "Supported" :
		      (family->scripts[i] & QtFontFamily::UnSupported) == QtFontFamily::UnSupported ?
		      "UnSupported" : "Unknown"));
	}

	for ( int fd = 0; fd < family->count; fd++ ) {
	    QtFontFoundry *foundry = family->foundries[fd];
	    FD_DEBUG("\t\t'%s'", foundry->name.latin1() );
	    for ( int s = 0; s < foundry->count; s++ ) {
		QtFontStyle *style = foundry->styles[s];
		FD_DEBUG("\t\t\tstyle: italic=%d oblique=%d weight=%d (%s)\n"
			 "\t\t\tstretch=%d (%s)",
			 style->key.italic, style->key.oblique, style->key.weight,
			 style->weightName, style->key.stretch,
			 style->setwidthName ? style->setwidthName : "nil" );
		if ( style->smoothScalable )
		    FD_DEBUG("\t\t\t\tsmooth scalable" );
		else if ( style->bitmapScalable )
		    FD_DEBUG("\t\t\t\tbitmap scalable" );
		if ( style->pixelSizes ) {
		    qDebug("\t\t\t\t%d pixel sizes",  style->count );
		    for ( int z = 0; z < style->count; ++z ) {
			QtFontSize *size = style->pixelSizes + z;
			for ( int e = 0; e < size->count; ++e ) {
			    FD_DEBUG( "\t\t\t\t  size %5d pitch %c encoding %s",
				      size->pixelSize,
				      size->encodings[e].pitch,
				      xlfd_for_id( size->encodings[e].encoding ) );
			}
		    }
		}
	    }
	}
    }
#endif // QFONTDATABASE_DEBUG
}

void QFontDatabase::createDatabase()
{
    initializeDb();
}


// --------------------------------------------------------------------------------------
// font loader
// --------------------------------------------------------------------------------------
#define MAXFONTSIZE_XFT 256
#define MAXFONTSIZE_XLFD 128

static
QFontEngine *loadEngine( QFont::Script script,
			 const QFontPrivate *fp, const QFontDef &request,
			 QtFontFamily *family, QtFontFoundry *foundry,
			 QtFontStyle *style, QtFontSize *size,
			 QtFontEncoding *encoding, bool forced_encoding )
{
    if ( fp && fp->rawMode ) {
	QByteArray xlfd = request.family.toLatin1();
	FM_DEBUG( "Loading XLFD (rawmode) '%s'", xlfd.constData() );

	XFontStruct *xfs;
	if (! (xfs = XLoadQueryFont(QPaintDevice::x11AppDisplay(), xlfd ) ) )
	    return 0;

	QFontEngine *fe = new QFontEngineXLFD( xfs, xlfd, 0 );
	if ( ! qt_fillFontDef( xfs, &fe->fontDef, QPaintDevice::x11AppScreen() ) &&
	     ! qt_fillFontDef( xlfd, &fe->fontDef, QPaintDevice::x11AppScreen() ) )
	    fe->fontDef = QFontDef();

	return fe;
    }

#ifndef QT_NO_XFTFREETYPE
    if ( encoding->encoding == -1 ) {

	FM_DEBUG( "    using Xft" );

	XftPattern *pattern = XftPatternCreate();
	if ( !pattern ) return 0;

#  ifndef QT_XFT2
	XftPatternAddString (pattern, XFT_ENCODING, "iso10646-1");
#  endif // QT_XFT2

	if ( !foundry->name.isEmpty() )
	    XftPatternAddString( pattern, XFT_FOUNDRY,
				 foundry->name.local8Bit() );

	if ( !family->rawName.isEmpty() )
	    XftPatternAddString( pattern, XFT_FAMILY,
				 family->rawName.local8Bit() );

	const char *stylehint_value = 0;
	switch ( request.styleHint ) {
	case QFont::SansSerif:
	    stylehint_value = "sans";
	    break;
	case QFont::Serif:
	    stylehint_value = "serif";
	    break;
	case QFont::TypeWriter:
	    stylehint_value = "mono";
	    break;
	default:
	    break;
	}
	if (stylehint_value)
	    XftPatternAddString( pattern, XFT_FAMILY, stylehint_value );

	char pitch_value = ( encoding->pitch == 'c' ? XFT_CHARCELL :
			     ( encoding->pitch == 'm' ? XFT_MONO : XFT_PROPORTIONAL ) );
	XftPatternAddInteger( pattern, XFT_SPACING, pitch_value );

	int weight_value = XFT_WEIGHT_BLACK;
	if ( style->key.weight == 0 )
	    weight_value = XFT_WEIGHT_MEDIUM;
	else if ( style->key.weight < (QFont::Light + QFont::Normal) / 2 )
	    weight_value = XFT_WEIGHT_LIGHT;
	else if ( style->key.weight < (QFont::Normal + QFont::DemiBold) / 2 )
	    weight_value = XFT_WEIGHT_MEDIUM;
	else if ( style->key.weight < (QFont::DemiBold + QFont::Bold) / 2 )
	    weight_value = XFT_WEIGHT_DEMIBOLD;
	else if ( style->key.weight < (QFont::Bold + QFont::Black) / 2 )
	    weight_value = XFT_WEIGHT_BOLD;
	XftPatternAddInteger( pattern, XFT_WEIGHT, weight_value );

	int slant_value = XFT_SLANT_ROMAN;
	if ( style->key.italic )
	    slant_value = XFT_SLANT_ITALIC;
	else if ( style->key.oblique )
	    slant_value = XFT_SLANT_OBLIQUE;
	XftPatternAddInteger( pattern, XFT_SLANT, slant_value );

	/*
	  Xft1 doesn't obey user settings for turning off anti-aliasing using
	  the following:

	  match any size > 6 size < 12 edit antialias = false;

	  ... if we request pixel sizes.  so, work around this limitiation and
	  convert the pixel size to a point size and request that.
	*/
	double size_value = request.pixelSize;
	double scale = 1.;
	if ( size_value > MAXFONTSIZE_XFT ) {
	    scale = (double)size_value/(double)MAXFONTSIZE_XFT;
	    size_value = MAXFONTSIZE_XFT;
	}

	if ( fp->paintdevice )
	    size_value *= 72. / QPaintDeviceMetrics( fp->paintdevice ).logicalDpiY();
	else if (QPaintDevice::x11AppDpiY( fp->screen ) != 75)
	    size_value *= 72. / QPaintDevice::x11AppDpiY( fp->screen );

	XftPatternAddDouble( pattern, XFT_SIZE, size_value );

#  ifdef XFT_MATRIX
	if ( ( request.stretch > 0 && request.stretch != 100 ) ||
	     ( style->key.oblique && style->fakeOblique ) ) {
	    XftMatrix matrix;
	    XftMatrixInit( &matrix );

	    if ( request.stretch > 0 && request.stretch != 100 )
		XftMatrixScale( &matrix, double( request.stretch ) / 100.0, 1.0 );
	    if ( style->key.oblique && style->fakeOblique )
		XftMatrixShear( &matrix, 0.20, 0.0 );

	    XftPatternAddMatrix( pattern, XFT_MATRIX, &matrix );
	}
#  endif // XFT_MATRIX

	extern bool qt_use_antialiasing; // defined in qfont_x11.cpp
	if ( !qt_use_antialiasing || request.styleStrategy & ( QFont::PreferAntialias |
							       QFont::NoAntialias) ) {
	    Bool requestAA = ( qt_use_antialiasing &&
			       !( request.styleStrategy & QFont::NoAntialias ) );
	    XftPatternAddBool( pattern, XFT_ANTIALIAS, requestAA );
	}

	XftResult res;
	XftPattern *result =
	    XftFontMatch( QPaintDevice::x11AppDisplay(), fp->screen, pattern, &res );
	XftPatternDestroy(pattern);

	// We pass a duplicate to XftFontOpenPattern because either xft font
	// will own the pattern after the call or the pattern will be
	// destroyed.
	XftPattern *dup = XftPatternDuplicate( result );
	XftFont *xftfs = XftFontOpenPattern( QPaintDevice::x11AppDisplay(), dup );

	if ( ! xftfs ) // Xft couldn't find a font?
	    return 0;

	QFontEngine *fe = new QFontEngineXft( xftfs, result, 0 );
	fe->setScale( scale );
	return fe;
    }
#endif // QT_NO_XFTFREETYPE

    FM_DEBUG( "    using XLFD" );

    QByteArray xlfd("-");
    xlfd += foundry->name.isEmpty() ? "*" : foundry->name.latin1();
    xlfd += "-";
    xlfd += family->name.isEmpty() ? "*" : family->name.latin1();

    xlfd += "-";
    xlfd += style->weightName ? style->weightName : "*";
    xlfd += "-";
    xlfd += ( style->key.italic ? "i" : ( style->key.oblique ? "o" : "r" ) );

    xlfd += "-";
    xlfd += style->setwidthName ? style->setwidthName : "*";
    // ### handle add-style
    xlfd += "-*-";

    int px = size->pixelSize;
    if ( style->smoothScalable && px == SMOOTH_SCALABLE )
	px = request.pixelSize;
    else if ( style->bitmapScalable && px == 0 )
	px = request.pixelSize;
    double scale = 1.;
    if ( px > MAXFONTSIZE_XLFD ) {
	scale = (double)px/(double)MAXFONTSIZE_XLFD;
	px = MAXFONTSIZE_XLFD;
    }

    xlfd += QString::number( px ).latin1();
    xlfd += "-";
    xlfd += QString::number( encoding->xpoint );
    xlfd += "-";
    xlfd += QString::number( encoding->xres );
    xlfd += "-";
    xlfd += QString::number( encoding->yres );
    xlfd += "-";

    // ### handle cell spaced fonts
    xlfd += encoding->pitch;
    xlfd += "-";
    xlfd += QString::number( encoding->avgwidth );
    xlfd += "-";
    xlfd += xlfd_for_id( encoding->encoding );

    FM_DEBUG( "    xlfd: '%s'", xlfd.data() );

    XFontStruct *xfs;
    if (! (xfs = XLoadQueryFont(QPaintDevice::x11AppDisplay(), xlfd ) ) )
	return 0;

    QFontEngine *fe = 0;
    const int mib = xlfd_encoding[ encoding->encoding ].mib;
    switch ( script ) {
    case QFont::Latin:
	if ( ! forced_encoding ) {
	    fe = new QFontEngineLatinXLFD( xfs, xlfd, mib );
	    break;
	}

    default:
	fe = new QFontEngineXLFD( xfs, xlfd, mib );
	break;
    }

    fe->setScale( scale );

    return fe;
}
