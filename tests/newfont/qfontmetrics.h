/****************************************************************************
**
** Definition of QFontMetrics class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFONTMETRICS_H
#define QFONTMETRICS_H

#ifndef QT_H
#include "qfont.h"
#include "qrect.h"
#endif // QT_H


class QTextCodec;
class QTextParag;

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
    
#ifndef QT_NO_COMPAT
    int		width( QChar ) const;
    int		width( char c ) const { return width( (QChar) c ); }
#endif // QT_NO_COMPAT
    
    int 		charWidth( const QString &str, int pos ) const;
    QRect	boundingRect( const QString &, int len = -1 ) const;
    QRect	boundingRect( QChar ) const;
    QRect	boundingRect( int x, int y, int w, int h, int flags,
			      const QString& str, int len=-1, int tabstops=0,
			      int *tabarray=0, QTextParag **intern=0 ) const;
    QSize	size( int flags,
		      const QString& str, int len=-1, int tabstops=0,
		      int *tabarray=0, QTextParag **intern=0 ) const;

    int		underlinePos()	const;
    int		strikeOutPos()	const;
    int		lineWidth()	const;

private:
    QFontMetrics( const QPainter * );
    static void reset( const QPainter * );

#if defined(Q_WS_WIN)
    void   *textMetric() const;
    HDC	    hdc() const;
#elif defined(Q_WS_QWS)
    QFontInternal *internal();
#endif

    friend class QWidget;
    friend class QPainter;

    QFontPrivate  *d;
    QPainter      *painter;
    int		   flags;

    bool    underlineFlag()  const { return (flags & 0x1) != 0; }
    bool    strikeOutFlag()  const { return (flags & 0x2) != 0; }
    void    setUnderlineFlag()	   { flags |= 0x1; }
    void    setStrikeOutFlag()	   { flags |= 0x2; }
};


#endif // QFONTMETRICS_H
