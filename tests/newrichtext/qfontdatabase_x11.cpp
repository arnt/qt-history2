/****************************************************************************
** $Id$
**
** Implementation of platform specific QFontDatabase
**
** Created : 970521
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qfontdata_p.h"

#include "qt_x11.h"

#include <ctype.h>
#include <stdlib.h>

// #define QFONTDATABASE_DEBUG
#ifdef QFONTDATABASE_DEBUG
#  include <qdatetime.h>
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
    { "iso8859-1", 0, 0, make_tag('i','s','o','8'), make_tag('5','9','-','1') },
    { "iso8859-2", 1, 5, make_tag('i','s','o','8'), make_tag('5','9','-','2') },
    { "iso8859-3", 2, 6, make_tag('i','s','o','8'), make_tag('5','9','-','3') },
    { "iso8859-4", 3, 7, make_tag('i','s','o','8'), make_tag('5','9','-','4') },
    { "iso8859-14", 4, 110, make_tag('i','s','o','8'), make_tag('9','-','1','4') },
    { "iso8859-15", 5, 111, make_tag('i','s','o','8'), make_tag('9','-','1','5') },
    { "iso8859-5", 6, 8, make_tag('i','s','o','8'), make_tag('5','9','-','5') },
    { "*-cp1251", 7, 2251, 0, make_tag('1','2','5','1') },
    { "koi8-ru", 8, 2084, make_tag('k','o','i','8'), make_tag('8','-','r','u') },
    { "koi8-u", 9, 2088, make_tag('k','o','i','8'), make_tag('i','8','-','u') },
    { "koi8-r", 10, 2084, make_tag('k','o','i','8'), make_tag('i','8','-','r') },
    { "iso8859-7", 11, 10, make_tag('i','s','o','8'), make_tag('5','9','-','7') },
    { "iso8859-6", 12, 82, make_tag('i','s','o','8'), make_tag('5','9','-','6') },
    { "iso8859-8", 13, 85, make_tag('i','s','o','8'), make_tag('5','9','-','8') },
    { "gb18030-0", 14, -114, make_tag('g','b','1','8'), make_tag('3','0','-','0') },
    { "gb18030.2000-0", 15, -113, make_tag('g','b','1','8'), make_tag('0','0','-','0') },
    { "gbk-0", 16, -113, make_tag('g','b','k','-'), make_tag('b','k','-','0') },
    { "gb2312.*-0", 17, 57, make_tag('g','b','2','3'), 0 },
    { "jisx0201*-0", 18, 15, make_tag('j','i','s','x'), 0 },
    { "jisx0208*-0", 19, 63, make_tag('j','i','s','x'), 0 },
    { "ksc5601.1987-0", 20, 36, make_tag('k','s','c','5'), make_tag('8','7','-','0') },
    { "big5hkscs-0", 21, -2101, make_tag('b','i','g','5'), make_tag('c','s','-','0') },
    { "hkscs-1", 22, -2101, make_tag('h','k','s','c'), make_tag('c','s','-','1') },
    { "big5*-*", 23, -2026, make_tag('b','i','g','5'), 0 },
    { "tscii-*", 24, 2028, make_tag('t','s','c','i'), 0 },
    { "tis620*-*", 25, 2259, make_tag('t','i','s','6'), 0 },
    { "iso8859-11", 26, 2259, make_tag('i','s','o','8'), make_tag('9','-','1','1') },
    { "mulelao-1", 27, -4242, make_tag('m','u','l','e'), make_tag('a','o','-','1') },
    { "ethiopic-unicode", 28, 0, make_tag('e','t','h','i'), make_tag('c','o','d','e') },
    { "iso10646-1", 29, 0, make_tag('i','s','o','1'), make_tag('4','6','-','1') },
    { "unicode-*", 30, 0, make_tag('u','n','i','c'), 0 },
    { "*-symbol", 31, 0, 0, make_tag('m','b','o','l') },
    { "*-fontspecific", 32, 0, 0, make_tag('i','f','i','c') },
    { 0, 0, 0, 0, 0 }
};

static const char scripts_for_xlfd_encoding[33][49] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1 }

};

// ----- end of generated code -----

int qt_xlfdEncoding_Id( const char *encoding )
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
    //    qDebug( "couldn't find encoding %s", encoding );
    return -1;
}

int qt_mibForXlfd( const char * encoding )
{
    int id = qt_xlfdEncoding_Id( encoding );
    if ( id != -1 )
	return xlfd_encoding[id].mib;
    return 0;
};

static const char * xlfd_for_id( int id )
{
    // special case: -1 returns the "*-*" encoding, allowing us to do full
    // database population in a single X server round trip.
    if ( id < 0 ||
	 id > (signed) ( sizeof( xlfd_encoding ) / sizeof( XlfdEncoding ) ) - 1 )
	return "*-*";
    return xlfd_encoding[id].name;
}

static QtFontStyle::Key getStyle( char ** tokens )
{
    QtFontStyle::Key key;

    char slant0 = tolower( (uchar) tokens[QFontPrivate::Slant][0] );

    if ( slant0 == 'r' ) {
        if ( tokens[QFontPrivate::Slant][1]) {
            char slant1 = tolower( (uchar) tokens[QFontPrivate::Slant][1] );

            if ( slant1 == 'o' )
                key.oblique = TRUE;
            else if ( slant1 == 'i' )
		key.italic = TRUE;
        }
    } else if ( slant0 == 'o' )
	key.oblique = TRUE;
    else if ( slant0 == 'i' )
	key.italic = TRUE;

    key.weight = QFontPrivate::getFontWeight( tokens[QFontPrivate::Weight] );

    return key;
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
#endif // QT_NO_XFTFREETYPE


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

static inline bool isZero(char *x)
{
    return (x[0] == '0' && x[1] == 0);
}

static inline bool isScalable( char **tokens )
{
    return (isZero(tokens[QFontPrivate::PixelSize]) &&
	    isZero(tokens[QFontPrivate::PointSize]) &&
	    isZero(tokens[QFontPrivate::AverageWidth]));
}

static inline bool isSmoothlyScalable( char **tokens )
{
    return (isZero(tokens[QFontPrivate::ResolutionX]) &&
	    isZero(tokens[QFontPrivate::ResolutionY]));
}

static inline bool isFixedPitch( char **tokens )
{
    return (tokens[QFontPrivate::Spacing][0] == 'm' ||
	    tokens[QFontPrivate::Spacing][0] == 'c' ||
	    tokens[QFontPrivate::Spacing][0] == 'M' ||
	    tokens[QFontPrivate::Spacing][0] == 'C');
}

extern bool qt_has_xft; // defined in qfont_x11.cpp


static void loadXlfdEncoding( int encoding_id )
{
    int fontCount;

    // force the X server to give us XLFDs
    QCString xlfd_pattern = "-*-*-*-*-*-*-*-*-*-*-*-*-";
    xlfd_pattern += xlfd_for_id( encoding_id );
    char **fontList = XListFonts( QPaintDevice::x11AppDisplay(),
				  xlfd_pattern.data(),
				  0xffff, &fontCount );

    char *tokens[QFontPrivate::NFontFields];


    for( int i = 0 ; i < fontCount ; i++ ) {
	if ( !QFontPrivate::parseXFontName( fontList[i], tokens ) ) continue;

	// get the encoding_id for this xlfd.  we need to do this
	// here, since we can pass -1 to this function to do full
	// database population
	*(tokens[QFontPrivate::CharsetEncoding]-1) = '-';
	int encoding_id = qt_xlfdEncoding_Id( tokens[QFontPrivate::CharsetRegistry] );
	if ( encoding_id == -1 )
	    continue;

	char *familyName = tokens[QFontPrivate::Family];
	capitalize( familyName );
	char *foundryName = tokens[QFontPrivate::Foundry];
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
	int pixelSize = atoi( tokens[QFontPrivate::PixelSize] );
	bool fixedPitch = isFixedPitch( tokens );

	for ( int script = 0; script < QFont::NScripts + 1; script++ ) {
	    if ( !scripts_for_xlfd_encoding[encoding_id][script] )
		continue;

	    QtFontFamily *family = db->scripts[script].family( familyName, TRUE );
	    QtFontFoundry *foundry = family->foundry( foundryName,  TRUE );
	    QtFontStyle *style = foundry->style( styleKey,  TRUE );

	    style->xlfd_uses_regular = ( qstrcmp( tokens[QFontPrivate::Weight],
						  "regular" ) == 0 );

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
	    QtFontEncoding *enc = size->encodingID( encoding_id, TRUE );
	    enc->pitch = *tokens[QFontPrivate::Spacing];
	    if ( !enc->pitch ) enc->pitch = '*';
	}
    }

    XFreeFontNames( fontList );
}


#ifndef QT_NO_XFTFREETYPE
static void loadXft()
{
    if (!qt_has_xft)
	return;

    XftFontSet  *fonts;

    QString familyName;
    char *value;
    int weight_value;
    int slant_value;
    int spacing_value;

    fonts =
	XftListFonts(QPaintDevice::x11AppDisplay(),
		     QPaintDevice::x11AppScreen(),
		     (const char *)0,
		     XFT_FAMILY, XFT_WEIGHT, XFT_SLANT, XFT_SPACING,
		     (const char *)0);

    for (int i = 0; i < fonts->nfont; i++) {
	if (XftPatternGetString(fonts->fonts[i],
				XFT_FAMILY, 0, &value) != XftResultMatch )
	    continue;
	// 	capitalize( value );
	familyName = value;

	slant_value = XFT_SLANT_ROMAN;
	weight_value = XFT_WEIGHT_MEDIUM;
	spacing_value = XFT_PROPORTIONAL;
	XftPatternGetInteger (fonts->fonts[i], XFT_SLANT, 0, &slant_value);
	XftPatternGetInteger (fonts->fonts[i], XFT_WEIGHT, 0, &weight_value);
	XftPatternGetInteger (fonts->fonts[i], XFT_SPACING, 0, &spacing_value);

	const int script = QFont::Unicode;

	QtFontStyle::Key styleKey;
	styleKey.italic = (slant_value == XFT_SLANT_ITALIC);
	styleKey.oblique = (slant_value == XFT_SLANT_OBLIQUE);
	styleKey.weight = getXftWeight( weight_value );

	QtFontFamily *family = db->scripts[script].family( familyName, TRUE );
	QtFontFoundry *foundry = family->foundry( QString::null,  TRUE );
	QtFontStyle *style = foundry->style( styleKey,  TRUE );

	style->smoothScalable = TRUE;
	family->fixedPitch = ( spacing_value >= XFT_MONO );

	QtFontSize *size = style->pixelSize( SMOOTH_SCALABLE, TRUE );
	QtFontEncoding *enc = size->encodingID( -1, TRUE );
	enc->pitch = ( spacing_value >= XFT_CHARCELL ? 'c' :
		       ( spacing_value >= XFT_MONO ? 'm' : 'p' ) );

    }

    XftFontSetDestroy (fonts);
}
#endif




void QFontDatabase::createDatabase()
{
    if ( db ) return;
    db = new QFontDatabasePrivate;

#ifdef QFONTDATABASE_DEBUG
    QTime t;
    t.start();
#endif // QFONTDATABASE_DEBUG
    /*
      when the time comes, we need to change the font database to do
      incremental loading, instead of fully creating the database all
      at once.
    */
    // loadXlfdEncoding( ... ); // load default encoding
    loadXlfdEncoding( -1 ); // full load

