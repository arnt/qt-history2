#ifndef __Q4MENU_P_H__
#define __Q4MENU_P_H__

#include <private/qwidget_p.h>
#include <qbasictimer.h>
#include <qdatetime.h>
#include <qaccel.h>

class Q4TornOffMenu;

struct Q4MenuAction {
    QAction *action;
    QRect rect;
};

class Q4MenuPrivate : public QWidgetPrivate 
{
    Q_DECLARE_PUBLIC(Q4Menu);
public:
    Q4MenuPrivate() : itemsDirty(0), maxIconWidth(0), tabWidth(0), ncols(0), mouseDown(0), 
		      currentAction(0), scroll(0), sync(0), tearoff(0), tornoff(0), tearoffHighlighted(0),
		      checkable(0), sloppyAction(0)  { }  
    ~Q4MenuPrivate() 
    { 
	for(QList<Q4MenuAction*>::Iterator it = actionItems.begin(); it != actionItems.end(); ++it) 
	    delete (*it);
	delete scroll;
    }

    //item calculations
    mutable uint itemsDirty : 1, maxIconWidth : 8, tabWidth : 8;
    QRect actionRect(Q4MenuAction *);
    QList<Q4MenuAction*> actionItems;
    QList<Q4MenuAction*> calcActionRects() const;
    void updateActions();
    uint ncols : 4; //4 bits is probably plenty

    //selection 
    uint mouseDown : 1;
    Q4MenuAction *currentAction;
    Q4MenuAction *actionAt(QPoint p);
    void setFirstActionActive();
    void setCurrentAction(Q4MenuAction *, int =-1, bool =false);
    void popupAction(Q4MenuAction *, int, bool);

    //scrolling support
    struct Q4MenuScroller {
	enum ScrollDirections { ScrollNone=0, ScrollUp=0x01, ScrollDown=0x02 };
	uint scrollFlags : 2, scrollDirection : 2;
	int scrollOffset;
	QBasicTimer *scrollTimer;

	Q4MenuScroller() : scrollFlags(ScrollNone), scrollDirection(ScrollNone), scrollOffset(0), scrollTimer(0) { }
	~Q4MenuScroller() { delete scrollTimer; }
    } *scroll;
    void scrollMenu(uint);

    //syncronous operation (ie exec())
    uint sync : 1;
    QPointer<QAction> syncAction;

    //passing of mouse events up the parent heirarchy
    QPointer<Q4Menu> activeMenu;
    bool mouseEventTaken(QMouseEvent *);

    //used to walk up the popup list
    QPointer<QWidget> causedPopup;
    void hideUpToMenuBar();

    //index mappings
    inline QAction *actionAt(int i) { return q_func()->actions().at(i); }
    inline int indexOf(QAction *act) { return q_func()->actions().indexOf(act); }

    //tear off support
    uint tearoff : 1, tornoff : 1, tearoffHighlighted : 1;
    QPointer<Q4TornOffMenu> tornPopup;

    //checkable
    uint checkable : 1;

    //sloppy selection
    Q4MenuAction *sloppyAction;
    QRegion sloppyRegion;
};

class Q4MenuBarPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(Q4MenuBar);
public:
    Q4MenuBarPrivate() : itemsDirty(0), itemsWidth(0), shortcuts(0), currentAction(0), mouseDown(0), 
			 closePopupMode(0), popupState(0), keyboardState(0), altPressed(0)  { }
    ~Q4MenuBarPrivate() 
    { 
	for(QList<Q4MenuAction*>::Iterator it = actionItems.begin(); it != actionItems.end(); ++it) 
	    delete (*it);
    }

    //item calculations
    uint itemsDirty : 1;
    int itemsWidth;
#ifndef QT_NO_ACCEL
    QAccel *shortcuts;
#endif
    QList<Q4MenuAction*> actionItems;
    QList<Q4MenuAction*> calcActionRects(int width) const;
    QRect actionRect(Q4MenuAction *);
    void updateActions();

    //selection 
    Q4MenuAction *currentAction;
    uint mouseDown : 1, closePopupMode : 1;
    Q4MenuAction *actionAt(QPoint p);
    void setCurrentAction(Q4MenuAction *, bool =false, bool =false);
    void popupAction(Q4MenuAction *, bool);

    //active popup state
    uint popupState : 1;
    QPointer<Q4Menu> activeMenu;

    //keyboard mode for keyboard navigation
    void setKeyboardMode(bool);
    uint keyboardState : 1, altPressed : 1;
    QPointer<QWidget> keyboardFocusWidget;
};



#endif /* __Q4MENU_P_H__ */
