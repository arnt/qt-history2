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

#ifndef QTEXTOPTION_H
#define QTEXTOPTION_H

#include <QtCore/qnamespace.h>

template <typename T> class QList;
struct QTextOptionPrivate;

class Q_GUI_EXPORT QTextOption
{
public:
    QTextOption();
    QTextOption(Qt::Alignment alignment);
    ~QTextOption();

    QTextOption(const QTextOption &o);
    QTextOption &operator=(const QTextOption &o);

    inline void setAlignment(Qt::Alignment alignment) { align = alignment; }
    inline Qt::Alignment alignment() const { return Qt::Alignment(align); }

    inline void setLayoutDirection(Qt::LayoutDirection direction) { this->direction = direction; }
    inline Qt::LayoutDirection layoutDirection() const { return (Qt::LayoutDirection)direction; }

    enum WrapMode {
        WordWrap,
        ManualWrap,
        WrapAnywhere
    };
    inline void setWrapMode(WrapMode wrap) { wordWrap = wrap; }
    inline WrapMode wrapMode() const { return static_cast<WrapMode>(wordWrap); }

    enum Flag {
        IncludeTrailingSpaces = 0x80000000
    };
    Q_DECLARE_FLAGS(Flags, Flag)
    inline void setFlags(Flags flags) { f = flags; }
    inline Flags flags() const { return Flags(f); }

    inline void setTabStop(qreal tabStop) { tab = tabStop; }
    inline qreal tabStop() const { return tab; }

    void useDesignMetrics(bool b) { design = b; }
    bool usesDesignMetrics() const { return design; }

    void setTabArray(QList<qreal> tabStops);
    QList<qreal> tabArray() const;
private:
    uint align : 8;
    uint wordWrap : 4;
    uint design : 1;
    uint direction : 1;
    uint unused : 19;
    uint f;
    qreal tab;
    QTextOptionPrivate *d;
};

#endif // QTEXTOPTION_H
