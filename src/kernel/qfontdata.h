/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdata.h#38 $
**
**		      ***   INTERNAL HEADER FILE   ***
**
**		This file is NOT a part of the Qt interface!
**
** Definition of QFontData struct
**
** Created : 941229
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

#ifndef QFONTDATA_H
#define QFONTDATA_H


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
    uint	rezAdj		: 1;
    short	lbearing;
    short	rbearing;
};


class QFontInternal;
class QTextCodec;

struct QFontData : public QShared {
    QFontData()
	: exactMatch(FALSE), fin(0)
	{}
    QFontData( const QFontData &d )
	: QShared( d ), req(d.req), exactMatch(d.exactMatch), fin(d.fin)
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
    const QTextCodec* mapper() const;
    void* fontSet() const;
};


#endif // QFONTDATA_H
