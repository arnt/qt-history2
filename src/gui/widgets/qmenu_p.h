#ifndef __QMENU_P_H__
#define __QMENU_P_H__

#include <private/qwidget_p.h>
#include <qbasictimer.h>
#include <qdatetime.h>
#include <qmenubar.h>

class QTornOffMenu;

struct QMenuAction {
    QAction *action;
    QRect rect;
};
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
        for(QList<QMenuAction*>::Iterator it = actionItems.begin(); it != actionItems.end(); ++it)
            delete (*it);
        delete scroll;
#ifdef Q_WS_MAC
        delete mac_menu;
#endif
    }

    //item calculations
    mutable uint itemsDirty : 1, maxIconWidth : 8, tabWidth : 8;
    QRect actionRect(QMenuAction *) const;
    QList<QMenuAction*> actionItems;
    QList<QMenuAction*> calcActionRects() const;
    void updateActions();
    uint ncols : 4; //4 bits is probably plenty

    //selection
    uint mouseDown : 1;
    QMenuAction *currentAction;
    QMenuAction *actionAt(QPoint p) const;
    void setFirstActionActive();
    void setCurrentAction(QMenuAction *, int =-1, bool =false);
    void popupAction(QMenuAction *, int, bool);

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
    QMenuAction *sloppyAction;
    QRegion sloppyRegion;

    //default action
    QPointer<QAction> defaultAction;

    //firing of events
    void activateAction(QAction *, QAction::ActionEvent);

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
};

#endif /* __QMENU_P_H__ */
