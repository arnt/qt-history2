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

#ifndef QMENUBAR_P_H
#define QMENUBAR_P_H

#ifndef QMAC_Q3MENUBAR_CPP_FILE
#include "qstyleoption.h"

class QMenuBarPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QMenuBar)
public:
    QMenuBarPrivate() : itemsDirty(0), itemsWidth(0), itemsStart(-1), currentAction(0), mouseDown(0),
                         closePopupMode(0), defaultPopDown(1), popupState(0), keyboardState(0), altPressed(0)
#ifdef Q_WS_MAC
                         , mac_menubar(0)
#endif
    { }
    ~QMenuBarPrivate()
        {
#ifdef Q_WS_MAC
            delete mac_menubar;
#endif
        }

    void init();
    QStyleOptionMenuItem getStyleOption(const QAction *action) const;

    //item calculations
    uint itemsDirty : 1;
    int itemsWidth, itemsStart;

    QVector<int> shortcutIndexMap;
    mutable QMap<QAction*, QRect> actionRects;
    mutable QList<QAction*> actionList;
    void calcActionRects(int width, int start) const;
    QRect actionRect(QAction *) const;
    void updateActions();

    //selection
    QPointer<QAction>currentAction;
    uint mouseDown : 1, closePopupMode : 1, defaultPopDown;
    QAction *actionAt(QPoint p) const;
    void setCurrentAction(QAction *, bool =false, bool =false);
    void popupAction(QAction *, bool);

    //active popup state
    uint popupState : 1;
    QPointer<QMenu> activeMenu;

    //keyboard mode for keyboard navigation
    void setKeyboardMode(bool);
    uint keyboardState : 1, altPressed : 1;
    QPointer<QWidget> keyboardFocusWidget;

    //firing of events
    void activateAction(QAction *, QAction::ActionEvent);

    void actionTriggered();
    void actionHovered();
    void internalShortcutActivated(int);
    void updateLayout();

    //extra widgets in the menubar
    QPointer<QWidget> leftWidget, rightWidget;

#ifdef Q_WS_MAC
    //mac menubar binding
    struct QMacMenuBarPrivate {
        static QPointer<QMenuBar> fallback;
        static QHash<QWidget *, QMenuBar *> menubars;
        QList<QMacMenuAction*> actionItems;
        MenuRef menu, apple_menu;
        QMacMenuBarPrivate();
        ~QMacMenuBarPrivate();

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
    } *mac_menubar;
    void macCreateMenuBar(QWidget *);
    void macDestroyMenuBar();
    MenuRef macMenu();
#endif
};
#endif

#endif // QMENUBAR_P_H
