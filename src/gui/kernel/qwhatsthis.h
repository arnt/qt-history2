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

#ifndef QWHATSTHIS_H
#define QWHATSTHIS_H

#include "QtCore/qobject.h"
#include "QtGui/qcursor.h"
#include "QtGui/qaction.h"

#ifndef QT_NO_WHATSTHIS

#ifdef QT3_SUPPORT
class QToolButton;
#endif

class Q_GUI_EXPORT QWhatsThis
{
    QWhatsThis();
public:
    static void enterWhatsThisMode();
    static bool inWhatsThisMode();
    static void leaveWhatsThisMode();

    static void showText(const QPoint &pos, const QString& text, QWidget* w = 0);
    static void hideText();

#ifdef QT3_SUPPORT
    static QT3_SUPPORT void add(QWidget *w, const QString &s);
    static QT3_SUPPORT void remove(QWidget *);
    static QT3_SUPPORT QToolButton * whatsThisButton(QWidget * parent);
#endif
};

class QWhatsThisAction: public QAction
{
    Q_OBJECT

public:
    explicit QWhatsThisAction(QObject* parent = 0);

private slots:
    void actionTriggered();
};

#endif // QT_NO_WHATSTHIS

#endif // QWHATSTHIS_H
