/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdata.h#24 $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
*** Definition of QFontData struct
**
** Created : 941229
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** Internal header file containing private data common to QFont,
** QFontInfo and QFontMetrics.
**
** Uses definitions found in qfont.h, must always be included after qfont.h.
*****************************************************************************/

#ifndef QFONTDTA_H
#define QFONTDTA_H


struct QFontDef {				// font definition
    QString	family;
    short	pointSize;
    uint	styleHint	: 8;
    uint	charSet		: 8;
    uint	weight		: 8;
    uint	italic		: 1;
    uint	underline	: 1;
    uint	strikeOut	: 1;
    uint	fixedPitch	: 1;
    uint	hintSetByUser	: 1;
    uint	rawMode		: 1;
    uint	dirty		: 1;
};

class QFontInternal;

struct QFontData : public QShared {
    QFontData();
   ~QFontData();
    QFontData( const QFontData & );
    QFontData  &operator=( const QFontData & );
    QFontDef	req;				// requested font
    QFontDef	act;				// actual font
    uint	exactMatch : 1;
    short	lineW;				// underline/strikeOut font
    QFontInternal *fin;
};


#endif // QFONTDTA_H
