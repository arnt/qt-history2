/****************************************************************************
**
** Definition of Q3MenuBar class.
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

#ifndef Q3MENUBAR_H
#define Q3MENUBAR_H

#ifndef QT_H
#include "q3popupmenu.h" // ### remove or keep for users' convenience?
#include "qframe.h"
#include "q3menudata.h"
#endif // QT_H

#ifndef QT_NO_MENUBAR

class Q3PopupMenu;

class Q_COMPAT_EXPORT Q3MenuBar : public Q3Frame, public Q3MenuData
{
    Q_OBJECT
    Q_ENUMS(Separator)
    Q_PROPERTY(Separator separator READ separator WRITE setSeparator DESIGNABLE false)
    Q_PROPERTY(bool defaultUp READ isDefaultUp WRITE setDefaultUp)

public:
    Q3MenuBar(QWidget* parent=0, const char* name=0);
    ~Q3MenuBar();

    void        updateItem(int id);

    void        show();                                // reimplemented show
    void        hide();                                // reimplemented hide

    bool        eventFilter(QObject *, QEvent *);

    int                heightForWidth(int) const;

    enum        Separator { Never=0, InWindowsStyle=1 };
    Separator         separator() const;
    virtual void        setSeparator(Separator when);

    void        setDefaultUp(bool);
    bool        isDefaultUp() const;

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize minimumSizeHint() const;

    void activateItemAt(int index);

signals:
    void        activated(int itemId);
    void        highlighted(int itemId);

protected:
    void        drawContents(QPainter *);
    void        mousePressEvent(QMouseEvent *);
    void        mouseReleaseEvent(QMouseEvent *);
    void        mouseMoveEvent(QMouseEvent *);
    void        keyPressEvent(QKeyEvent *);
    void        focusInEvent(QFocusEvent *);
    void        focusOutEvent(QFocusEvent *);
    void        resizeEvent(QResizeEvent *);
    void        leaveEvent(QEvent *);
    void        menuContentsChanged();
    void        menuStateChanged();
    void        changeEvent(QEvent *);
    int        itemAtPos(const QPoint &);
    void        hidePopups();
    QRect        itemRect(int item);

private slots:
    void        subActivated(int itemId);
    void        subHighlighted(int itemId);
#ifndef QT_NO_ACCEL
    void        accelActivated(int itemId);
    void        accelDestroyed();
#endif
    void        popupDestroyed(QObject*);
    void         performDelayedChanges();

private:
    void         performDelayedContentsChanged();
    void         performDelayedStateChanged();
    void        menuInsPopup(Q3PopupMenu *);
    void        menuDelPopup(Q3PopupMenu *);
    void        frameChanged();

    bool        tryMouseEvent(Q3PopupMenu *, QMouseEvent *);
    void        tryKeyEvent(Q3PopupMenu *, QKeyEvent *);
    void        goodbye(bool cancelled = false);
    void        openActPopup();

    void setActiveItem(int index, bool show = true, bool activate_first_item = true);
    void setAltMode(bool);

    int                calculateRects(int max_width = -1);

#ifndef QT_NO_ACCEL
    void        setupAccelerators();
    QAccel     *autoaccel;
#endif
    QRect      *irects;
    int                rightSide;

    uint        mseparator : 1;
    uint        waitforalt : 1;
    uint        popupvisible  : 1;
    uint        hasmouse : 1;
    uint         defaultup : 1;
    uint         toggleclose : 1;
    uint        pendingDelayedContentsChanges : 1;
    uint        pendingDelayedStateChanges : 1;

    friend class Q3PopupMenu;

#if defined(Q_WS_MAC) && !defined(QMAC_Q3MENUBAR_NO_NATIVE)
    friend class QWidget;
    friend class QApplication;
    friend class QMenuBar; //compat
    friend OSStatus qt_mac_menu_event(EventHandlerCallRef, EventRef event, void *); //compat

    void macCreateNativeMenubar();
    void macRemoveNativeMenubar();
    void macDirtyNativeMenubar();

#if !defined(QMAC_Q3MENUBAR_NO_EVENT)
    static void qt_mac_install_menubar_event(MenuRef);
    static OSStatus qt_mac_menubar_event(EventHandlerCallRef, EventRef, void *);
#endif
    virtual void macWidgetChangedWindow();
    bool syncPopups(MenuRef ret, Q3PopupMenu *d);
    MenuRef createMacPopup(Q3PopupMenu *d, int id, bool =false);
    bool updateMenuBar();
#if !defined(QMAC_Q3MENUBAR_NO_MERGE)
    uint isCommand(Q3MenuItem *, bool just_check=false);
#endif

    uint mac_eaten_menubar : 1;
    class MacPrivate;
    MacPrivate *mac_d;
    static bool activate(MenuRef, short, bool highlight=false, bool by_accel=false);
    static bool activateCommand(uint cmd);
    static bool macUpdateMenuBar();
    static bool macUpdatePopupVisible(MenuRef, bool);
    static bool macUpdatePopup(MenuRef);
#endif

private:        // Disabled copy constructor and operator=

#if defined(Q_DISABLE_COPY)
    Q3MenuBar(const Q3MenuBar &);
    Q3MenuBar &operator=(const Q3MenuBar &);
#endif
};


#endif // QT_NO_MENUBAR

#endif // Q3MENUBAR_H
