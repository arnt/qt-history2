/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontinfo.h#30 $
**
** Definition of QFontInfo class
**
** Created : 950131
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

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
    QFont::CharSet	charSet()	const;
    bool		rawMode()	const;

    bool		exactMatch()	const;

#if 1	/* OBSOLETE */
    const QFont &font() const;
#endif

private:
    QFontInfo( const QPainter * );
    static void reset( const QPainter * );
    const QFontDef *spec() const;

    QFontInternal *fin;
    QPainter      *painter;
    int		   flags;

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
