/****************************************************************************
**
** Definition of QWidget class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef __QMAINWINDOW_P_H__
#define __QMAINWINDOW_P_H__

#include <private/qwidget_p.h>

class QMainWindowLayout;

class QMainWindowPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QMainWindow);
public:
    QMainWindowPrivate()
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

    ~QMainWindowPrivate()
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
    QMainWindowLayout * mwl;

    uint ubp :1;
    uint utl :1;
    uint justify :1;
    uint movable :1;
    uint opaque :1;
    uint dockMenu :1;

    QDockArea *topDock, *bottomDock, *leftDock, *rightDock;

    QList<QDockWindow *> dockWindows;
    QMap<Qt::Dock, bool> docks;
    QStringList disabledDocks;
    QHideDock *hideDock;

    QPointer<QPopupMenu> rmbMenu, tbMenu, dwMenu;
    QMap<QDockWindow*, bool> appropriate;
};

/* QMainWindowLayout, respects widthForHeight layouts (like the left
  and right docks are)
*/

class QMainWindowLayout : public QLayout
{
    Q_OBJECT

public:
    QMainWindowLayout(QMainWindow *mw, QLayout* parent = 0);
    ~QMainWindowLayout() {}

    void addItem(QLayoutItem *);
    void setLeftDock(QDockArea *l);
    void setRightDock(QDockArea *r);
    void setCentralWidget(QWidget *w);
    bool hasHeightForWidth() const { return false; }
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutItem *itemAt(int) const { return 0; } //###
    QLayoutItem *takeAt(int) { return 0; } //###

    QSizePolicy::ExpandData expanding() const { return QSizePolicy::BothDirections; }

protected:
    void setGeometry(const QRect &r) {
        QLayout::setGeometry(r);
        layoutItems(r);
    }

private:
    int layoutItems(const QRect&, bool testonly = false);
    int extraPixels() const;

    QDockArea *left, *right;
    QWidget *central;
    QMainWindow *mainWindow;

};

#endif /* __QMAINWINDOW_P_H__ */
