/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontmetrics.h#51 $
**
** Definition of QFontMetrics class
**
** Created : 940514
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

#ifndef QFONTMETRICS_H
#define QFONTMETRICS_H

#ifndef QT_H
#include "qfont.h"
#include "qrect.h"
#endif // QT_H

class QTextCodec;


class Q_EXPORT QFontMetrics
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
    int		minLeftBearing() const;
    int		minRightBearing() const;
    int		maxWidth()	const;

    bool	inFont(QChar)	const;

    int		leftBearing(QChar) const;
    int		rightBearing(QChar) const;
    int		width( const QString &, int len = -1 ) const;
    int		width( QChar ) const;
    int		width( char c ) const { return width( (QChar) c ); }
    QRect	boundingRect( const QString &, int len = -1 ) const;
    QRect	boundingRect( QChar ) const;
    QRect	boundingRect( int x, int y, int w, int h, int flags,
			      const QString& str, int len=-1, int tabstops=0,
			      int *tabarray=0, char **intern=0 ) const;
    QSize	size( int flags,
		      const QString& str, int len=-1, int tabstops=0,
		      int *tabarray=0, char **intern=0 ) const;

    int		underlinePos()	const;
    int		strikeOutPos()	const;
    int		lineWidth()	const;

private:
    QFontMetrics( const QPainter * );
    static void reset( const QPainter * );
    const QFontDef *spec() const;
#if defined(_WS_WIN_)
    void   *textMetric() const;
    HDC	    hdc() const;
#elif defined(_WS_X11_)
    void   *fontStruct() const;
    void   *fontSet() const;
    const QTextCodec *mapper() const;
    int	    printerAdjusted(int) const;
#endif

    QFontInternal *fin;
    QPainter      *painter;
    int		   flags;

    bool    underlineFlag()  const { return (flags & 0x1) != 0; }
    bool    strikeOutFlag()  const { return (flags & 0x2) != 0; }
    void    setUnderlineFlag()	   { flags |= 0x1; }
    void    setStrikeOutFlag()	   { flags |= 0x2; }

    friend class QWidget;
    friend class QPainter;
};


#endif // QFONTMETRICS_H
