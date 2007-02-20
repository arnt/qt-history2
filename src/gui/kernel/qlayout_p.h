/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QLAYOUT_P_H
#define QLAYOUT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qlayout*.cpp, and qabstractlayout.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "private/qobject_p.h"
#include "qstyle.h"

class Q_GUI_EXPORT QLayoutPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QLayout)

public:
    QLayoutPrivate();

    void getMargin(int *result, int userMargin, QStyle::PixelMetric pm) const;
    void doResize(const QSize &);
    void reparentChildWidgets(QWidget *mw);

    int insideSpacing;
    int leftMargin;
    int topMargin;
    int rightMargin;
    int bottomMargin;
    uint topLevel : 1;
    uint enabled : 1;
    uint activated : 1;
    uint autoNewChild : 1;
    QLayout::SizeConstraint constraint;
    QLayout::ItemRectPolicy itemRectPolicy;
    QRect rect;
    QWidget *menubar;
};

#endif // QLAYOUT_P_H
