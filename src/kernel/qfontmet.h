/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontmet.h#3 $
**
** Definition of QFontMetrics class
**
** Author  : Haavard Nord
** Created : 940514
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFONTMET_H
#define QFONTMET_H

#include "qfont.h"

class QFontMetrics
{
public:
    QFontMetrics();
    QFontMetrics( const QFont & );

    int			ascent()	const;
    int			descent()	const;
    int			height()	const;

    int			width( const char *, int len = -1 ) const;
    int			width( char ) const;

    void		setFont( const QFont &);
    QFont	       *font();

    const char	       *family()	const;
    int			pointSize()	const;
    bool		italic()	const;
    int			weight()	const;
    bool		fixedPitch()	const;
    QFont::StyleHint	styleHint()	const;
    QFont::CharSet	charSet()	const;
    bool        	exactMatch()	const;
    bool		rawMode()	const;

private:
    void		updateData()	const;

    QFontData        *data;
    const QFont      *f;
};


#endif // QFONTMET_H
