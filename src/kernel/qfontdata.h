/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdata.h#1 $
**
** Definition of QFontData struct
**
** Author  : Eirik Eng
** Created : 941229
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************
**
** Internal header file containing private data common to QFont
** and QFontMetrics.
**
** Uses definitions found in qfont.h, must always be included after qfont.h
**
*****************************************************************************/

#ifndef QFONTDTA_H
#define QFONTDTA_H

struct QFontData : QShared {	// font data, used by QFont and QFontMetrics
	QString   family;
        short     pointSize;
#if defined(_WS_WIN_)
	HANDLE	hfont;
#elif defined(_WS_PM_)
#elif defined(_WS_X11_)
	QXFontStruct *f;
        QString       xFontName;
#endif
        uint      styleHint     : 8;
        uint      charSet       : 8;
        uint      weight        : 8;
        uint      italic        : 1;
        uint      fixedPitch    : 1;
        uint      hintSetByUser : 1;
        uint      rawMode       : 1;
        uint      dirty         : 1;
        uint      exactMatch    : 1;
};

#endif
