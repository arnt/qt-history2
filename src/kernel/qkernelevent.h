#ifndef __QKERNELEVENT_H__
#define __QKERNELEVENT_H__

#include <qnamespace.h>

class Q_KERNEL_EXPORT QEvent: public Qt		// event base class
{
public:
    enum Type {
	/*
	  If you get a strange compiler error on the line with None,
	  it's probably because you're also including X11 headers,
	  which #define the symbol None. Put the X11 includes after
	  the Qt includes to solve this problem.
	*/
	None = 0,				// invalid event
	Timer = 1,				// timer event
	MouseButtonPress = 2,			// mouse button pressed
	MouseButtonRelease = 3,			// mouse button released
	MouseButtonDblClick = 4,		// mouse button double click
	MouseMove = 5,				// mouse move
	KeyPress = 6,				// key pressed
	KeyRelease = 7,				// key released
	FocusIn = 8,				// keyboard focus received
	FocusOut = 9,				// keyboard focus lost
	Enter = 10,				// mouse enters widget
	Leave = 11,				// mouse leaves widget
	Paint = 12,				// paint widget
	Move = 13,				// move widget
	Resize = 14,				// resize widget
	Create = 15,				// after object creation
	Destroy = 16,				// during object destruction
	Show = 17,				// widget is shown
	Hide = 18,				// widget is hidden
	Close = 19,				// request to close widget
	Quit = 20,				// request to quit application
	Reparent = 21,				// widget has been reparented
	ShowMinimized = 22,		       	// widget is shown minimized
	ShowNormal = 23,	       		// widget is shown normal
	WindowActivate = 24,	       		// window was activated
	WindowDeactivate = 25,	       		// window was deactivated
	ShowToParent = 26,	       		// widget is shown to parent
	HideToParent = 27,	       		// widget is hidden to parent
	ShowMaximized = 28,		       	// widget is shown maximized
	ShowFullScreen = 29,			// widget is shown full-screen
	Accel = 30,				// accelerator event
	Wheel = 31,				// wheel event
	AccelAvailable = 32,			// accelerator available event
	CaptionChange = 33,			// caption changed
	IconChange = 34,			// icon changed
	ApplicationFontChange = 36,             // application font changed
	ApplicationPaletteChange = 38,          // application palette changed
	PaletteChange = 39,			// widget palette changed
	Clipboard = 40,				// internal clipboard event
	Speech = 42,				// reserved for speech input
	InvokeSlot =  43,			// invoke slot event
	EmitSignal =  44,			// emit signal event
	SockAct = 50,				// socket activation
	AccelOverride = 51,			// accelerator override event
	DeferredDelete = 52,			// deferred delete event
	DragEnter = 60,				// drag moves into widget
	DragMove = 61,				// drag moves in widget
	DragLeave = 62,				// drag leaves or is cancelled
	Drop = 63,				// actual drop
	DragResponse = 64,			// drag accepted/rejected
	ChildAdded = 68,			// new child widget
	ChildPolished = 69,			// polished child widget
#ifndef QT_NO_COMPAT
	ChildInserted = 70,			// compatibility posted insert
	LayoutHint = 72,			// compatibility relayout request
#endif
	ChildRemoved = 71,			// deleted child widget
	ShowWindowRequest = 73,			// widget's window should be mapped
	PolishRequest = 74,			// object should be polished
	Polish = 75,				// object is polished
	LayoutRequest = 76,			// widget should be relayouted
	UpdateRequest = 77,			// widget should be repainted
#ifdef Q_WS_QWS
	QWSUpdate = 78,
#endif
	ActivateControl = 80,			// ActiveX activation
	DeactivateControl = 81,			// ActiveX deactivation
	ContextMenu = 82,			// context popup menu
	IMStart = 83,				// input method composition start
	IMCompose = 84,				// input method composition
	IMEnd = 85,				// input method composition end
	Accessibility = 86,			// accessibility information is requested
	TabletMove = 87,			// Wacom tablet event
	LocaleChange = 88,			// the system locale changed
	LanguageChange = 89,			// the application language changed
	LayoutDirectionChange = 90,		// the layout direction changed
	Style = 91,				// internal style event
	TabletPress = 92,			// tablet press
	TabletRelease = 93,			// tablet release
	OkRequest = 94,				// CE (Ok) button pressed
	HelpRequest = 95,			// CE (?)  button pressed

	FontChange = 96,                        // font has changed
	EnabledChange = 97,                     // enabled state has changed
	ActivationChange = 98,                  // window activation has changed
	StyleChange = 99,                       // style has changed
	IconTextChange = 100,                   // icon text has changed
	ModifiedChange = 101,                   // modified state has changed

	User = 1000,				// first user event id
	MaxUser = 65535				// last user event id
    };

    QEvent(Type type) : t(type), posted(FALSE), spont(FALSE) {}
    virtual ~QEvent();
    inline Type type() const { return static_cast<Type>(t); }
    inline bool spontaneous() const { return spont; }
protected:
    ushort  t;
private:
    ushort posted : 1;
    ushort spont : 1;

    friend class QKernelApplication;
    friend class QApplication;
    friend class QAccelManager;
    friend class QBaseApplication;
    friend class QETWidget;
};

class Q_KERNEL_EXPORT QTimerEvent : public QEvent
{
public:
    QTimerEvent( int timerId )
	: QEvent(Timer), id(timerId) {}
    int	  timerId()	const	{ return id; }
protected:
    int	  id;
};

class Q_KERNEL_EXPORT QChildEvent : public QEvent
{
public:
    QChildEvent( Type type, QObject *child )
	: QEvent(type), c(child) {}
    QObject *child() const { return c; }
    bool added() const { return type() == ChildAdded; }
#ifndef QT_NO_COMPAT
    bool inserted() const { return type() == ChildInserted; }
#endif
    bool polished() const { return type() == ChildPolished; }
    bool removed() const { return type() == ChildRemoved; }
protected:
    QObject *c;
};

class Q_KERNEL_EXPORT QCustomEvent : public QEvent
{
public:
    QCustomEvent( int type );
    QCustomEvent( Type type, void *data )
	: QEvent(type), d(data) {};
    void       *data()	const	{ return d; }
    void	setData( void* data )	{ d = data; }
private:
    void       *d;
};

#endif /* __QKERNELEVENT_H__ */
