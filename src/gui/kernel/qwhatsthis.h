/****************************************************************************
**
** Definition of QWhatsThis class.
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

#ifndef QWHATSTHIS_H
#define QWHATSTHIS_H

#ifndef QT_H
#include "qobject.h"
#include "qcursor.h"
#endif // QT_H

#ifndef QT_NO_WHATSTHIS

#ifdef QT_COMPAT
class QToolButton;
#endif
class QAction;
class Q_GUI_EXPORT QWhatsThis: public Qt
{
    QWhatsThis();
public:
    static void enterWhatsThisMode();
    static bool inWhatsThisMode();
    static void leaveWhatsThisMode();

    static void showText(const QPoint &pos, const QString& text, QWidget* w = 0);
    static void hideText();

    static QAction *action();

    static void add(QWidget *w, const QString &s); // obsolete
    static void remove(QWidget *); // obsolete

#ifdef QT_COMPAT
    static QT_COMPAT QToolButton * whatsThisButton(QWidget * parent);
#endif
};

#endif // QT_NO_WHATSTHIS

#endif // QWHATSTHIS_H
