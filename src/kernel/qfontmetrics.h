/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontmetrics.h#48 $
**
** Definition of QFontMetrics class
**
** Created : 940514
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
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

    friend class QWidget;
    friend class QPainter;
};


#endif // QFONTMETRICS_H
