/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontmetrics.h#7 $
**
** Definition of QFontMetrics class
**
** Author  : Eirik Eng
** Created : 940514
**
** Copyright (C) 1994, 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFONTMET_H
#define QFONTMET_H

#include "qfont.h"

class QFontMetrics
{
public:
    QFontMetrics( const QFont & );

    int			ascent()	const;
    int			descent()	const;
    int			height()	const; // ascent() + descent()
    int                 leading()       const;
    int                 lineSpacing()   const; // height() + leading()

    int			width( const char *, int len = -1 ) const;
    int			width( char ) const;
    int                 maxWidth() const;

    int                 underlinePos()  const;
    int			strikeOutPos()  const;
    int			lineWidth()     const;

    void		setFont( const QFont & );
    const QFont	       &font() const;
private:
    QFont f;
};


#endif // QFONTMET_H
