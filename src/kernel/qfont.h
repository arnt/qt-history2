/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfont.h#3 $
**
** Definition of QFont class
**
** Author  : Haavard Nord
** Created : 940514
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFONT_H
#define QFONT_H

#include "qwindefs.h"
#include "qstring.h"


enum FontType { TimesFont, CourierFont, SystemFont };	// NOT USED YET...


class QFont
{
friend class QFontMetrics;
friend class QPainter;
public:
    QFont();					// default font
    QFont( const char *name );
    QFont( const QFont & );
   ~QFont();
    QFont &operator=( const QFont & );

    char  *name() const;			// get font name

    bool   changeFont( const char *name );

#if defined(_WS_X11_)
    Font   fontId() const;
#endif

    static void initialize();			// initialize font system
    static void cleanup();			// cleanup font system

private:
    struct QFontData : QShared {		// font data
	QString  name;
#if defined(_WS_WIN_)
	HANDLE hfont;
#elif defined(_WS_PM_)
#elif defined(_WS_X11_)
	QXFontStruct *f;
#endif
    } *data;
};


#endif // QFONT_H
