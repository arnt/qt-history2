/****************************************************************************
**
** Definition of QPopupMenu class.
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

#ifndef QPOPUPMENU_H
#define QPOPUPMENU_H

#include "qmenu.h"

class Q_GUI_EXPORT QPopupMenu : public QMenu
{
    Q_OBJECT
public:
    QPopupMenu(QWidget *parent = 0, const char * =0) : QMenu(parent)  { }
#ifdef QT_COMPAT
    inline QT_COMPAT int exec() { return findIdForAction(QMenu::exec()); }
    inline QT_COMPAT int exec(const QPoint & pos, int indexAtPoint = 0) {
        return findIdForAction(QMenu::exec(pos, actions().value(indexAtPoint)));
    }
#endif
};

#endif // QPOPUPMENU_H
