/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdata.h#41 $
**
** Definition of internal QFontData struct
**
** Created : 941229
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFONTDATA_P_H
#define QFONTDATA_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qmenudata.cpp, qmenubar.cpp, qmenubar.cpp, qpopupmenu.cpp,
// qmotifstyle.cpp and qwindowssstyle.cpp.  This header file may change
// from version to version without notice, or even be removed.
//
//


struct QFontDef {				// font definition
    QString	family;
    short	pointSize;
    uint	styleHint	: 8;
    uint	styleStrategie	: 8;
    uint	charSet		: 8;
    uint	weight		: 8;
    uint	italic		: 1;
    uint	underline	: 1;
    uint	strikeOut	: 1;
    uint	fixedPitch	: 1;
    uint	hintSetByUser	: 1;
    uint	rawMode		: 1;
    uint	dirty		: 1;
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
	: QShared(d), req(d.req), exactMatch(d.exactMatch), fin(d.fin)
	// Copy the QShared count as well. The count may need to be
	// reset when using the QFontData class, see QFont::QFont(QFontData*)
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
    QFontDef	      req;			// requested font
    bool	      exactMatch;
    QFontInternal    *fin;
    const QTextCodec *mapper()  const;
    void	     *fontSet() const;
};


#endif // QFONTDATA_P_H
