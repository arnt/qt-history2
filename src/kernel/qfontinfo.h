/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontinfo.h#22 $
**
** Definition of QFontInfo class
**
** Created : 950131
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
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
    bool    exactMatchFlag() const { return (t.flags & 0x400) != 0; }
    void    setUnderlineFlag()	   { t.flags |= 0x100; }
    void    setStrikeOutFlag()	   { t.flags |= 0x200; }
    void    setExactMatchFlag()	   { t.flags |= 0x400; }

    friend class QWidget;
    friend class QPainter;
};


inline bool QFontInfo::bold() const
{ return weight() > QFont::Normal; }


#endif // QFONTINF_H
