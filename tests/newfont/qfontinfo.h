/****************************************************************************
**
** Definition of QFontInfo class.
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

#ifndef QFONTINFO_H
#define QFONTINFO_H

#ifndef QT_H
#include "qfont.h"
#endif // QT_H


class Q_EXPORT QFontInfo
{
public:
    QFontInfo( const QFont & );
    QFontInfo( const QFontInfo & );
    ~QFontInfo();

    QFontInfo	       &operator=( const QFontInfo & );

    QString   	        family()	const;
    int			pointSize()	const;
    bool		italic()	const;
    int			weight()	const;
    bool		bold()		const;
    bool		underline()	const;
    bool		strikeOut()	const;
    bool		fixedPitch()	const;
    QFont::StyleHint	styleHint()	const;
    bool		rawMode()	const;

    bool		exactMatch()	const;
    
#ifndef QT_NO_COMPAT
    QFont::CharSet charSet() const;
#endif // QT_NO_COMPAT

#if 1	/* OBSOLETE */
    const QFont &font() const;
#endif

private:
    QFontInfo( const QPainter * );
    static void reset( const QPainter * );

    QFontPrivate *d;
    QPainter *painter;
    int flags;

    bool    underlineFlag()  const { return (flags & 0x1) != 0; }
    bool    strikeOutFlag()  const { return (flags & 0x2) != 0; }
    bool    exactMatchFlag() const { return (flags & 0x4) != 0; }
    void    setUnderlineFlag()	   { flags |= 0x1; }
    void    setStrikeOutFlag()	   { flags |= 0x2; }
    void    setExactMatchFlag()	   { flags |= 0x4; }

    friend class QWidget;
    friend class QPainter;
};


inline bool QFontInfo::bold() const
{ return weight() > QFont::Normal; }


#endif // QFONTINFO_H
