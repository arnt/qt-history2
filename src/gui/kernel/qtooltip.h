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

#ifndef QTOOLTIP_H
#define QTOOLTIP_H

#include "QtGui/qwidget.h"

#ifndef QT_NO_TOOLTIP

class Q_GUI_EXPORT QToolTip
{
    QToolTip();
public:
    static void showText(const QPoint &pos, const QString &text, QWidget *w = 0);

    static QPalette palette();
#ifdef QT3_SUPPORT
    static inline QT3_SUPPORT void add(QWidget *w, const QString &s) { w->setToolTip(s); }
    static inline QT3_SUPPORT void add(QWidget *w, const QRect &, const QString &s)
    { w->setToolTip(s); }
    static inline QT3_SUPPORT void remove(QWidget *w) { w->setToolTip(QString()); }
#endif
};

#endif // QT_NO_TOOLTIP
#endif // QTOOLTIP_H
