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
        for(QList<QMenuAction*>::Iterator it = actionItems.begin(); it != actionItems.end(); ++it)
            delete (*it);
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
    QList<QMenuAction*> actionItems;
    QList<QMenuAction*> calcActionRects(int width, int start) const;
    QRect actionRect(QMenuAction *) const;
    void updateActions();

    //selection
    QMenuAction *currentAction;
    uint mouseDown : 1, closePopupMode : 1, defaultPopDown;
    QMenuAction *actionAt(QPoint p) const;
    void setCurrentAction(QMenuAction *, bool =false, bool =false);
    void popupAction(QMenuAction *, bool);

    //active popup state
    uint popupState : 1;
    QPointer<QMenu> activeMenu;

    //keyboard mode for keyboard navigation
    void setKeyboardMode(bool);
    uint keyboardState : 1, altPressed : 1;
    QPointer<QWidget> keyboardFocusWidget;

    //firing of events
    void activateAction(QAction *, QAction::ActionEvent);

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

#ifdef QT_COMPAT
#ifdef Q_WS_MAC
struct Q3MenuBarCallBacks {
    typedef bool (*ActivateCmd)(MenuRef, short, bool, bool);
    ActivateCmd activate;

    typedef bool (*UpdatePopupCmd)(MenuRef);
    UpdatePopupCmd updatePopup;

    typedef bool (*UpdatePopupVisibleCmd)(MenuRef, bool);
    UpdatePopupVisibleCmd updatePopupVisible;

    typedef bool (*UpdateMenuBarCmd)();
    UpdateMenuBarCmd updateMenuBar;
};
#endif
#endif

#endif // QMENUBAR_P_H
