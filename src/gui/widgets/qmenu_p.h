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

#ifndef QMENU_P_H
#define QMENU_P_H

#include <private/qwidget_p.h>
#include <qbasictimer.h>
#include <qdatetime.h>
#include <qmenubar.h>
#include <qstyleoption.h>
#include <qmap.h>

class QTornOffMenu;

#ifdef Q_WS_MAC
struct QMacMenuAction {
    uint command;
    uchar ignore_accel : 1;
    QPointer<QAction> action;
    MenuRef menu;
};
#endif

class QMenuPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QMenu)
public:
    QMenuPrivate() : itemsDirty(0), maxIconWidth(0), tabWidth(0), ncols(0), mouseDown(0),
                      currentAction(0), scroll(0), sync(0), tearoff(0), tornoff(0), tearoffHighlighted(0),
                      checkable(0), sloppyAction(0)
#ifdef Q_WS_MAC
                      ,mac_menu(0)
#endif
    { }
    ~QMenuPrivate()
    {
        delete scroll;
#ifdef Q_WS_MAC
        delete mac_menu;
#endif
    }

    //item calculations
    mutable uint itemsDirty : 1, maxIconWidth : 8, tabWidth : 8;
    QRect actionRect(QAction *) const;
    mutable QMap<QAction*, QRect> actionRects;
    mutable QList<QAction*> actionList;
    void calcActionRects(QMap<QAction*, QRect> &actionRects, QList<QAction*> &actionList) const;
    void updateActions();
    uint ncols : 4; //4 bits is probably plenty

    //selection
    uint mouseDown : 1;
    QAction *currentAction;
    QAction *actionAt(QPoint p) const;
    void setFirstActionActive();
    void setCurrentAction(QAction *, int =-1, bool =false);
    void popupAction(QAction *, int, bool);

    //scrolling support
    struct QMenuScroller {
        enum ScrollDirections { ScrollNone=0, ScrollUp=0x01, ScrollDown=0x02 };
        uint scrollFlags : 2, scrollDirection : 2;
        int scrollOffset;
        QBasicTimer *scrollTimer;

        QMenuScroller() : scrollFlags(ScrollNone), scrollDirection(ScrollNone), scrollOffset(0), scrollTimer(0) { }
        ~QMenuScroller() { delete scrollTimer; }
    } *scroll;
    void scrollMenu(uint);

    //syncronous operation (ie exec())
    uint sync : 1;
    QPointer<QAction> syncAction;
    QStyleOptionMenuItem getStyleOption(const QAction *action) const;

    //passing of mouse events up the parent heirarchy
    QPointer<QMenu> activeMenu;
    bool mouseEventTaken(QMouseEvent *);

    //used to walk up the popup list
    QPointer<QWidget> causedPopup;
    void hideUpToMenuBar();

    //index mappings
    inline QAction *actionAt(int i) const { return q_func()->actions().at(i); }
    inline int indexOf(QAction *act) const { return q_func()->actions().indexOf(act); }

    //tear off support
    uint tearoff : 1, tornoff : 1, tearoffHighlighted : 1;
    QPointer<QTornOffMenu> tornPopup;

    //checkable
    uint checkable : 1;

    //sloppy selection
    QAction *sloppyAction;
    QRegion sloppyRegion;

    //default action
    QPointer<QAction> defaultAction;

    QAction *menuAction;

    //firing of events
    void activateAction(QAction *, QAction::ActionEvent);

    void actionTriggered();
    void actionHovered();

#ifdef Q_WS_MAC
    //mac menu binding
    struct QMacMenuPrivate {
        QList<QMacMenuAction*> actionItems;
        MenuRef menu;
        QMacMenuPrivate();
        ~QMacMenuPrivate();

        void addAction(QAction *, QMacMenuAction* =0);
        void addAction(QMacMenuAction *, QMacMenuAction* =0);
        void syncAction(QMacMenuAction *);
        inline void syncAction(QAction *a) { syncAction(findAction(a)); }
        void removeAction(QMacMenuAction *);
        inline void removeAction(QAction *a) { removeAction(findAction(a)); }
        inline QMacMenuAction *findAction(QAction *a) {
            for(int i = 0; i < actionItems.size(); i++) {
                QMacMenuAction *act = actionItems[i];
                if(a == act->action)
                    return act;
            }
            return 0;
        }
    } *mac_menu;
    MenuRef macMenu(MenuRef merge);
#endif

    QPointer<QWidget> noReplayFor;
};

#endif // QMENU_P_H
