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

#include "qwidget.h"

class Q_GUI_EXPORT QToolTip
{
    QToolTip();
public:
    static void showText(const QPoint &pos, const QString &text, QWidget *w = 0);

    static QPalette palette();
#ifdef QT_COMPAT
    static inline QT_COMPAT void add(QWidget *w, const QString &s) { w->setToolTip(s); }
    static inline QT_COMPAT void add(QWidget *w, const QRect &, const QString &s)
    { w->setToolTip(s); }
    static inline QT_COMPAT void remove(QWidget *w) { w->setToolTip(QString()); }
#endif
};

#endif // QTOOLTIP_H
