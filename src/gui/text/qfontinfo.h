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


class Q_GUI_EXPORT QFontInfo
{
public:
    QFontInfo(const QFont &);
    QFontInfo(const QFont &, QFont::Script);
    QFontInfo(const QFontInfo &);
    ~QFontInfo();

    QFontInfo               &operator=(const QFontInfo &);

    QString                   family()        const;
    int                        pixelSize()        const;
    int                        pointSize()        const;
    bool                italic()        const;
    int                        weight()        const;
    bool                bold()                const;
    bool                underline()        const;
    bool                overline()      const;
    bool                strikeOut()        const;
    bool                fixedPitch()        const;
    QFont::StyleHint        styleHint()        const;
    bool                rawMode()        const;

    bool                exactMatch()        const;


private:
    QFontInfo(const QPainter *);

    QFontPrivate *d;
    QPainter *painter;
    int fscript;

    friend class QWidget;
    friend class QPainter;
};


inline bool QFontInfo::bold() const
{ return weight() > QFont::Normal; }


#endif // QFONTINFO_H
