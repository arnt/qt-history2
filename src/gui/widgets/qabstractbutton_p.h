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

#ifndef QABSTRACTBUTTON_P_H
#define QABSTRACTBUTTON_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qwidget_p.h>
#include "qbasictimer.h"

class QAbstractButtonPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QAbstractButton)
public:
    QAbstractButtonPrivate();

    QString text;
    QIcon icon;
    QKeySequence shortcut;
    int shortcutId;
    uint checkable :1;
    uint checked :1;
    uint autoRepeat :1;
    uint autoExclusive :1;
    uint down :1;
    uint mlbDown :1;
    uint blockRefresh :1;

    QButtonGroup* group;
    QBasicTimer repeatTimer;
    QBasicTimer animateTimer;

    void init();
    void click();
    void refresh();

    QList<QAbstractButton *>queryButtonList() const;
    QAbstractButton *queryCheckedButton() const;
    void notifyChecked();
    void moveFocus(int key);
    void fixFocusPolicy();
};

#endif
