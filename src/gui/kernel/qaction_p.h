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

#ifndef QACTION_P_H
#define QACTION_P_H

#include "qaction.h"
#include <private/qobject_p.h>
#include "qmenu.h"

#ifdef QT_COMPAT
class QMenuItemEmitter;
#endif

class QActionPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAction)
public:
    QActionPrivate();
    ~QActionPrivate();

    QPointer<QActionGroup> group;
    QString text;
    QString menuText;
    QIconSet *icons;
    QString tooltip;
    QString statustip;
    QString whatsthis;
    QKeySequence shortcut;
    int shortcutId;
    QFont font;
    QPointer<QMenu> menu;
    uint enabled : 1, forceDisabled : 1;
    uint visible : 1, forceInvisible : 1;
    uint checkable : 1;
    uint checked : 1;
    uint separator : 1;

    QList<QWidget *> widgets;

    void redoGrab(QShortcutMap &map);

#ifdef QT_COMPAT //for menubar/menu compat
    QMenuItemEmitter *act_signal;
    int id, param;
#endif
    void sendDataChanged();
};

#endif // QACTION_P_H
