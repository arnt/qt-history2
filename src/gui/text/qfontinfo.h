/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFONTINFO_H
#define QFONTINFO_H

#include <QtGui/qfont.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class Q_GUI_EXPORT QFontInfo
{
public:
    QFontInfo(const QFont &);
    QFontInfo(const QFontInfo &);
    ~QFontInfo();

    QFontInfo &operator=(const QFontInfo &);

    QString family() const;
    int pixelSize() const;
    int pointSize() const;
    qreal pointSizeF() const;
    bool italic() const;
    QFont::Style style() const;
    int weight() const;
    inline bool bold() const { return weight() > QFont::Normal; }
    bool underline() const;
    bool overline() const;
    bool strikeOut() const;
    bool fixedPitch() const;
    QFont::StyleHint styleHint() const;
    bool rawMode() const;

    bool exactMatch() const;

private:
    QFontPrivate *d;
};

QT_END_HEADER

#endif // QFONTINFO_H
