/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontmetrics.h#17 $
**
** Definition of QFontMetrics class
**
** Author  : Eirik Eng
** Created : 940514
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFONTMET_H
#define QFONTMET_H

#include "qfont.h"
#include "qrect.h"


class QFontMetrics
{
public:
    QFontMetrics( const QPaintDevice * );
   ~QFontMetrics();

    int		ascent()	const;
    int		descent()	const;
    int		height()	const;
    int		leading()	const;
    int		lineSpacing()	const;

    int		width( const char *, int len = -1 ) const;
    int		width( char )	const;
    QRect	boundingRect( const char *, int len = -1 ) const;
    QRect	boundingRect( char ) const;
    int		maxWidth()	const;

    int		underlinePos()	const;
    int		strikeOutPos()	const;
    int		lineWidth()	const;

    const QFont &font()		const	{ return f; }

private:
    QFontMetrics( const QPainter * );
    static void reset( const QPaintDevice * );
    QFont	f;
    QPaintDevice *pdev;
    friend class QPaintDevice;
};


#endif // QFONTMET_H
