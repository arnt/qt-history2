/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontinf.h#19 $
**
** Definition of QFontInfo class
**
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
    QFontInfo( const QFont & );
    QFontInfo( const QFontInfo & );
   ~QFontInfo();

    QFontInfo	       &operator=( const QFontInfo & );

    const char	       *family()	const;
    int			pointSize()	const;
    bool		italic()	const;
    int			weight()	const;
    bool		bold()		const;
    bool		underline()	const;
    bool		strikeOut()	const;
    bool		fixedPitch()	const;
    QFont::StyleHint	styleHint()	const;
    QFont::CharSet	charSet()	const;
    bool		rawMode()	const;

    bool		exactMatch()	const;

#if 1	/* OBSOLETE */
    const QFont &font() const;
#endif

private:
    QFontInfo( const QWidget * );
    QFontInfo( const QPainter * );
    static void reset( const QWidget * );
    static void reset( const QPainter * );
    const QFontDef *spec() const;

    enum Type { FontInternal, FontInternalExactMatch, Widget, Painter };
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


inline bool QFontInfo::bold() const
{ return weight() > QFont::Normal; }


#endif // QFONTINF_H
