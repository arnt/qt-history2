/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdata.h#3 $
**
** Definition of QFontData struct
**
** Author  : Eirik Eng
** Created : 941229
**
** Copyright (C) 1994, 1995 by Troll Tech AS.	 All rights reserved.
**
** --------------------------------------------------------------------------
** Internal header file containing private data common to QFont,
** QFontInfo and QFontMetrics.
**
** Uses definitions found in qfont.h, must always be included after qfont.h.
*****************************************************************************/

#ifndef QFONTDTA_H
#define QFONTDTA_H


struct QFontDef  {
	QString	  family;
	short	  pointSize;
	uint	  styleHint	: 8;
	uint	  charSet	: 8;
	uint	  weight	: 8;
	uint	  italic	: 1;
	uint	  underline	: 1;
	uint	  strikeOut	: 1;
	uint	  fixedPitch	: 1;
	uint	  hintSetByUser : 1;
	uint	  rawMode	: 1;
	uint	  dirty		: 1;
	uint	  exactMatch	: 1;
};

struct QFontData : QShared {
    QFontDef        req;                // requested
    QFontDef        act;                // actual
    short           lineW;              // width of underline and strikeOut
    short           refCount;           // number of QFontInfo and QFontMetrics
                                        // referencing this font.
#if defined(_WS_WIN_)
	HANDLE	hfont;
#elif defined(_WS_PM_)
#elif defined(_WS_X11_)
	QXFontStruct *xfont;
	QString	  xFontName;
#endif
};


#endif // QFONTDTA_H
