/****************************************************************************
**
** Definition of QWidget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt Compat Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef __Q3MAINWINDOW_P_H__
#define __Q3MAINWINDOW_P_H__

#include <private/qwidget_p.h>

class Q3MainWindowLayout;

class Q3MainWindowPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(Q3MainWindow)
public:
    Q3MainWindowPrivate()
        :  mb(0), sb(0), ttg(0), mc(0), tll(0), mwl(0), ubp(false), utl(false),
           justify(false), movable(true), opaque(false), dockMenu(true)
    {
        docks.insert(Qt::DockTop, true);
        docks.insert(Qt::DockBottom, true);
        docks.insert(Qt::DockLeft, true);
        docks.insert(Qt::DockRight, true);
        docks.insert(Qt::DockMinimized, false);
        docks.insert(Qt::DockTornOff, true);
    }

    ~Q3MainWindowPrivate()
    {
    }

#ifndef QT_NO_MENUBAR
    mutable QMenuBar * mb;
#else
    mutable QWidget * mb;
#endif
    QStatusBar * sb;
    QToolTipGroup * ttg;

    QWidget * mc;

    QBoxLayout * tll;
    Q3MainWindowLayout * mwl;

    uint ubp :1;
    uint utl :1;
    uint justify :1;
    uint movable :1;
    uint opaque :1;
    uint dockMenu :1;

    Q3DockArea *topDock, *bottomDock, *leftDock, *rightDock;

    QList<Q3DockWindow *> dockWindows;
    QMap<Qt::Dock, bool> docks;
    QStringList disabledDocks;
    QHideDock *hideDock;

    QPointer<QPopupMenu> rmbMenu, tbMenu, dwMenu;
    QMap<Q3DockWindow*, bool> appropriate;
};

#endif /* __Q3MAINWINDOW_P_H__ */
