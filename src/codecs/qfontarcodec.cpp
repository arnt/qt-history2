/****************************************************************************
** $Id$
**
** Arabic Font utilities for X11
**
** Created : 20010130
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#include "private/qfontcodecs_p.h"
#include "private/qcomplextext_p.h"

#ifndef QT_NO_CODECS
#ifndef QT_NO_BIG_CODECS

#if 0
// this table covers basic arabic letter, not the extensions used in various other languages.
static const uchar arabic68PresentationA[] = {
    0x00, // fb50
    0x00, // fb51
    0x00, // fb52
    0x00, // fb53
    0x00, // fb54
    0x00, // fb55
    0x00, // fb56
    0x00, // fb57
    0x00, // fb58
    0x00, // fb59
    0x00, // fb5a
    0x00, // fb5b
    0x00, // fb5c
    0x00, // fb5d
    0x00, // fb5e
    0x00, // fb5f

    0x00, // fb60
    0x00, // fb61
    0x00, // fb62
    0x00, // fb63
    0x00, // fb64
    0x00, // fb65
    0x00, // fb66
    0x00, // fb67
    0x00, // fb68
    0x00, // fb69
    0x00, // fb6a
    0x00, // fb6b
    0x00, // fb6c
    0x00, // fb6d
    0x00, // fb6e
    0x00, // fb6f

    0x00, // fb70
    0x00, // fb71
    0x00, // fb72
    0x00, // fb73
    0x00, // fb74
    0x00, // fb75
    0x00, // fb76
    0x00, // fb77
    0x00, // fb78
    0x00, // fb79
    0x00, // fb7a
    0x00, // fb7b
    0x00, // fb7c
    0x00, // fb7d
    0x00, // fb7e
    0x00, // fb7f

    0x00, // fb80
    0x00, // fb81
    0x00, // fb82
    0x00, // fb83
    0x00, // fb84
    0x00, // fb85
    0x00, // fb86
    0x00, // fb87
    0x00, // fb88
    0x00, // fb89
    0x00, // fb8a
    0x00, // fb8b
    0x00, // fb8c
    0x00, // fb8d
    0x00, // fb8e
    0x00, // fb8f

    0x00, // fb90
    0x00, // fb91
    0x00, // fb92
    0x00, // fb93
    0x00, // fb94
    0x00, // fb95
    0x00, // fb96
    0x00, // fb97
    0x00, // fb98
    0x00, // fb99
    0x00, // fb9a
    0x00, // fb9b
    0x00, // fb9c
    0x00, // fb9d
    0x00, // fb9e
    0x00, // fb9f

    0x00, // fba0
    0x00, // fba1
    0x00, // fba2
    0x00, // fba3
    0x00, // fba4
    0x00, // fba5
    0x00, // fba6
    0x00, // fba7
    0x00, // fba8
    0x00, // fba9
    0x00, // fbaa
    0x00, // fbab
    0x00, // fbac
    0x00, // fbad
    0x00, // fbae
    0x00, // fbaf

    0x00, // fbb0
    0x00, // fbb1
    0x00, // fbb2
    0x00, // fbb3
    0x00, // fbb4
    0x00, // fbb5
    0x00, // fbb6
    0x00, // fbb7
    0x00, // fbb8
    0x00, // fbb9
    0x00, // fbba
    0x00, // fbbb
    0x00, // fbbc
    0x00, // fbbd
    0x00, // fbbe
    0x00, // fbbf

    0x00, // fbc0
    0x00, // fbc1
    0x00, // fbc2
    0x00, // fbc3
    0x00, // fbc4
    0x00, // fbc5
    0x00, // fbc6
    0x00, // fbc7
    0x00, // fbc8
    0x00, // fbc9
    0x00, // fbca
    0x00, // fbcb
    0x00, // fbcc
    0x00, // fbcd
    0x00, // fbce
    0x00, // fbcf

    0x00, // fbd0
    0x00, // fbd1
    0x00, // fbd2
    0x00, // fbd3
    0x00, // fbd4
    0x00, // fbd5
    0x00, // fbd6
    0x00, // fbd7
    0x00, // fbd8
    0x00, // fbd9
    0x00, // fbda
    0x00, // fbdb
    0x00, // fbdc
    0x00, // fbdd
    0x00, // fbde
    0x00, // fbdf

    0x00, // fbe0
    0x00, // fbe1
    0x00, // fbe2
    0x00, // fbe3
    0x00, // fbe4
    0x00, // fbe5
    0x00, // fbe6
    0x00, // fbe7
    0x00, // fbe8
    0x00, // fbe9
    0x00, // fbea
    0x00, // fbeb
    0x00, // fbec
    0x00, // fbed
    0x00, // fbee
    0x00 // fbef
};
#endif

static const uchar arabic68PresentationB[] = {
    0xa8, // fe70 	I Fathatan
    0xbc, // fe71 	M Fathatan on Tatweel
    0xa9, // fe72  	I Dammatan
    0xff, // fe73  	reserved
    0xaa, // fe74 	I Kasratan
    0xff, // fe75	reserved
    0xab, // fe76	I Fatha
    0xdb, // fe77	M Fatha
    0xac, // fe78	I damma
    0xdc, // fe79	M damma
    0xad, // fe7a	I Kasra
    0xdd, // fe7b	M kasra
    0xae, // fe7c	I Shadda
    0xde, // fe7d	M shadda
    0xaf, // fe7e	I sukun
    0xdf, // fe7f	M sukun

    0xc1, // fe80    	I hamza
    0xc2, // fe81	I alef with madda
    0xc2, // fe82	f alef with madda
    0xc3, // fe83	I alef with hamza above
    0xc3, // fe84	f alef with hamza above
    0xc4, // fe85	i waw with hamza above
    0xc4, // fe86	f with with hamza above
    0xc5, // fe87	i alef with hamza below
    0xc5, // fe88	f alef with hamza below
    0xc6, // fe89	I yeh with hamza above
    0xc6, // fe8a	f yeh with hamza above
    0xc0, // fe8b	in yeh with hamza above
    0xc0, // fe8c	m yeh with hamza above
    0xc7, // fe8d	Is alef
    0xc7, // fe8e	f alef
    0xc8, // fe8f	is beh

    0xc8, // fe90    	f beh
    0xeb, // fe91	in beh
    0xeb, // fe92	m beh
    0xc9, // fe93	is teh marbuta
    0x8e, // fe94	f teh marbuta 
    0xca, // fe95	is teh
    0xca, // fe96	f
    0xec, // fe97	in 
    0xec, // fe98	m  
    0xcb, // fe99	is theh
    0xcb, // fe9a	f
    0xed, // fe9b	in
    0xed, // fe9c	m
    0xcc, // fe9d	is jeem
    0xcc, // fe9e	f 
    0xee, // fe9f	in

    0xee, // fea0    	m
    0xcd, // fea1	is hah
    0xcd, // fea2	f
    0xef, // fea3	in
    0xef, // fea4	m
    0xce, // fea5	is khah
    0xce, // fea6	f
    0xf0, // fea7	in
    0xf0, // fea8	m
    0xcf, // fea9	is dal
    0xcf, // feaa	f
    0xd0, // feab	is thal
    0xd0, // feac	f
    0xd1, // fead	is reh
    0xd1, // feae	f
    0xd2, // feaf	zain

    0xd2, // feb0
    0xd3, // feb1	seen
    0x8f, // feb2
    0xf1, // feb3
    0xf1, // feb4
    0xd4, // feb5	sheen
    0x90, // feb6
    0xf2, // feb7
    0xf2, // feb8
    0xd5, // feb9	sad
    0x91, // feba
    0xf3, // febb
    0xf3, // febc
    0xd6, // febd	dad
    0x92, // febe
    0xf4, // febf

    0xf4, // fec0
    0xd7, // fec1	tah
    0x93, // fec2
    0xd7, // fec3
    0xd7, // fec4
    0xd8, // fec5	zah
    0x94, // fec6
    0xd8, // fec7
    0xd8, // fec8
    0xd9, // fec9	ain
    0x96, // feca
    0xf5, // fecb
    0x95, // fecc
    0xda, // fecd	ghain
    0x98, // fece
    0xf6, // fecf

    0x97, // fed0
    0xe1, // fed1	feh
    0xe1, // fed2
    0xf7, // fed3
    0x99, // fed4
    0xe2, // fed5	qaf
    0xe2, // fed6
    0xf8, // fed7
    0x9a, // fed8
    0xe3, // fed9	kaf
    0xe3, // feda
    0xf9, // fedb
    0x9b, // fedc 
    0xe4, // fedd	lam
    0xe4, // fede
    0xfa, // fedf

    0xa6, // fee0 
    0xe5, // fee1	meem
    0xe5, // fee2
    0xfb, // fee3
    0xfb, // fee4
    0xe6, // fee5	noon
    0xe6, // fee6
    0xfc, // fee7
    0xfc, // fee8
    0xe7, // fee9	heh
    0x9d, // feea
    0xfd, // feeb
    0x9c, // feec
    0xe8, // feed	waw
    0xe8, // feee
    0xe9, // feef	alef maksura

    0xe9, // fef0
    0xea, // fef1	yeh
    0xea, // fef2
    0xfe, // fef3
    0xfe, // fef4
    0xff, // fef5	lam-alef with madda above
    0xff, // fef6
    0xff, // fef7	lam-alef with hamza above
    0xff, // fef8
    0xff, // fef9	lam-alef with hamza below
    0xff, // fefa
    0xff, // fefb	lam-alef
    0xff, // fefc
    0xff, // fefd	reserved
    0xff, // fefe	reserved
    0xff // feff 	reserved
};

static const uchar arabic68LamAlefMapping[8][2] = {
    // alef has to come first. We are in visual order already
    { 0xa2, 0xa5 },// fef5	lam-alef with madda above
    { 0xa2, 0xa6 }, // fef6
    { 0xa3, 0xa5 }, // fef7	lam-alef with hamza above
    { 0xa3, 0xa6 }, // fef8
    { 0xa4, 0xa5 }, // fef9	lam-alef with hamza below
    { 0xa4, 0xa6 }, // fefa
    { 0xa1, 0xa5 }, // fefb	lam-alef
    { 0xa1, 0xa6 } // fefc
};

int QFontArabic68Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}

QFontArabic68Codec::QFontArabic68Codec()
{
}

const char* QFontArabic68Codec::name() const
{
    return "iso8859-6.8x";
}

int QFontArabic68Codec::mibEnum() const
{
    return -1;
}

QString QFontArabic68Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString(); //###
}

QCString QFontArabic68Codec::fromUnicode(const QString& , int&  ) const
{
    return QCString();
}

QByteArray QFontArabic68Codec::fromUnicode(const QString& uc, int from, int len ) const
{
    if( len < 0 )
	len = uc.length() - from;
    if( len == 0 )
	return QByteArray();

    QByteArray result( len*2 ); // worst case
    uchar *data = (uchar *)result.data();
    const QChar *ch = uc.unicode() + from;
    int lenOut = len;
    for ( int i = 0; i < len; i++ ) {
	uchar r = ch->row();
	uchar c = ch->cell();
	if ( r == 0 && c < 0x80 ) {
	    *data = c;
	} else if ( r == 0xfe ) {
	    // presentation forms B
	    if ( c < 0x70 )
		*data = 0xff; // undefined char in iso8859-6.8x
	    else if ( c >= 0xf5 && c <= 0xfc) {
		// lam alef ligature
		*data = arabic68LamAlefMapping[c - 0xf5][0];
		data++;
		lenOut++;
		*data = arabic68LamAlefMapping[c - 0xf5][1];
	    } else
		*data = arabic68PresentationB[c - 0x70];
	} else if ( r == 0xfb ) {
	    // ### FIXME
	    // presentation forms A
	    *data = 0xff;
	} else if ( r == 0x06 ) {
	    // regular arabic
	    if ( c >= 0x60 && c <= 0x69 )
		// numbers
		*data = 0xb0 + c - 0x60;
	    else if ( c >= 0x1b && c <= 0x4a )
		// letters
		*data = c + 0xa0;
	    else if ( c == 0x0c )
		// arabic comma
		*data = 0xba;
	    else
		*data = 0xff;
	}
	ch++;
	data++;
    }
    result.resize( lenOut );
    return result;
}

ushort QFontArabic68Codec::characterFromUnicode( const QString &str, int pos ) const
{
    const QChar ch = QComplexText::shapedCharacter(str, pos);
    uchar r = ch.row();
    uchar c = ch.cell();
    ushort data = 0;
    if ( r == 0 && c < 0x80 ) {
	data = c;
    } else if ( r == 0xfe ) {
	// presentation forms B
	if ( c < 0x70 )
	    data = 0xff; // undefined char in iso8859-6.8x
	else if ( c >= 0xf5 ) {
	    // lam alef ligature
	    data = arabic68LamAlefMapping[c - 0xf5][0];
	    // ### FIXME
	    //data++;
	    //*data = arabic68LamAlefMapping[c - 0xf5][1];
	} else
	    data = arabic68PresentationB[c - 0x70];
    } else if ( r == 0xfb ) {
	// ### FIXME
	// presentation forms A
	data = 0xff;
    } else if ( r == 0x06 ) {
	// regular arabic
	if ( c >= 0x60 && c <= 0x69 )
	    // numbers
	    data = 0xb0 + c - 0x60;
	else if ( c == 0x40 )
	    // tatweel
	    data = 0xe0;
	else
	    data = 0xff;
    }
    return data;
}


#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS
