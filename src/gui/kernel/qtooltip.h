/****************************************************************************
**
** Definition of Tool Tips (or Balloon Help) for any widget or rectangle.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTOOLTIP_H
#define QTOOLTIP_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class Q_GUI_EXPORT QToolTip: public Qt
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