#ifdef QFONTDATABASE_DEBUG
    qDebug("QFontDatabase: loaded XLFD: %d ms",  t.elapsed() );
    t.start();
#endif // QFONTDATABASE_DEBUG

#ifndef QT_NO_XFTFREETYPE
    loadXft();
#endif

#ifdef QFONTDATABASE_DEBUG
    qDebug("QFontDatabase: loaded Xft: %d ms",  t.elapsed() );
#endif // QFONTDATABASE_DEBUG

#ifdef QFONTDATABASE_DEBUG
    // print the database
    for ( int i = 0; i < QFont::NScripts; i++ ) {
	QtFontScript &script = db->scripts[i];
	qDebug("Script %s", QFontDatabase::scriptName( (QFont::Script)i ).latin1() );
	if ( !script.count ) {
	    qDebug("\tNo fonts found!");
	    continue;
	}
	for ( int f = 0; f < script.count; f++ ) {
	    QtFontFamily *family = script.families[f];
	    qDebug("\t'%s' fixed pitch %d", family->name.latin1(),
		   family->fixedPitch );

	    for ( int fd = 0; fd < family->count; fd++ ) {
		QtFontFoundry *foundry = family->foundries[fd];
		qDebug("\t\t'%s'", foundry->name.latin1() );
		for ( int s = 0; s < foundry->count; s++ ) {
		    QtFontStyle *style = foundry->styles[s];
		    qDebug("\t\t\tstyle: italic=%d oblique=%d weight=%d",
			   style->key.italic, style->key.oblique, style->key.weight );
		    if ( style->smoothScalable )
			qDebug("\t\t\t\tsmooth scalable" );
		    else if ( style->bitmapScalable )
			qDebug("\t\t\t\tbitmap scalable" );
		    if ( style->pixelSizes ) {
			qDebug("\t\t\t\t%d pixel sizes",  style->count );
			for ( int z = 0; z < style->count; ++z ) {
			    QtFontSize *size = style->pixelSizes + z;
			    for ( int e = 0; e < size->count; ++e ) {
				qDebug( "\t\t\t\t  size %5d pitch %c encoding %s",
					size->pixelSize,
					size->encodings[e].pitch,
					xlfd_for_id( size->encodings[e].encoding ) );
			    }
			}
		    }
		}
	    }
	}
    }
