/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontmetrics.h#26 $
**
** Definition of QFontMetrics class
**
** Created : 940514
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFONTMET_H
#define QFONTMET_H

#include "qfont.h"
#include "qrect.h"


class QFontMetrics
{
public:
    QFontMetrics( const QFont & );
    QFontMetrics( const QFontMetrics & );
   ~QFontMetrics();

    QFontMetrics &operator=( const QFontMetrics & );

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

#if 1	/* OBSOLETE */
    const QFont &font() const;
#endif

private:
    QFontMetrics( const QWidget * );
    QFontMetrics( const QPainter * );
    static void reset( const QWidget * );
    static void reset( const QPainter * );
    const QFontDef *spec() const;
#if defined(_WS_WIN_)
    void *textMetric() const;
#elif defined(_WS_X11_)
    void *fontStruct() const;
#endif

    enum Type { FontInternal, Widget, Painter };
    union {
	Type  t;
	void *dummy;
    } type;
    union {
	QFontInternal *f;
	QWidget	      *w;
	QPainter      *p;
    } u;

    friend class QWidget;
    friend class QPainter;
};


#endif // QFONTMET_H
