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

#ifndef QCOREEVENT_H
#define QCOREEVENT_H

#include <QtCore/qnamespace.h>

QT_MODULE(Core)

class QEventPrivate;
class Q_CORE_EXPORT QEvent           // event base class
{
    QDOC_PROPERTY(bool accepted READ isAccepted WRITE setAccepted)
public:
    enum Type {
        /*
          If you get a strange compiler error on the line with None,
          it's probably because you're also including X11 headers,
          which #define the symbol None. Put the X11 includes after
          the Qt includes to solve this problem.
        */
        None = 0,                               // invalid event
        Timer = 1,                              // timer event
        MouseButtonPress = 2,                   // mouse button pressed
        MouseButtonRelease = 3,                 // mouse button released
        MouseButtonDblClick = 4,                // mouse button double click
        MouseMove = 5,                          // mouse move
        KeyPress = 6,                           // key pressed
        KeyRelease = 7,                         // key released
        FocusIn = 8,                            // keyboard focus received
        FocusOut = 9,                           // keyboard focus lost
        Enter = 10,                             // mouse enters widget
        Leave = 11,                             // mouse leaves widget
        Paint = 12,                             // paint widget
        Move = 13,                              // move widget
        Resize = 14,                            // resize widget
        Create = 15,                            // after widget creation
        Destroy = 16,                           // during widget destruction
        Show = 17,                              // widget is shown
        Hide = 18,                              // widget is hidden
        Close = 19,                             // request to close widget
        Quit = 20,                              // request to quit application
        ParentChange = 21,                      // widget has been reparented
        ParentAboutToChange = 131,              // sent just before the parent change is done
#ifdef QT3_SUPPORT
        Reparent = ParentChange,
#endif
        ThreadChange = 22,                      // object has changed threads
        WindowActivate = 24,                    // window was activated
        WindowDeactivate = 25,                  // window was deactivated
        ShowToParent = 26,                      // widget is shown to parent
        HideToParent = 27,                      // widget is hidden to parent
        Wheel = 31,                             // wheel event
        WindowTitleChange = 33,                 // window title changed
        WindowIconChange = 34,                  // icon changed
        ApplicationWindowIconChange = 35,       // application icon changed
        ApplicationFontChange = 36,             // application font changed
        ApplicationLayoutDirectionChange = 37,  // application layout direction changed
        ApplicationPaletteChange = 38,          // application palette changed
        PaletteChange = 39,                     // widget palette changed
        Clipboard = 40,                         // internal clipboard event
        Speech = 42,                            // reserved for speech input
        MetaCall =  43,                         // meta call event
        SockAct = 50,                           // socket activation
        WinEventAct = 132,                      // win event activation
        DeferredDelete = 52,                    // deferred delete event
        DragEnter = 60,                         // drag moves into widget
        DragMove = 61,                          // drag moves in widget
        DragLeave = 62,                         // drag leaves or is cancelled
        Drop = 63,                              // actual drop
        DragResponse = 64,                      // drag accepted/rejected
        ChildAdded = 68,                        // new child widget
        ChildPolished = 69,                     // polished child widget
#ifdef QT3_SUPPORT
        ChildInserted = 70,                     // compatibility posted insert
        LayoutHint = 72,                        // compatibility relayout request
#endif
        ChildRemoved = 71,                      // deleted child widget
        ShowWindowRequest = 73,                 // widget's window should be mapped
        PolishRequest = 74,                     // widget should be polished
        Polish = 75,                            // widget is polished
        LayoutRequest = 76,                     // widget should be relayouted
        UpdateRequest = 77,                     // widget should be repainted
#ifdef Q_WS_QWS
        QWSUpdate = 78,
#endif
        EmbeddingControl = 79,                  // ActiveX embedding
        ActivateControl = 80,                   // ActiveX activation
        DeactivateControl = 81,                 // ActiveX deactivation
        ContextMenu = 82,                       // context popup menu
        InputMethod = 83,                       // input method
        AccessibilityPrepare = 86,              // accessibility information is requested
        TabletMove = 87,                        // Wacom tablet event
        LocaleChange = 88,                      // the system locale changed
        LanguageChange = 89,                    // the application language changed
        LayoutDirectionChange = 90,             // the layout direction changed
        Style = 91,                             // internal style event
        TabletPress = 92,                       // tablet press
        TabletRelease = 93,                     // tablet release
        OkRequest = 94,                         // CE (Ok) button pressed
        HelpRequest = 95,                       // CE (?)  button pressed

