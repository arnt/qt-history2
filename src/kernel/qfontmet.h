/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontmet.h#34 $
**
** Definition of QFontMetrics class
**
** Created : 940514
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
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
    int		maxLeftBearing() const;
    int		maxRightBearing() const;
    int		maxWidth()	const;

    bool	inFont(char)	const;

    int		leftBearing(char) const;
    int		rightBearing(char) const;
    int		width( const char *, int len = -1 ) const;
    int		width( char ) const;
    QRect	boundingRect( const char *, int len = -1 ) const;
    QRect	boundingRect( char ) const;
    QRect	boundingRect( int x, int y, int w, int h, int flags,
			      const char *str, int len=-1, int tabstops=0,
			      int *tabarray=0, char **intern=0 ) const;
    QSize	size( int flags,
		      const char *str, int len=-1, int tabstops=0,
		      int *tabarray=0, char **intern=0 ) const;

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
    HDC hdc() const;
#elif defined(_WS_X11_)
    void *fontStruct() const;
    int printerAdjusted(int) const;
#endif

    enum Type { FontInternal, Widget, Painter };
    union {
	int   flags;
	void *dummy;
    } t;
    union {
	QFontInternal *f;
	QWidget	      *w;
	QPainter      *p;
    } u;

    int	    type()	     const { return t.flags & 0xff; }
    bool    underlineFlag()  const { return (t.flags & 0x100) != 0; }
    bool    strikeOutFlag()  const { return (t.flags & 0x200) != 0; }
    void    setUnderlineFlag()	   { t.flags |= 0x100; }
    void    setStrikeOutFlag()	   { t.flags |= 0x200; }

    friend class QWidget;
    friend class QPainter;
};


#endif // QFONTMET_H
