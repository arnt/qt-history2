/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdata.h#28 $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Definition of QFontData struct
**
** Created : 941229
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
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
    QFontData()
	: exactMatch(FALSE), fin(0)
	{}
    QFontData( const QFontData &d )
	: req(d.req), exactMatch(d.exactMatch), fin(d.fin)
	{}
   ~QFontData()
	{}
    QFontData &operator=( const QFontData &d )
	{
	    req = d.req;
	    exactMatch = d.exactMatch;
	    fin = d.fin;
	    return *this;
	}
    QFontDef	    req;			// requested font
    bool	    exactMatch;
    QFontInternal  *fin;
};


#endif // QFONTDTA_H
