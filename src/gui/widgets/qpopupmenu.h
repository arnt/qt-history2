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

#ifdef QT_USE_NEW_MENU_SYSTEM
class Q_GUI_EXPORT QPopupMenu : public QMenu
{
    Q_OBJECT
public:
    QPopupMenu(QWidget *parent = 0, const char * =0) : QMenu(parent)  { }
};
typedef QAction QMenuItem;
#else
#include "q3popupmenu.h"
class Q_GUI_EXPORT QPopupMenu : public Q3PopupMenu
{
    Q_OBJECT
public:
    QPopupMenu(QWidget *parent = 0, const char *name=0) : Q3PopupMenu(parent, name)  { }
};
class Q_GUI_EXPORT QCustomMenuItem : public Q3CustomMenuItem
{
public:
    QCustomMenuItem() : Q3CustomMenuItem() { }
};
typedef Q3MenuItem QMenuItem;
#endif

#endif // QPOPUPMENU_H
