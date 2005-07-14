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

#ifndef Q3POPUPMENU_H
#define Q3POPUPMENU_H

#include "QtGui/qmenu.h"

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3PopupMenu : public QMenu
{
    Q_OBJECT
public:
    inline Q3PopupMenu(QWidget *parent = 0, const char * =0) : QMenu(parent)
    { }

    inline int exec() { return findIdForAction(QMenu::exec()); }
    inline int exec(const QPoint & pos, int indexAtPoint = 0) {
        return findIdForAction(QMenu::exec(pos, actions().value(indexAtPoint)));
    }
};

#endif // QPOPUPMENU_H
