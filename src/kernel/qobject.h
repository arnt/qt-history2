/****************************************************************************
**
** Definition of QObject class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QOBJECT_H
#define QOBJECT_H

#ifndef QT_H
#include "qobjectdefs.h"
#include "qwindowdefs.h"
#include "qnamespace.h"
#include "qstring.h"
#include "qbytearray.h"
#endif // QT_H

#define QT_TR_NOOP(x) (x)
#define QT_TRANSLATE_NOOP(scope,x) (x)

struct QMetaObject;
class QVariant;
struct QObjectPrivate;
struct QWidgetPrivate;
#ifndef QT_NO_USERDATA
class QObjectUserData;
#endif
class QEvent;
class QTimerEvent;
class QChildEvent;
class QCustomEvent;
template<typename T>class QList;
typedef QList<QObject *> QObjectList;

class Q_EXPORT QObject: public Qt
{
    Q_OBJECT
    Q_PROPERTY( QByteArray name READ name WRITE setName )

public:
    QObject(QObject *parent=0, const char *name=0);
    virtual ~QObject();

    const char *className() const;
#ifdef Q_QDOC
    static QString tr(const char *, const char *);
    static QString trUtf8(const char *, const char *);
    virtual const QMetaObject *metaObject() const;
#endif
#ifdef QT_NO_TRANSLATION
    static QString tr( const char *sourceText, const char * = 0)
	{ return QString::fromLatin1(sourceText); }
#ifndef QT_NO_TEXTCODEC
    static QString trUtf8( const char *sourceText, const char * = 0)
	{ return QString::fromUtf8(sourceText); }
#endif
#endif //QT_NO_TRANSLATION

    const char *name() const;
    const char *name(const char *defaultName) const;

    virtual void setName(const char *name);
    inline bool isWidgetType() const { return isWidget; }

    inline bool signalsBlocked() const { return blockSig; }
    bool blockSignals(bool b);

    int startTimer(int interval);
    void killTimer(int id);

    QObject *child(const char *objName, const char *inheritsClass = 0,
		   bool recursiveSearch = true) const;
    const QObjectList &children() const;

    QObjectList queryList(const char *inheritsClass = 0,
			  const char *objName = 0,
			  bool regexpMatch = true,
			  bool recursiveSearch = true) const;

    void setParent(QObject *);
    void installEventFilter(const QObject *);
    void removeEventFilter(const QObject *);

    static bool connect(const QObject *sender, const char *signal,
			const QObject *receiver, const char *member);
    inline bool connect(const QObject *sender, const char *signal,
			const char *member) const
	{ return connect(sender, signal, this, member); }

    static bool disconnect(const QObject *sender, const char *signal,
			   const QObject *receiver, const char *member);
    inline bool disconnect(const char *signal = 0,
			   const QObject *receiver = 0, const char *member = 0)
	{ return disconnect(this, signal, receiver, member); }
    inline bool disconnect(const QObject *receiver, const char *member = 0)
	{ return disconnect(this, 0, receiver, member); }

    void dumpObjectTree();
    void dumpObjectInfo();

#ifndef QT_NO_PROPERTIES
    bool setProperty(const char *name, const QVariant &value);
    QVariant property(const char *name) const;
#endif // QT_NO_PROPERTIES

#ifndef QT_NO_USERDATA
    static uint registerUserData();
    void setUserData(uint id, QObjectUserData* data);
    QObjectUserData* userData(uint id) const;
#endif // QT_NO_USERDATA

    bool isAncestorOf(const QObject *child) const;

    void ensurePolished() const;

signals:
    void destroyed(QObject * = 0);

public:
    inline QObject *parent() const { return parentObj; }

public slots:
    void deleteLater();

protected:
    const QObject *sender();
    int receivers(const char* signal ) const;

    virtual bool event(QEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

    virtual void timerEvent(QTimerEvent *);
    virtual void childEvent(QChildEvent *);
    virtual void polishEvent(QEvent *);
    virtual void customEvent(QCustomEvent *);

    virtual void connectNotify(const char *signal);
    virtual void disconnectNotify(const char *signal);

#ifndef QT_NO_COMPAT
public:
    inline void insertChild(QObject *o)
	{ if (o) o->setParent(this); }
    inline void removeChild(QObject *o)
	{ if (o) o->setParent(0); }
    inline bool isA(const char *classname) const
	{ return qstrcmp(classname, className() ) == 0; }
    inline bool inherits(const char *classname) const
	{ return metaObject()->inherits(classname); }
protected:
    inline bool checkConnectArgs(const char *signal,
				  const QObject *,
				  const char *member)
	{ return QMetaObject::checkConnectArgs(signal, member); }
    static inline QByteArray normalizeSignalSlot(const char *signalSlot)
	{ return QMetaObject::normalizedSignature(signalSlot); }
#endif

protected:
    explicit QObject(QObjectPrivate *d, QObject *parent, const char *name);
private:
    explicit QObject(QWidgetPrivate *d, QObject *parent, const char *name);
    void setParent_helper(QObject *);
    uint isSignal : 1;
    uint isWidget : 1;
    uint pendTimer : 1;
    uint blockSig : 1;
    uint wasDeleted : 1;
    uint hasPostedEvents : 1;
#ifndef QT_NO_COMPAT
    uint hasPostedChildInsertedEvents : 1;
#endif
    uint unused : 26;

    const char *objname;
    QObject *parentObj;

protected:
    QObjectPrivate *d_ptr;
    Q_DECL_PRIVATE( QObject );

    static const QMetaObject staticQtMetaObject;

    friend struct QMetaObject;
    friend class QApplication;
    friend class QKernelApplication;
    friend class QWidget;
    friend class QSignal;

private: // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QObject(const QObject &);
    QObject &operator=(const QObject &);
#endif
};

inline bool QObject::isAncestorOf(const QObject *child) const
{
    while (child) {
	child = child->parentObj;
	if (child == this)
	    return true;
    }
    return false;
}


#ifndef QT_NO_USERDATA
class Q_EXPORT QObjectUserData {
public:
    virtual ~QObjectUserData();
};
#endif

template <class T>
class QPointer
{
    QObject *o;
public:
    inline QPointer() : o(0) {}
    inline QPointer(T *obj) : o(obj)
	{ QMetaObject::addGuard(&o); }
    inline QPointer(const QPointer<T> &p) : o(p.o)
	{ QMetaObject::addGuard(&o); }
    inline ~QPointer()
	{ QMetaObject::removeGuard(&o); }
    inline QPointer<T> &operator=(const QPointer<T> &p)
	{ QMetaObject::changeGuard(&o, p.o); return *this; }
    inline QPointer<T> &operator=(T* obj)
	{ QMetaObject::changeGuard(&o, obj); return *this; }

    inline bool operator==( const QPointer<T> &p ) const
	{ return o == p.o; }
    inline bool operator!= ( const QPointer<T>& p ) const
	{ return o != p.o; }

    inline bool isNull() const
	{ return !o; }

    inline T* operator->() const
	{ return static_cast<T*>(const_cast<QObject*>(o)); }
    inline T& operator*() const
	{ return *static_cast<T*>(const_cast<QObject*>(o)); }
    inline operator T*() const
	{ return static_cast<T*>(const_cast<QObject*>(o)); }
};

typedef QPointer<QObject> QObjectPointer;



class Q_EXPORT QEvent: public Qt		// event base class
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
	ApplicationFontChange = 36,		// application font changed
	ApplicationPaletteChange = 38,		// application palette changed
	PaletteChange = 39,			// widget palette changed
	Clipboard = 40,				// internal clipboard event
	Speech = 42,				// reserved for speech input
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
    friend class QAccelManager;
    friend class QBaseApplication;
    friend class QETWidget;
};


#define Q_DEFINED_QOBJECT
#include "qwinexport.h"
#endif // QOBJECT_H