        IconDrag = 96,                          // proxy icon dragged

        FontChange = 97,                        // font has changed
        EnabledChange = 98,                     // enabled state has changed
        ActivationChange = 99,                  // window activation has changed
        StyleChange = 100,                      // style has changed
        IconTextChange = 101,                   // icon text has changed
        ModifiedChange = 102,                   // modified state has changed
        MouseTrackingChange = 109,              // mouse tracking state has changed

        WindowBlocked = 103,                    // window is about to be blocked modally
        WindowUnblocked = 104,                  // windows modal blocking has ended
        WindowStateChange = 105,

        ToolTip = 110,
        WhatsThis = 111,
        StatusTip = 112,

        ActionChanged = 113,
        ActionAdded = 114,
        ActionRemoved = 115,

        FileOpen = 116,                         // file open request

        Shortcut = 117,                         // shortcut triggered
        ShortcutOverride = 51,                  // shortcut override request

#ifdef QT3_SUPPORT
        Accel = 30,                             // accelerator event
        AccelAvailable = 32,                    // accelerator available event
        AccelOverride = ShortcutOverride,       // accelerator override event
#endif

        WhatsThisClicked = 118,

#ifdef QT3_SUPPORT
        CaptionChange = WindowTitleChange,
        IconChange = WindowIconChange,
#endif
        ToolBarChange = 120,                    // toolbar visibility toggled

        ApplicationActivated = 121,             // application has been changed to active
        ApplicationDeactivated = 122,           // application has been changed to inactive

        QueryWhatsThis = 123,                   // query what's this widget help
        EnterWhatsThisMode = 124,
        LeaveWhatsThisMode = 125,

        ZOrderChange = 126,                     // child widget has had its z-order changed

        HoverEnter = 127,                       // mouse cursor enters a hover widget
        HoverLeave = 128,                       // mouse cursor leaves a hover widget
        HoverMove = 129,                        // mouse cursor move inside a hover widget

        AccessibilityHelp = 119,                // accessibility help text request
        AccessibilityDescription = 130,         // accessibility description text request

        // last event id used = 132

#ifdef QT_KEYPAD_NAVIGATION
        EnterEditFocus = 150,                   // enter edit mode in keypad navigation
        LeaveEditFocus = 151,                   // enter edit mode in keypad navigation
#endif

        User = 1000,                            // first user event id
        MaxUser = 65535                         // last user event id
    };

    QEvent(Type type);
    virtual ~QEvent();
    inline Type type() const { return static_cast<Type>(t); }
    inline bool spontaneous() const { return spont; }

    inline void setAccepted(bool accepted) { m_accept = accepted; }
    inline bool isAccepted() const { return m_accept; }

    inline void accept() { m_accept = true; }
    inline void ignore() { m_accept = false; }

protected:
    QEventPrivate *d;
    ushort t;

private:
    ushort posted : 1;
    ushort spont : 1;
    ushort m_accept : 1;
    ushort reserved : 13;

    friend class QCoreApplication;
    friend class QCoreApplicationPrivate;
    friend class QThreadData;
    friend class QApplication;
    friend class QApplicationPrivate;
    friend class Q3AccelManager;
    friend class QShortcutMap;
    friend class QETWidget;
};

class Q_CORE_EXPORT QTimerEvent : public QEvent
{
public:
    QTimerEvent( int timerId );
    ~QTimerEvent();
    int timerId() const { return id; }
protected:
    int id;
};

class QObject;

class Q_CORE_EXPORT QChildEvent : public QEvent
{
public:
    QChildEvent( Type type, QObject *child );
    ~QChildEvent();
    QObject *child() const { return c; }
    bool added() const { return type() == ChildAdded; }
#ifdef QT3_SUPPORT
    QT3_SUPPORT bool inserted() const { return type() == ChildInserted; }
#endif
    bool polished() const { return type() == ChildPolished; }
    bool removed() const { return type() == ChildRemoved; }
protected:
    QObject *c;
};

#ifdef QT3_SUPPORT
class Q_CORE_EXPORT QCustomEvent : public QEvent
{
public:
    QT3_SUPPORT_CONSTRUCTOR QCustomEvent(int type, void *data = 0);
    ~QCustomEvent();
    QT3_SUPPORT void *data()  const { return d; }
    QT3_SUPPORT void setData(void* data) { d = reinterpret_cast<QEventPrivate *>(data); }
};
#endif

#endif // QCOREEVENT_H
