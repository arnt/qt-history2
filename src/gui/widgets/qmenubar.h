/****************************************************************************
**
** Definition of QMenuBar class.
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

#ifndef QMENUBAR_H
#define QMENUBAR_H

#include "qmenu.h"

#ifdef QT_USE_NEW_MENU_SYSTEM
class Q_GUI_EXPORT QMenuBar : public Q4MenuBar
{
    Q_OBJECT
public:
    QMenuBar(QWidget* parent=0, const char* =0) : Q4MenuBar(parent) { }
};
typedef QAction QMenuItem;
#else
#include "q3menubar.h"
class Q_GUI_EXPORT QMenuBar : public Q3MenuBar
{
    Q_OBJECT
public:
    QMenuBar(QWidget* parent=0, const char* name=0) : Q3MenuBar(parent, name) { }
};
typedef Q3MenuItem QMenuItem;
#endif

#endif // QMENUBAR_H
