/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontinf.h#10 $
**
** Definition of QFontInfo class
**
** Author  : Eirik Eng
** Created : 950131
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QFONTINF_H
#define QFONTINF_H

#include "qfont.h"


class QFontInfo
{
public:
   ~QFontInfo();

    const char	       *family()	const;
    int			pointSize()	const;
    bool		italic()	const;
    int			weight()	const;
    bool		underline()	const;
    bool		strikeOut()	const;
    bool		fixedPitch()	const;
    QFont::StyleHint	styleHint()	const;
    QFont::CharSet	charSet()	const;
    bool		rawMode()	const;

    bool		exactMatch()	const;

    const QFont	       &font()		const;

private:
    QFontInfo( const QWidget * );
    QFontInfo( const QPainter * );
    static void reset( const void * );
    struct {
	bool widget;
	union {
	    QWidget  *w;
	    QPainter *p;
	};
    }		data;
    friend class QWidget;
    friend class QPainter;
};


#endif // QFONTINF_H
