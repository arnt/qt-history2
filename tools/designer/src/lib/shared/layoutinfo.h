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

#ifndef LAYOUTINFO_H
#define LAYOUTINFO_H

#include "shared_global.h"

#include <QList>

class QWidget;
class QLayout;
class AbstractFormEditor;

class QT_SHARED_EXPORT LayoutInfo
{
public:
    enum Type
    {
        HBox,
        VBox,
        Grid,
        Stacked,
        NoLayout
    };

    static void deleteLayout(AbstractFormEditor *core, QWidget *widget);
    static Type layoutType(AbstractFormEditor *core, QWidget *w, QLayout *&layout);
    static Type layoutType(AbstractFormEditor *core, QLayout *layout);
    static Type layoutType(AbstractFormEditor *core, QWidget *w);
    static QWidget *layoutParent(AbstractFormEditor *core, QLayout *layout);

    class Interval
    {
    public:
        int v1, v2;
        inline Interval(int _v1 = 0, int _v2 = 0)
            : v1(_v1), v2(_v2) {}
        bool operator < (const Interval &other) const
            { return v1 < other.v1; }
    };
    typedef QList<Interval> IntervalList;
    static void cells(QLayout *layout, IntervalList *rows, IntervalList *columns);
};

#endif // LAYOUTINFO_H
