/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontinfo.h#1 $
**
** Definition of QFontInfo class
**
** Author  : Eirik Eng
** Created : 940131
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFONTINF_H
#define QFONTINF_H

#include "qfont.h"


class QFontInfo
{
public:
    QFontInfo( const QFont & );

    const char	       *family()	const;
    int			pointSize()	const;
    bool		italic()	const;
    int			weight()	const;
    bool		underline()	const;
    bool		strikeOut()	const;
    bool		fixedPitch()	const;
    QFont::StyleHint	styleHint()	const;
    QFont::CharSet	charSet()	const;
    bool		exactMatch()	const;
    bool		rawMode()	const;

    void		setFont( const QFont & );
    const QFont	       &font() const;
private:
    void  updateData();
    QFont f;
};


#endif // QFONTINF_H