#endif // QFONTDATABASE_DEBUG
}


bool QFontDatabase::findFont( int script, int styleStrategy,
			      QString &family, QString &foundry,
			      int &weight, bool &italic, bool &oblique, int &pixelSize,
			      char &pitch, bool &xlfd_uses_regular, QCString &encoding )
{
    QtFontScript &scr = db->scripts[script];
    QtFontFamily *fam = 0;
    if ( family.isEmpty() && scr.count )
	fam = scr.families[0];
    else
	fam = scr.family( family );
    if ( !fam ) return FALSE;

    QtFontFoundry *fnd = 0;
    if ( !foundry.isEmpty() ) {
	fnd = fam->foundry( foundry );
    }
    // ### iterate over all foundries if not found/specified to find
    // ### the best match in all foundries
    if ( !fnd ) {
	fnd = fam->foundries[0];
    }

    QtFontStyle::Key styleKey;
    styleKey.italic = italic;
    styleKey.weight = weight;
    QtFontStyle *sty = fnd->style( styleKey );
    if ( !sty && italic ) {
	styleKey.italic = FALSE;
	styleKey.oblique = TRUE;
	sty = fnd->style( styleKey );
    }
    if ( !sty ) {
	// ### do some matching here
	sty = fnd->styles[0];
    }


    int px = pixelSize;
    if ( sty->smoothScalable )
	px = SMOOTH_SCALABLE;
    else if ( sty->bitmapScalable && ( styleStrategy & QFont::PreferMatch ) )
	px = 0;

    QtFontSize *size = sty->pixelSize( px );
    if ( !size ) {
	// find closest size match
	unsigned int distance = ~0;
	for ( int x = 0; x < sty->count; ++x ) {
	    unsigned int d = QABS( sty->pixelSizes[x].pixelSize - pixelSize);
	    if ( d < distance ) {
		distance = d;
		size = sty->pixelSizes + x;
	    }
	}
    }

    // set all the out values
    family = fam->name;
    foundry = fnd->name;
    weight  = sty->key.weight;
    italic  = sty->key.italic;
    oblique = sty->key.oblique;
    if ( !sty->smoothScalable )
	pixelSize = size->pixelSize;
    xlfd_uses_regular = sty->xlfd_uses_regular;
    int encoding_id = size->encodings[0].encoding;
    pitch = size->encodings[0].pitch;
    for ( int i = 1; i < size->count; i++ ) {
	// Xft is preferred
	if ( size->encodings[i].encoding == -1 ) {
	    pitch = size->encodings[i].pitch;
	    encoding_id = -1;
	    break;
	}
    }
    encoding = xlfd_for_id( encoding_id );

    return TRUE;
}
