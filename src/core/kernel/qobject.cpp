/****************************************************************************
**
** Implementation of QObject class.
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

#include "qobject.h"
#include "qobject_p.h"

#include "qcoreapplication.h"
#include "qcorevariant.h"
#include "qeventloop.h"
#include "qmetaobject.h"

#include <qdebug.h>
#include <qregexp.h>
#include <qthread.h>

#include <new>

#include <ctype.h>
#include <limits.h>

#define d d_func()
#define q q_func()


static const int GUARDED_SIGNAL = INT_MIN;
static int DIRECT_CONNECTION_ONLY = 0;


/*!\internal
 */
void QMetaObject::addGuard(QObject **ptr)
{
    if (!*ptr) return;
    (*ptr)->d->addConnection(GUARDED_SIGNAL, reinterpret_cast<QObject*>(ptr), 0);
}

/*!\internal
 */
void QMetaObject::removeGuard(QObject **ptr)
{
    if (!*ptr) return;
    (*ptr)->d->removeReceiver(reinterpret_cast<QObject*>(ptr));
}

/*!\internal
 */
void QMetaObject::changeGuard(QObject **ptr, QObject *o)
{
    removeGuard(ptr);
    *ptr = o;
    addGuard(ptr);
}

/*! \internal
 */
QMetaCallEvent::QMetaCallEvent(QEvent::Type type, int id, const QObject *sender,
                               int nargs, int *types, void **args)
    :QEvent(type), id_(id), sender_(sender), nargs_(nargs), types_(types), args_(args)
{ }

/*! \internal
 */
QMetaCallEvent::~QMetaCallEvent()
{
    for (int i = 0; i < nargs_; ++i) {
        if (types_[i] && args_[i])
            QMetaType::destroy(types_[i], args_[i]);
    }
    if (types_) qFree(types_);
    if (args_) qFree(args_);
}




/*!
    \class Qt qnamespace.h

    \brief The Qt class is a namespace for miscellaneous identifiers
    that need to be global-like.

    \ingroup misc

    Normally, you can ignore this class. QObject and a few other
    classes inherit it, so all the identifiers in the Qt namespace are
    normally usable without qualification.

    However, you may occasionally need to say \c Qt::black instead of
    just \c black, particularly in static utility functions (such as
    many class factories).

*/

/*!
    \enum Qt::Orientation

    This type is used to signify an object's orientation.

    \value Horizontal
    \value Vertical

    Orientation is used with QScrollBar for example.
*/


/*!
    \class QObject qobject.h
    \brief The QObject class is the base class of all Qt objects.

    \ingroup objectmodel
    \mainclass
    \reentrant

    QObject is the heart of the \link object.html Qt object model.
    \endlink The central feature in this model is a very powerful
    mechanism for seamless object communication called \link
    signalsandslots.html signals and slots \endlink. You can
    connect a signal to a slot with connect() and destroy the
    connection with disconnect(). To avoid never ending notification
    loops you can temporarily block signals with blockSignals(). The
    protected functions connectNotify() and disconnectNotify() make it
    possible to track connections.

    QObjects organize themselves in object trees. When you create a
    QObject with another object as parent, the object will
    automatically add itself to the parent's children() list. The
    parent takes ownership of the object i.e. it will automatically
    delete its children in its destructor. You can look for an object
    by name and optionally type using child() or queryList(), and get
    the list of tree roots using objectTrees().

    Every object has an objectName() and can report its className()
    and whether it inherits() another class in the QObject inheritance
    hierarchy.

    When an object is deleted, it emits a destroyed() signal. You can
    catch this signal to avoid dangling references to QObjects. The
    QPointer class provides an elegant way to use this feature.

    QObjects can receive events through event() and filter the events
    of other objects. See installEventFilter() and eventFilter() for
    details. A convenience handler, childEvent(), can be reimplemented
    to catch child events.

    Last but not least, QObject provides the basic timer support in
    Qt; see QTimer for high-level support for timers.

    Notice that the Q_OBJECT macro is mandatory for any object that
    implements signals, slots or properties. You also need to run the
    \link moc.html moc program (Meta Object Compiler) \endlink on the
    source file. We strongly recommend the use of this macro in \e all
    subclasses of QObject regardless of whether or not they actually
    use signals, slots and properties, since failure to do so may lead
    certain functions to exhibit undefined behaviour.

    All Qt widgets inherit QObject. The convenience function
    isWidgetType() returns whether an object is actually a widget. It
    is much faster than inherits("QWidget").

    Some QObject functions, e.g. children(), objectTrees() and
    queryList() return a QObjectList. A QObjectList is a QList of
    QObjects.
*/


//
// Remove white space from SIGNAL and SLOT names.
// Internal for QObject::connect() and QObject::disconnect()
//

static inline bool isIdentChar(char x)
{                                                // Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
         (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

static inline bool isSpace(char x)
{
#if defined(Q_CC_BOR)
  /*
    Borland C++ 4.5 has a weird isspace() bug.
    isspace() usually works, but not here.
    This implementation is sufficient for our internal use: rmWS()
  */
    return (uchar) x <= 32;
#else
    return isspace((uchar) x);
#endif
}

// Event functions, implemented in qapplication_xxx.cpp
/*!
    \relates QObject

    Returns a pointer to the object named \a name that inherits \a
    type and with a given \a parent.

    Returns 0 if there is no such child.

    \code
        QListBox *c = (QListBox *) qt_find_obj_child(myWidget, "QListBox",
                                                      "my list box");
        if (c)
            c->insertItem("another string");
    \endcode
*/

void *qt_find_obj_child(QObject *parent, const char *type, const char *name)
{
    QObjectList list = parent->children();
    if (list.size() == 0) return 0;
    for (int i = 0; i < list.size(); ++i) {
        QObject *obj = list.at(i);
        if (qstrcmp(name,obj->objectName()) == 0 && obj->inherits(type))
            return obj;
    }
    return 0;
}


/*****************************************************************************
  QObject member functions
 *****************************************************************************/

/*!
    Constructs an object with parent object \a parent.

    The parent of an object may be viewed as the object's owner. For
    instance, a \link QDialog dialog box\endlink is the parent of the
    "OK" and "Cancel" buttons it contains.

    The destructor of a parent object destroys all child objects.

    Setting \a parent to 0 constructs an object with no parent. If the
    object is a widget, it will become a top-level window.

    \sa parent(), child(), queryList()
*/

QObject::QObject(QObject *parent)
    : d_ptr(new QObjectPrivate)
{
    d_ptr->q_ptr = this;
    d->thread = QThread::currentThread();
    setParent(parent);
    QEvent e(QEvent::Create);
    QCoreApplication::sendEvent(this, &e);
    QCoreApplication::postEvent(this, new QEvent(QEvent::PolishRequest));
}


/*!
    \overload
    \obsolete
 */
QObject::QObject(QObject *parent, const char *name)
    : d_ptr(new QObjectPrivate)
{
    d_ptr->q_ptr = this;
    d->thread = QThread::currentThread();
    setParent(parent);
    if (name)
        setObjectName(name);
    QEvent e(QEvent::Create);
    QCoreApplication::sendEvent(this, &e);
    QCoreApplication::postEvent(this, new QEvent(QEvent::PolishRequest));
}

/*! \internal
 */
QObject::QObject(QObjectPrivate &dd, QObject *parent)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    d->thread = QThread::currentThread();
    if (d->isWidget) {
        if (parent) {
            d->parent = parent;
            d->parent->d->children.append(this);
            d->thread = 0;
        }
        // no events sent here, this is done at the end of the QWidget constructor
    } else {
        setParent(parent);
        QEvent e(QEvent::Create);
        QCoreApplication::sendEvent(this, &e);
        QCoreApplication::postEvent(this, new QEvent(QEvent::PolishRequest));
    }
}

/*!
    Destroys the object, deleting all its child objects.

    All signals to and from the object are automatically disconnected.

    \warning All child objects are deleted. If any of these objects
    are on the stack or global, sooner or later your program will
    crash. We do not recommend holding pointers to child objects from
    outside the parent. If you still do, the QObject::destroyed()
    signal gives you an opportunity to detect when an object is
    destroyed.

    \warning Deleting a QObject while pending events are waiting to be
    delivered can cause a crash.  You must not delete the QObject
    directly from a thread that is not the GUI thread.  Use the
    QObject::deleteLater() method instead, which will cause the event
    loop to delete the object after all pending events have been
    delivered to the object.
*/

QObject::~QObject()
{
    if (d->wasDeleted) {
#if defined(QT_DEBUG)
        qWarning("Double QObject deletion detected");
#endif
        return;
    }
    d->wasDeleted = true;

    QEvent e(QEvent::Destroy);
    QCoreApplication::sendEvent(this, &e);

    d->blockSig = 0; // unblock signals so we always emit destroyed()
    emit destroyed(this);

    // disconnect receivers
    if (d->connections) {
        d->connections->lock.acquire();

        // reset all guarded pointers
        int i = 0;
        QObjectPrivate::Connections::Connection *c;
        while ((c = d->findConnection(GUARDED_SIGNAL, i))) {
            (void) qAtomicSetPtr(c->guarded, (QObject *) 0);
            (void) qAtomicSetPtr(&c->guarded, (QObject **) 0);
        }
        for (int i = 0; i < d->connections->count; ++i) {
            QObjectPrivate::Connections::Connection &c = d->connections->connections[i];
            if (c.receiver)
                c.receiver->d->removeSender(this);
            if (c.types && c.types != &DIRECT_CONNECTION_ONLY) {
                qFree(c.types);
                c.types = 0;
            }
        }

        d->connections->lock.release();
        d->connections->lock.~QSpinLock();

        qFree(d->connections);
        d->connections = 0;
    }

    // disconnect senders
    if (d->senders) {
        d->senders->lock.acquire();

        for (int i = 0; i < d->senders->count; ++i) {
            QObjectPrivate::Senders::Sender &sender = d->senders->senders[i];
            if (sender.sender) sender.sender->d->removeReceiver(this);
        }
        if (!--d->senders->ref) {
            if (d->senders->senders != d->senders->stack)
                qFree(d->senders->senders);

            d->senders->lock.release();
            d->senders->lock.~QSpinLock();

            qFree(d->senders);
        } else {
            d->senders->lock.release();
        }
        d->senders = 0;
    }

    // might have pending timers
    QEventLoop *eventloop = QEventLoop::instance(thread());
    if (eventloop && d->pendTimer)
        eventloop->unregisterTimers(this);

    d->eventFilters.clear();

    while (!d->children.isEmpty())                        // delete children objects
        delete d->children.takeFirst();

    QCoreApplication::removePostedEvents(this);

    if (d->parent)                                // remove it from parent object
        setParent_helper(0);

    if (d->objectName && d->ownObjectName) {
        delete [] (char*)d->objectName;
        d->objectName = 0;
    }

    delete d;
}


/*!
    \fn QMetaObject *QObject::metaObject() const

    Returns a pointer to the meta object of this object.

    A meta object contains information about a class that inherits
    QObject, e.g. class name, superclass name, properties, signals and
    slots. Every class that contains the Q_OBJECT macro will also have
    a meta object.

    The meta object information is required by the signal/slot
    connection mechanism and the property system. The functions isA()
    and inherits() also make use of the meta object.
*/

/*!
    Returns the class name of this object.

    \warning This function depends on the \link metaobjects.html Meta
    Object Compiler. \endlink It will return the wrong name if the
    class definion lacks the Q_OBJECT macro.
*/
const char *QObject::className() const
{
    return metaObject()->className();
}

/*!
    \fn bool QObject::isA(const char *clname) const

    \obsolete
    Returns true if this object is an instance of the class \a clname;
    otherwise returns false.

  Example:
  \code
    QTimer *t = new QTimer; // QTimer inherits QObject
    t->isA("QTimer");     // returns true
    t->isA("QObject");    // returns false
  \endcode

  \sa inherits() metaObject()
*/

/*! \fn Type *qt_cast<Type *>(QObject *o)

  \relates QObject

  Returns the object \a o cast to Type if the object is of type Type,
  otherwise returns 0.
*/


/*!
    \fn bool QObject::inherits(const char *clname) const

    Returns true if this object is an instance of a class that
    inherits \a clname or a QObject subclass that inherits classname,
    otherwise returns false.

    A class is considered to inherit itself.

    Consider using qt_cast<Type *>(object) instead. The method is both
    faster and safer.

    Example:
    \code
        QTimer *t = new QTimer;         // QTimer inherits QObject
        t->inherits("QTimer");        // returns true
        t->inherits("QObject");       // returns true
        t->inherits("QButton");       // returns false

        // QScrollBar inherits QWidget and QRangeControl
        QScrollBar *s = new QScrollBar(0);
        s->inherits("QWidget");       // returns true
        s->inherits("QRangeControl"); // returns true
    \endcode

    (\l QRangeControl is not a QObject.)

    \sa isA(), metaObject(), qt_cast
*/

/*!
    \property QObject::objectName

    \brief the name of this object

    You can find an object by name (and type) using child(). You can
    find a set of objects with queryList().

    If the object does not have a name, the objectName() function
    returns "unnamed", so printf() (used in qDebug()) will not be
    asked to output a null pointer. If you want a null pointer to be
    returned for unnamed objects, you can call objectName(0).

    \code
        qDebug("MyClass::setPrecision(): (%s) invalid precision %f",
                objectName(), newPrecision);
    \endcode

    \sa className(), child(), queryList()
*/

const char * QObject::objectName() const
{
    return d->objectName ? d->objectName : "unnamed";
}

/*!
    Sets the object's name to \a name.
*/
void QObject::setObjectName(const char *name)
{
    if (d->objectName && d->ownObjectName)
        delete [] (char*) d->objectName;
    d->ownObjectName = true;
    d->objectName = name ? qstrdup(name) : 0;
}

/*!
    Sets the object's name to \a name, not copying the string.
*/
void QObject::setObjectNameConst(const char *name)
{
    if (d->objectName && d->ownObjectName)
        delete [] (char*) d->objectName;
    d->ownObjectName = false;
    d->objectName = name;
}

/*!
    \overload

    Returns the name of this object, or \a defaultName if the object
    does not have a name.
*/

const char * QObject::objectName(const char * defaultName) const
{
    return d->objectName ? d->objectName : defaultName;
}


#ifdef QT_COMPAT
/*!
    Searches the children and optionally grandchildren of this object,
    and returns a child that is called \a objName that inherits \a
    inheritsClass. If \a inheritsClass is 0 (the default), any class
    matches.

    If \a recursiveSearch is true (the default), child() performs a
    depth-first search of the object's children.

    If there is no such object, this function returns 0. If there are
    more than one, the first one found is retured; if you need all of
    them, use queryList().
*/
QObject* QObject::child(const char *objName, const char *inheritsClass,
                         bool recursiveSearch) const
{
    if (d->children.isEmpty())
        return 0;

    bool onlyWidgets = (inheritsClass && qstrcmp(inheritsClass, "QWidget") == 0);
    for (int i = 0; i < d->children.size(); ++i) {
        QObject *obj = d->children.at(i);
        if (onlyWidgets) {
            if (obj->isWidgetType() && (!objName || qstrcmp(objName, obj->objectName()) == 0))
                return obj;
        } else if ((!inheritsClass || obj->inherits(inheritsClass)) && (!objName || qstrcmp(objName, obj->objectName()) == 0))
            return obj;
        if (recursiveSearch && (obj = obj->child(objName, inheritsClass, recursiveSearch)))
            return obj;
    }
    return 0;
}
#endif

/*!
    \fn bool QObject::isWidgetType() const

    Returns true if the object is a widget; otherwise returns false.

    Calling this function is equivalent to calling
    inherits("QWidget"), except that it is much faster.
*/


/*!
    This virtual function receives events to an object and should
    return true if the event \a e was recognized and processed.

    The event() function can be reimplemented to customize the
    behavior of an object.

    \sa installEventFilter(), timerEvent(), QApplication::sendEvent(),
    QApplication::postEvent(), QWidget::event()
*/

bool QObject::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Timer:
        timerEvent((QTimerEvent*)e);
        return true;

    case QEvent::ChildAdded:
    case QEvent::ChildPolished:
#ifdef QT_COMPAT
    case QEvent::ChildInserted:
#endif
    case QEvent::ChildRemoved:
        childEvent((QChildEvent*)e);
        return true;

    case QEvent::Polish:
        polishEvent(e);
        return true;

    case QEvent::DeferredDelete:
        delete this;
        return true;

    case QEvent::InvokeSlot:
    case QEvent::EmitSignal: {
        QMetaCallEvent *mce = static_cast<QMetaCallEvent*>(e);
        QObjectPrivate::Senders *senders = d->senders;
        QObject *sender =
            QObjectPrivate::setCurrentSender(senders, const_cast<QObject*>(mce->sender()));
#if defined(QT_NO_EXCEPTIONS)
        qt_metacall((e->type() == QEvent::InvokeSlot
                     ? QMetaObject::InvokeSlot
                     : QMetaObject::EmitSignal),
                    mce->id(), mce->args());
#else
        try {
            qt_metacall((e->type() == QEvent::InvokeSlot
                         ? QMetaObject::InvokeSlot
                         : QMetaObject::EmitSignal),
                        mce->id(), mce->args());
        } catch (...) {
            QObjectPrivate::resetCurrentSender(senders, sender);
            throw;
        }
#endif
        QObjectPrivate::resetCurrentSender(senders, sender);
        return true;
    }

    default:
        if (e->type() >= QEvent::User) {
            customEvent((QCustomEvent*) e);
            return true;
        }
        break;
    }
    return false;
}

/*!
    This event handler can be reimplemented in a subclass to receive
    timer events for the object.

    QTimer provides a higher-level interface to the timer
    functionality, and also more general information about timers.

    \sa startTimer(), killTimer(), event()
*/

void QObject::timerEvent(QTimerEvent *)
{
}


/*!
    This event handler can be reimplemented in a subclass to receive
    child events.

    \c QEvent::ChildAdded and \c QEvent::ChildRemoved events are sent
    to objects when children are added or removed. In both cases you
    can only rely on the child being a QObject, or if isWidgetType()
    returns true, a QWidget (Reason: in the \c ChildAdded case the
    child is not yet fully constructed, in the \c ChildRemoved case it
    might have been destructed already).

    \c QEvent::ChildPolished events are sent to objects when children
    are polished, or polished children added. If you receive a child
    polished event, the child's construction typically is completed.

    For every child widget you receive one \c ChildAdded event, zero
    or more \c ChildPolished events, and one \c ChildRemoved event.

    The polished event is omitted if a child is removed immediately
    after it is added. In turn, if a child is polished several times
    during construction and destruction, you may receive several child
    polished events for the same child, each time with a different
    virtual table.

    \sa event(), QChildEvent
*/

void QObject::childEvent(QChildEvent *)
{
}

/*!
    Ensures delayed initialization of an object and its children.

    This function will be called \e after an object has been fully
    created and \e before it is shown the very first time.  Children
    will be polished after the polishEvent() handler for this object
    is called.

    Polishing is useful for final initialization which depends on
    having an instantiated object. This is something a constructor
    cannot guarantee since the initialization of the subclasses might
    not be finished.

    For widgets, this function makes sure the widget has a proper font
    and palette and QApplication::polish() has been called.

    If you need to change some settings when an object is polished,
    use polishEvent().

    \sa polishEvent(), QApplication::polish()
*/
void QObject::ensurePolished() const
{
    const QMetaObject *m = metaObject();
    if (m == d->polished)
        return;
    d->polished = m;

    QEvent e(QEvent::Polish);
    QCoreApplication::sendEvent((QObject*)this, &e);

    // polish children after 'this'
    for (int i = 0; i < d->children.size(); ++i)
	d->children.at(i)->ensurePolished();

    if (d->parent) {
        QChildEvent e(QEvent::ChildPolished, (QObject*)this);
        QCoreApplication::sendEvent(d->parent, &e);
    }
}

/*!
    This event handler can be reimplemented in a subclass to receive
    object polish events.

    \sa event(), ensurePolished(), QApplication::polish()
*/
void QObject::polishEvent(QEvent *)
{
}

/*!
    This event handler can be reimplemented in a subclass to receive
    custom events. Custom events are user-defined events with a type
    value at least as large as the "User" item of the \l QEvent::Type
    enum, and is typically a QCustomEvent or QCustomEvent subclass.

    \sa event(), QCustomEvent
*/
void QObject::customEvent(QCustomEvent *)
{
}



/*!
    Filters events if this object has been installed as an event
    filter for the \a watched object.

    In your reimplementation of this function, if you want to filter
    the event \a e, out, i.e. stop it being handled further, return
    true; otherwise return false.

    Example:
    \code
    class MyMainWindow : public QMainWindow
    {
    public:
        MyMainWindow(QWidget *parent = 0, const char *name = 0);

    protected:
        bool eventFilter(QObject *obj, QEvent *ev);

    private:
        QTextEdit *textEdit;
    };

    MyMainWindow::MyMainWindow(QWidget *parent, const char *name)
        : QMainWindow(parent, name)
    {
        textEdit = new QTextEdit(this);
        setCentralWidget(textEdit);
        textEdit->installEventFilter(this);
    }

    bool MyMainWindow::eventFilter(QObject *obj, QEvent *ev)
    {
        if (obj == textEdit) {
            if (e->type() == QEvent::KeyPress) {
                qDebug("Ate key press %d", k->key());
                return true;
            } else {
                return false;
            }
        } else {
            // pass the event on to the parent class
            return QMainWindow::eventFilter(obj, ev);
        }
    }
    \endcode

    Notice in the example above that unhandled events are passed to
    the base class's eventFilter() function, since the base class
    might have reimplemented eventFilter() for its own internal
    purposes.

    \warning If you delete the receiver object in this function, be
    sure to return true. Otherwise, Qt will forward the event to the
    deleted object and the program might crash.

    \sa installEventFilter()
*/

bool QObject::eventFilter(QObject * /* watched */, QEvent * /* e */)
{
    return false;
}

/*!
    \fn bool QObject::signalsBlocked() const

    Returns true if signals are blocked; otherwise returns false.

    Signals are not blocked by default.

    \sa blockSignals()
*/

/*!
    Blocks signals if \a block is true, or unblocks signals if \a
    block is false.

    Emitted signals disappear into hyperspace if signals are blocked.
    Note that the destroyed() signals will be emitted even if the signals
    for this object have been blocked.
*/

bool QObject::blockSignals(bool block)
{
    bool previous = d->blockSig;
    d->blockSig = block;
    return previous;
}

/*!
    Returns the thread id in which this object was created.
 */
Qt::HANDLE QObject::thread() const
{
    const QObject *toplevel = this;
    for (; toplevel && toplevel->d->parent; toplevel = toplevel->d->parent)
        ;
    Q_ASSERT(toplevel->d->thread != 0);
    return toplevel->d->thread;
}

/*!
    \internal

    Note: Any running timers for thie object are stopped in the
    object's eventloop .  They are \e not restarted in the new
    thread's eventloop.
 */
void QObject::setThread(Qt::HANDLE thr)
{
    Q_ASSERT_X(!d->parent, "QObject::setThread",
               "Cannot set the thread on an object with a parent.");
    Q_ASSERT_X(thr != 0, "QObject::setThread",
               "thread argument must not be zero.");

    // ### TODO: make sure this works for objects with children
    Q_ASSERT_X(d->children.size() == 0, "QObject::setThread",
               "TODO: make sure this works for objects with children");

    if (d->thread == thr) return;

    // might have pending timers
    QEventLoop *eventloop = QEventLoop::instance(thread());
    if (eventloop && d->pendTimer)
        eventloop->unregisterTimers(this);
    d->pendTimer = false;

    // move any posted events to the new thread
    extern void qt_movePostedEvents(QObject *object, Qt::HANDLE _from, Qt::HANDLE _to);
    qt_movePostedEvents(this, d->thread, thr);

    d->thread = thr;
}

//
// The timer flag hasTimer is set when startTimer is called.
// It is not reset when killing the timer because more than
// one timer might be active.
//

/*!
    Starts a timer and returns a timer identifier, or returns zero if
    it could not start a timer.

    A timer event will occur every \a interval milliseconds until
    killTimer() is called. If \a interval is 0, then the timer event
    occurs once every time there are no more window system events to
    process.

    The virtual timerEvent() function is called with the QTimerEvent
    event parameter class when a timer event occurs. Reimplement this
    function to get timer events.

    If multiple timers are running, the QTimerEvent::timerId() can be
    used to find out which timer was activated.

    Example:
    \code
    class MyObject : public QObject
    {
        Q_OBJECT
    public:
        MyObject(QObject *parent = 0, const char *name = 0);

    protected:
        void timerEvent(QTimerEvent *);
    };

    MyObject::MyObject(QObject *parent, const char *name)
        : QObject(parent, name)
    {
        startTimer(50);    // 50-millisecond timer
        startTimer(1000);  // 1-second timer
        startTimer(60000); // 1-minute timer
    }

    void MyObject::timerEvent(QTimerEvent *e)
    {
        qDebug("timer event, id %d", e->timerId());
    }
    \endcode

    There is practically no upper limit for the interval value (more
    than one year is possible). Note that QTimer's accuracy depends on
    the underlying operating system and hardware. Most platforms
    support an accuracy of 20ms; some provide more. If Qt is unable to
    deliver the requested number of timer clicks, it will silently
    discard some.

    The QTimer class provides a high-level programming interface with
    one-shot timers and timer signals instead of events.

    \sa timerEvent(), killTimer()
*/

int QObject::startTimer(int interval)
{
    d->pendTimer = true;                                // set timer flag
    QEventLoop *eventloop = QEventLoop::instance(thread());
    Q_ASSERT_X(eventloop, "QObject::startTimer", "Cannot start timer without an event loop");
    return eventloop->registerTimer(interval, (QObject *)this);
}

/*!
    Kills the timer with timer identifier, \a id.

    The timer identifier is returned by startTimer() when a timer
    event is started.

    \sa timerEvent(), startTimer()
*/

void QObject::killTimer(int id)
{
    QEventLoop *eventloop = QEventLoop::instance(thread());
    if (eventloop) eventloop->unregisterTimer(id);
}

static void objSearch(QObjectList &result,
                       const QObjectList &list,
                       const char  *inheritsClass,
                       bool onlyWidgets,
                       const char  *objName,
                       QRegExp           *rx,
                       bool            recurse)
{
    for (int i = 0; i < list.size(); ++i) {
        QObject *obj = list.at(i);
        bool ok = true;
        if (onlyWidgets)
            ok = obj->isWidgetType();
        else if (inheritsClass && !obj->inherits(inheritsClass))
            ok = false;
        if (ok) {
            if (objName)
                ok = (qstrcmp(objName,obj->objectName()) == 0);
#ifndef QT_NO_REGEXP
            else if (rx)
                ok = (rx->search(QString::fromLatin1(obj->objectName())) != -1);
#endif
        }
        if (ok)                                // match!
            result.append(obj);
        if (recurse) {
            QObjectList clist = obj->children();
            if (!clist.isEmpty())
                objSearch(result, clist, inheritsClass,
                           onlyWidgets, objName, rx, recurse);
        }
    }
}

/*!
    \fn QObject *QObject::parent() const

    Returns a pointer to the parent object.

    \sa children()
*/

/*! \fn const QObjectList &QObject::children() const

    Returns a list of child objects, or 0 if this object has no
    children.

    The QObjectList class is defined in the \c qobjectlist.h header
    file.

    The first child added is the \link QList::first() first\endlink
    object in the list and the last child added is the \link
    QList::last() last\endlink object in the list, i.e. new
    children are appended at the end.

    Note that the list order changes when QWidget children are \link
    QWidget::raise() raised\endlink or \link QWidget::lower()
    lowered.\endlink A widget that is raised becomes the last object
    in the list, and a widget that is lowered becomes the first object
    in the list.

    \sa child(), queryList(), parent(), setParent()
*/


#ifdef QT_COMPAT
/*!
    Searches the children and optionally grandchildren of this object,
    and returns a list of those objects that are named or that match
    \a objName and inherit \a inheritsClass. If \a inheritsClass is 0
    (the default), all classes match. If \a objName is 0 (the
    default), all object names match.

    If \a regexpMatch is true (the default), \a objName is a regular
    expression that the objects's names must match. The syntax is that
    of a QRegExp. If \a regexpMatch is false, \a objName is a string
    and object names must match it exactly.

    Note that \a inheritsClass uses single inheritance from QObject,
    the way inherits() does. According to inherits(), QMenuBar
    inherits QWidget but not QMenuData. This does not quite match
    reality, but is the best that can be done on the wide variety of
    compilers Qt supports.

    Finally, if \a recursiveSearch is true (the default), queryList()
    searches \e{n}th-generation as well as first-generation children.

    If all this seems a bit complex for your needs, the simpler
    child() function may be what you want.

    This somewhat contrived example disables all the buttons in this
    window:
    \code
    QObjectList *l = topLevelWidget()->queryList("QButton");
    QObjectListIterator it(*l); // iterate over the buttons
    QObject *obj;

    while ((obj = it.current()) != 0) {
        // for each found object...
        ++it;
        ((QButton*)obj)->setEnabled(false);
    }
    delete l; // delete the list, not the objects
    \endcode

    The QObjectList class is defined in the \c qobjectlist.h header
    file.

    \warning Delete the list as soon you have finished using it. The
    list contains pointers that may become invalid at almost any time
    without notice (as soon as the user closes a window you may have
    dangling pointers, for example).

    \sa child() children(), parent(), inherits(), objectName(), QRegExp
*/

QObjectList QObject::queryList(const char *inheritsClass,
                                const char *objName,
                                bool regexpMatch,
                                bool recursiveSearch) const
{
    QObjectList list;
    bool onlyWidgets = (inheritsClass && qstrcmp(inheritsClass, "QWidget") == 0);
#ifndef QT_NO_REGEXP
    if (regexpMatch && objName) {                // regexp matching
        QRegExp rx(QString::fromLatin1(objName));
        objSearch(list, d->children, inheritsClass, onlyWidgets, 0, &rx, recursiveSearch);
    } else
#endif
    {
        objSearch(list, d->children, inheritsClass, onlyWidgets, objName, 0, recursiveSearch);
    }
    return list;
}
#endif

QObject *QObject::findChild(const char *name) const
{
    return findChild_helper(name, QObject::staticMetaObject);
}

QObjectList QObject::findChildren(const char *name) const
{
    QList<QObject *> list;
    findChildren_helper(name, 0, QObject::staticMetaObject,
                        reinterpret_cast<QList<void *>*>(&list));
    return list;
}

#ifndef QT_NO_REGEXP
QObjectList QObject::findChildren(const QRegExp &re) const
{
    QList<QObject *> list;
    findChildren_helper(0, &re, QObject::staticMetaObject,
                        reinterpret_cast<QList<void *>*>(&list));
    return list;
}
#endif

/*! \internal
 */
void QObject::findChildren_helper(const char *name, const QRegExp *re,
                         const QMetaObject &mo, QList<void*> *list) const
{
    QObject *obj;
    for (int i = 0; i < d->children.size(); ++i) {
        obj = d->children.at(i);
        if (mo.cast(obj)) {
            if (re) {
                if (re->search(QString::fromLatin1(obj->d->objectName)) != -1)
                    list->append(obj);
            } else {
                if (!name || qstrcmp(obj->d->objectName, name) == 0)
                    list->append(obj);
            }
        }
        obj->findChildren_helper(name, re, mo, list);
    }
}

/*! \internal
 */
QObject *QObject::findChild_helper(const char *name, const QMetaObject &mo) const
{
    QObject *obj;
    int i;
    for (i = 0; i < d->children.size(); ++i) {
        obj = d->children.at(i);
        if (mo.cast(obj) && (!name || qstrcmp(obj->d->objectName, name) == 0))
            return obj;
    }
    for (i = 0; i < d->children.size(); ++i) {
        obj = d->children.at(i)->findChild_helper(name, mo);
        if (obj)
            return obj;
    }
    return 0;
}

/*!
    Makes the object a child of \a parent.

    \sa QWidget::setParent()
*/

void QObject::setParent(QObject *parent)
{
    Q_ASSERT(!d->isWidget);
    setParent_helper(parent);
}


void QObject::setParent_helper(QObject *parent)
{
    if (parent == d->parent)
        return;
    if (d->parent && d->parent->d->children.removeAll(this)) {
        QChildEvent e(QEvent::ChildRemoved, this);
        QCoreApplication::sendEvent(d->parent, &e);
    }
    const Qt::HANDLE thr = thread();
    d->parent = parent;
    if (d->parent) {
        // object heirarchies are constrained to a single thread
        Q_ASSERT_X(thr == d->parent->thread(), "QObject::setParent",
                   "New parent must be in the same thread as the previous parent");
        d->thread = 0;

        d->parent->d->children.append(this);
        const QMetaObject *polished = d->polished;
        QChildEvent e(QEvent::ChildAdded, this);
        QCoreApplication::sendEvent(d->parent, &e);
        if (polished) {
            QChildEvent e(QEvent::ChildPolished, this);
            QCoreApplication::sendEvent(d->parent, &e);
        }
#ifdef QT_COMPAT
        QCoreApplication::postEvent(d->parent, new QChildEvent(QEvent::ChildInserted, this));
#endif
    } else {
        const Qt::HANDLE current = QThread::currentThread();
        if (thr != current) setThread(current);
    }
}

/*!
    \fn void QObject::installEventFilter(const QObject *filterObj)

    Installs an event filter \a filterObj on this object. For example:
    \code
    monitoredObj->installEventFilter(filterObj);
    \endcode

    An event filter is an object that receives all events that are
    sent to this object. The filter can either stop the event or
    forward it to this object. The event filter \a filterObj receives
    events via its eventFilter() function. The eventFilter() function
    must return true if the event should be filtered, (i.e. stopped);
    otherwise it must return false.

    If multiple event filters are installed on a single object, the
    filter that was installed last is activated first.

    Here's a \c KeyPressEater class that eats the key presses of its
    monitored objects:
    \code
    class KeyPressEater : public QObject
    {
        ...
    protected:
        bool eventFilter(QObject *o, QEvent *e);
    };

    bool KeyPressEater::eventFilter(QObject *o, QEvent *e)
    {
        if (e->type() == QEvent::KeyPress) {
            // special processing for key press
            QKeyEvent *k = (QKeyEvent *)e;
            qDebug("Ate key press %d", k->key());
            return true; // eat event
        } else {
            // standard event processing
            return false;
        }
    }
    \endcode

    And here's how to install it on two widgets:
    \code
        KeyPressEater *keyPressEater = new KeyPressEater(this);
        QPushButton *pushButton = new QPushButton(this);
        QListView *listView = new QListView(this);

        pushButton->installEventFilter(keyPressEater);
        listView->installEventFilter(keyPressEater);
    \endcode

    The QAccel class, for example, uses this technique to intercept
    accelerator key presses.

    \warning If you delete the receiver object in your eventFilter()
    function, be sure to return true. If you return false, Qt sends
    the event to the deleted object and the program will crash.

    \sa removeEventFilter(), eventFilter(), event()
*/

void QObject::installEventFilter(const QObject *obj)
{
    if (!obj)
        return;
    QObject *o = const_cast<QObject *>(obj);
    // clean up unused items in the list
    d->eventFilters.removeAll((QObject*)0);
    d->eventFilters.removeAll(o);
    d->eventFilters.prepend(o);
}

/*!
    Removes an event filter object \a obj from this object. The
    request is ignored if such an event filter has not been installed.

    All event filters for this object are automatically removed when
    this object is destroyed.

    It is always safe to remove an event filter, even during event
    filter activation (i.e. from the eventFilter() function).

    \sa installEventFilter(), eventFilter(), event()
*/

void QObject::removeEventFilter(const QObject *obj)
{
    QObject *o = const_cast<QObject *>(obj);
    d->eventFilters.removeAll(o);
}


/*!
    \fn QObject::destroyed(QObject* obj)

    This signal is emitted immediately before the object \a obj is
    destroyed, and can not be blocked.

    All the objects's children are destroyed immediately after this
    signal is emitted.
*/

/*!
    Performs a deferred deletion of this object.

    Instead of an immediate deletion this function schedules a
    deferred delete event for processing when Qt returns to the main
    event loop.
*/
void QObject::deleteLater()
{
    QCoreApplication::postEvent(this, new QEvent(QEvent::DeferredDelete));
}

/*!
    \fn QString QObject::tr(const char *sourceText, const char * comment)
    \reentrant

    Returns a translated version of \a sourceText, or \a sourceText
    itself if there is no appropriate translated version. The
    translation context is Object with \a comment (0 by default).
    All Object subclasses using the Q_OBJECT macro automatically have
    a reimplementation of this function with the subclass name as
    context.

    \warning This method is reentrant only if all translators are
    installed \e before calling this method. Installing or removing
    translators while performing translations is not supported. Doing
    so will probably result in crashes or other undesirable behavior.

    \sa trUtf8() QApplication::translate()
        \link i18n.html Internationalization with Qt\endlink
*/

/*!
    \fn QString QObject::trUtf8(const char *sourceText,
                                 const char *comment)
    \reentrant

    Returns a translated version of \a sourceText, or
    QString::fromUtf8(\a sourceText) if there is no appropriate
    version. It is otherwise identical to tr(\a sourceText, \a
    comment).

    \warning This method is reentrant only if all translators are
    installed \e before calling this method. Installing or removing
    translators while performing translations is not supported. Doing
    so will probably result in crashes or other undesirable behavior.

    \sa tr() QApplication::translate()
*/


/*****************************************************************************
  Signals and slots
 *****************************************************************************/

/*
    Adds one reference to sender in this object's senders list. If
    sender has not yet an entry, creates one. Called from connect().
*/
void QObjectPrivate::refSender(QObject *sender)
{
    int i = 0;
    bool created = false;
    if (!senders) {
        Senders *s = (Senders *) qMalloc(sizeof(Senders));
        new (&s->lock) QSpinLock();
        s->senders = s->stack;
        s->ref = 1;
        s->count = 1;
        s->current = 0;

        s->lock.acquire();
        if (!q_atomic_test_and_set_ptr(&senders, 0, s)) {
            s->lock.release();
            s->lock.~QSpinLock();
            qFree(s);
        } else {
            created = true;
        }
    }

    if (!created) {
        senders->lock.acquire();
        while (i < senders->count) {
            if (senders->senders[i].sender == sender) {
                ++senders->senders[i].ref;
                senders->lock.release();
                return;
            }
            ++i;
        }
        i = 0;
        while (i < senders->count && senders->senders[i].sender)
            ++i;
        if (i == senders->count) {
            if (senders->senders != senders->stack) {
                senders->senders =
                    (Senders::Sender*)
                    qRealloc(senders->senders, (i+1)*sizeof(Senders::Sender));
            } else if (senders->ref > 1) { // we cannot realloc
                senders->senders =
                    (Senders::Sender*)
                    qMalloc((i+1)*sizeof(Senders::Sender));
                ::memcpy(senders->senders, senders->stack,
                         i*sizeof(Senders::Sender));
            } else {
                senders = (Senders*)qRealloc(senders, sizeof(Senders)
                                             + i * sizeof(Senders::Sender));
                senders->senders = senders->stack;
            }
            ++senders->count;
        }
    }
    senders->senders[i].sender = sender;
    senders->senders[i].ref = 1;

    senders->lock.release();
}

/*
    Removes one reference to sender from this object's senders
    list. If this was the last reference, removes the entire
    entry. Called from disconnect().
*/
void QObjectPrivate::derefSender(QObject *sender)
{
    if (!senders) return;

    QSpinLockLocker locker(&senders->lock);

    for (int i = 0; i < senders->count; ++i) {
        if (senders->senders[i].sender == sender) {
            if (!--senders->senders[i].ref) {
                senders->senders[i].sender = 0;
                if (senders->current == sender)
                    senders->current = 0;
            }
            break;
        }
    }
}

/*
    Removes sender from this object's list of senders. Called from
    sender's destructor.
*/
void QObjectPrivate::removeSender(QObject *sender)
{
    if (!senders) return;

    QSpinLockLocker locker(&senders->lock);

    for (int i = 0; i < senders->count; ++i) {
        if (senders->senders[i].sender == sender)
            senders->senders[i].sender = 0;
    }
    if (senders->current == sender)
        senders->current = 0;
}

/*
    Sets current sender, references senders list, and returns the
    previous current sender.
*/
QObject *QObjectPrivate::setCurrentSender(Senders *senders, QObject *sender)
{
    if (!senders) return 0;

    QSpinLockLocker locker(&senders->lock);

    register QObject *const previous = senders->current;
    senders->current = sender;
    ++senders->ref;
    return previous;
}

/*
    Resets current sender, dereferences senders list, and frees it
    when no longer referenced.
 */
void QObjectPrivate::resetCurrentSender(Senders *senders, QObject *sender)
{
    if (!senders) return;

    senders->lock.acquire();

    senders->current = 0;
    if (sender) {
        // only if the new current sender is still in the list
        for (int i = 0; i < senders->count; ++i) {
            if (senders->senders[i].sender == sender) {
                senders->current = sender;
                break;
            }
        }
    }

    if (!--senders->ref) {
        if (senders->senders != senders->stack)
            qFree(senders->senders);

        senders->lock.release();
        senders->lock.~QSpinLock();

        qFree(senders);
    } else {
        senders->lock.release();
    }
}


#ifndef QT_NO_DEBUG

static bool check_signal_macro(const QObject *sender, const char *signal,
                                const char *func, const char *op)
{
    int sigcode = (int)(*signal) - '0';
    if (sigcode != QSIGNAL_CODE) {
        if (sigcode == QSLOT_CODE)
            qWarning("Object::%s: Attempt to %s non-signal %s::%s",
                     func, op, sender->className(), signal+1);
        else
            qWarning("Object::%s: Use the SIGNAL macro to %s %s::%s",
                     func, op, sender->className(), signal);
        return false;
    }
    return true;
}

static bool check_member_code(int code, const QObject *object,
                               const char *member, const char *func)
{
    if (code != QSLOT_CODE && code != QSIGNAL_CODE) {
        qWarning("Object::%s: Use the SLOT or SIGNAL macro to "
                 "%s %s::%s", func, func, object->className(), member);
        return false;
    }
    return true;
}

static void err_member_notfound(int code, const QObject *object,
                                 const char *member, const char *func)
{
    const char *type = 0;
    switch (code) {
        case QSLOT_CODE:   type = "slot";   break;
        case QSIGNAL_CODE: type = "signal"; break;
    }
    if (strchr(member,')') == 0)                // common typing mistake
        qWarning("Object::%s: Parentheses expected, %s %s::%s",
                 func, type, object->className(), member);
    else
        qWarning("Object::%s: No such %s %s::%s",
                 func, type, object->className(), member);
}


static void err_info_about_objects(const char * func,
                                    const QObject * sender,
                                    const QObject * receiver)
{
    const char * a = sender->objectName(0), * b = receiver->objectName(0);
    if (a)
        qWarning("Object::%s:  (sender name:   '%s')", func, a);
    if (b)
        qWarning("Object::%s:  (receiver name: '%s')", func, b);
}

#endif // !QT_NO_DEBUG

int *QObjectPrivate::queuedConnectionTypes(const char *signal)
{
    int *types = 0;
    const char *s = signal;
    while (*s++ != '(') {}
    int nargs = 0;
    const char *e = s;
    while (*e != ')') {
        ++e;
        if (*e == ')' || *e == ',')
            ++nargs;
    }

    types = (int *) qMalloc((nargs+1)*sizeof(int));
    types[nargs] = 0;
    for (int n = 0; n < nargs; ++n) {
        e = s;
        while (*s != ',' && *s != ')')
            ++s;
        QByteArray type(e, s-e);
        ++s;

        if (type.endsWith('*')) {
            types[n] = QMetaType::type("void*");
        } else {
            types[n] = QMetaType::type(type);
        }
        if (!types[n]) {
            qWarning("QObject::connect: Cannot queue arguments of type '%s'", type.data());
            qFree(types);
            return 0;
        }
    }
    return types;
}

void QObjectPrivate::addConnection(int signal, QObject *receiver, int member, int type, int *types)
{
    int i = 0;
    bool created = false;
    if (!connections) {
        Connections *c = (Connections *) qMalloc(sizeof(Connections));
        new (&c->lock) QSpinLock();
        c->count = 1;
        c->active = false;
        c->dirty = false;

        c->lock.acquire();
        if (!q_atomic_test_and_set_ptr(&connections, 0, c)) {
            c->lock.release();
            c->lock.~QSpinLock();
            qFree(c);
        } else {
            created = true;
        }
    }

    if (!created) {
        connections->lock.acquire();

        if (connections->dirty && !connections->active) {
            // some items in the connection list have null-receivers
            // due to disconnect, indicated by the dirty flag.  The
            // list is not active currently, which gives us a splendid
            // opportunity to compress it.
            int j = 0;
            while (i < connections->count) {
                while (j < connections->count && !connections->connections[j].receiver)
                    ++j;
                if (j < connections->count)
                    connections->connections[i] = connections->connections[j++];
                else
                    connections->connections[i].receiver = 0;
                ++i;
            }
            connections->dirty = false;
        }


        // pick the first spare item from the end, otherwise grow the list
        i = connections->count;
        while (i >0 && !connections->connections[i-1].receiver)
            --i;
        if (i == connections->count) {
            connections = (Connections *) qRealloc(connections, sizeof(Connections) +
                                                   i*sizeof(Connections::Connection));
            ++connections->count;
        }
    }

    Connections::Connection &c = connections->connections[i];
    c.signal = signal;
    c.receiver = receiver;
    c.member = member;
    c.type = type;
    c.types = types;

    connections->lock.release();
}

/*! \internal

    Note: You MUST acquire the connections lock before calling this
    function.
*/
QObjectPrivate::Connections::Connection *QObjectPrivate::findConnection(int signal, int &i) const
{
    if (!connections) return 0;
    while (i < connections->count) {
        if (connections->connections[i].receiver
            && connections->connections[i].signal == signal)
            return &connections->connections[i++];
        ++i;
    }
    return 0;
}

void QObjectPrivate::removeReceiver(QObject *receiver)
{
    if (!connections) return;

    QSpinLockLocker locker(&connections->lock);

    for (int i = 0; i < connections->count; ++i) {
        QObjectPrivate::Connections::Connection &c = connections->connections[i];
        if (c.receiver == receiver) {
            c.receiver = 0;
            if (c.types && c.types != &DIRECT_CONNECTION_ONLY) {
                qFree(c.types);
                c.types = 0;
            }
        }
    }
}


/*!
    Returns a pointer to the object that sent the signal, if called in
    a slot activated by a signal; otherwise it returns 0. The pointer
    is valid only during the execution of the slot that calls this
    function.

    The pointer returned by this function becomes invalid if the
    sender is destroyed, or if the slot is disconnected from the
    sender's signal.

    \warning This function violates the object-oriented principle of
     modularity. However, getting access to the sender might be useful
     when many signals are connected to a single slot. The sender is
     undefined if the slot is called as a normal C++ function.
*/

const QObject *QObject::sender()
{
    if (d->senders) return d->senders->current;
    return 0;
}

/*!
    Returns the number of receivers connect to the \a signal.

    \warning This function violates the object-oriented principle of
    modularity. However, it might be useful when you need to perform
    expensive initialization only if something is connected to a
    signal.
*/

int QObject::receivers(const char *signal) const
{
    int receivers = 0;
    if (d->connections && signal) {
        QByteArray signal_name = QMetaObject::normalizedSignature(signal);
        signal = signal_name;
#ifndef QT_NO_DEBUG
        if (!check_signal_macro(this, signal, "receivers", "bind"))
            return 0;
#endif
        signal++; // skip code
        const QMetaObject *smeta = this->metaObject();
        int signal_index = smeta->indexOfSignal(signal);
        if (signal_index < 0) {
#ifndef QT_NO_DEBUG
            err_member_notfound(QSIGNAL_CODE, this, signal, "receivers");
#endif
            return false;
        }
        QSpinLockLocker locker(&d->connections->lock);
        for (int i = 0; d->findConnection(signal_index, i); ++receivers)
            ;
    }
    return receivers;
}


/*!
    \threadsafe

    Connects \a signal from the \a sender object to \a member in object
    \a receiver, and returns true if the connection succeeds; otherwise
    returns false.

    You must use the SIGNAL() and SLOT() macros when specifying the \a signal
    and the \a member, for example:
    \code
    QLabel     *label  = new QLabel;
    QScrollBar *scroll = new QScrollBar;
    QObject::connect(scroll, SIGNAL(valueChanged(int)),
                      label,  SLOT(setNum(int)));
    \endcode

    This example ensures that the label always displays the current
    scroll bar value. Note that the signal and slots parameters must not
    contain any variable names, only the type. E.g. the following would
    not work and return false:
    QObject::connect(scroll, SIGNAL(valueChanged(int v)),
                      label,  SLOT(setNum(int v)));

    A signal can also be connected to another signal:

    \code
    class MyWidget : public QWidget
    {
        Q_OBJECT
    public:
        MyWidget();

    signals:
        void myUsefulSignal();

    private:
        QPushButton *aButton;
    };

    MyWidget::MyWidget()
    {
        aButton = new QPushButton(this);
        connect(aButton, SIGNAL(clicked()), SIGNAL(myUsefulSignal()));
    }
    \endcode

    In this example, the MyWidget constructor relays a signal from a
    private member variable, and makes it available under a name that
    relates to MyWidget.

    A signal can be connected to many slots and signals. Many signals
    can be connected to one slot.

    If a signal is connected to several slots, the slots are activated
    in an arbitrary order when the signal is emitted.

    The function returns true if it successfully connects the signal
    to the slot. It will return false if it cannot create the
    connection, for example, if QObject is unable to verify the
    existence of either \a signal or \a member, or if their signatures
    aren't compatible.

    A signal is emitted for \e{every} connection you make, so if you
    duplicate a connection, two signals will be emitted. You can
    always break a connection using \c{disconnect()}.

    \sa disconnect()
*/

bool QObject::connect(const QObject *sender, const char *signal,
                      const QObject *receiver, const char *member,
                      Qt::ConnectionType type)
{
#ifndef QT_NO_DEBUG
    if (sender == 0 || receiver == 0 || signal == 0 || member == 0) {
        qWarning("Object::connect: Cannot connect %s::%s to %s::%s",
                 sender ? sender->className() : "(null)",
                 signal ? signal+1 : "(null)",
                 receiver ? receiver->className() : "(null)",
                 member ? member+1 : "(null)");
        return false;
    }
#endif
    QByteArray tmp_signal_name;

#ifndef QT_NO_DEBUG
    if (!check_signal_macro(sender, signal, "connect", "bind"))
        return false;
#endif
    const QMetaObject *smeta = sender->metaObject();
    ++signal; //skip code
    int signal_index = smeta->indexOfSignal(signal);
    if (signal_index < 0) {
        // check for normalized signatures
        tmp_signal_name = QMetaObject::normalizedSignature(signal).prepend(*(signal - 1));
        signal = tmp_signal_name.constData() + 1;
        signal_index = smeta->indexOfSignal(signal);
        if (signal_index < 0) {
#ifndef QT_NO_DEBUG
            err_member_notfound(QSIGNAL_CODE, sender, signal, "connect");
            err_info_about_objects("connect", sender, receiver);
#endif
            return false;
        }
    }

    QByteArray tmp_member_name;
    int membcode = member[0] - '0';

#ifndef QT_NO_DEBUG
    if (!check_member_code(membcode, receiver, member, "connect"))
        return false;
#endif
    ++member; // skip code

    const QMetaObject *rmeta = receiver->metaObject();
    int member_index = -1;
    switch (membcode) {
    case QSLOT_CODE:
        member_index = rmeta->indexOfSlot(member);
        break;
    case QSIGNAL_CODE:
        member_index = rmeta->indexOfSignal(member);
        break;
    }
    if (member_index < 0) {
        // check for normalized members
        tmp_member_name = QMetaObject::normalizedSignature(member);
        member = tmp_member_name.constData();
        switch (membcode) {
        case QSLOT_CODE:
            member_index = rmeta->indexOfSlot(member);
            break;
        case QSIGNAL_CODE:
            member_index = rmeta->indexOfSignal(member);
            break;
        }
        if (member_index < 0) {
#ifndef QT_NO_DEBUG
            err_member_notfound(membcode, receiver, member, "connect");
            err_info_about_objects("connect", sender, receiver);
#endif
            return false;
        }
    }
#ifndef QT_NO_DEBUG
    if (!QMetaObject::checkConnectArgs(signal, member)) {
        qWarning("Object::connect: Incompatible sender/receiver arguments"
                 "\n\t%s::%s --> %s::%s",
                 sender->className(), signal,
                 receiver->className(), member);
        return false;
    }
#endif

    int *types = 0;
    if (type == QueuedConnection && !(types = QObjectPrivate::queuedConnectionTypes(signal)))
        return false;

#ifndef QT_NO_DEBUG
    {
        QMetaMember smember = smeta->signal(signal_index), rmember;
        switch (membcode) {
        case QSLOT_CODE:
            rmember = rmeta->slot(member_index);
            break;
        case QSIGNAL_CODE:
            rmember = rmeta->signal(member_index);
            break;
        }
        if(smember.attributes() & QMetaMember::Compatability)
            qWarning("Object::connect: Connecting from COMPAT signal (%s).", signal);
        if(rmember.attributes() & QMetaMember::Compatability)
            qWarning("Object::connect: Connecting to COMPAT %s (%s).",
                     (membcode == QSLOT_CODE) ? "slot" : "signal", member);
        switch(rmember.access()) {
        case QMetaMember::Private:
            break;
        case QMetaMember::Protected:
            break;
        default:
            break;
        }
    }
#endif
    QMetaObject::connect(sender, signal_index, receiver, membcode, member_index, type, types);
    const_cast<QObject*>(sender)->connectNotify(signal - 1);
    return true;
}


/*!
    \threadsafe

    \fn bool QObject::connect(const QObject *sender, const char *signal, const char *member) const
    \overload

    Connects \a signal from the \a sender object to this object's \a
    member.

    Equivalent to: \c{Object::connect(sender, signal, this, member)}.

    \sa disconnect()
*/

/*!
    \threadsafe

    Disconnects \a signal in object \a sender from \a member in object
    \a receiver.

    A signal-slot connection is removed when either of the objects
    involved are destroyed.

    disconnect() is typically used in three ways, as the following
    examples demonstrate.
    \list 1
    \i Disconnect everything connected to an object's signals:
       \code
       disconnect(myObject, 0, 0, 0);
       \endcode
       equivalent to the non-static overloaded function
       \code
       myObject->disconnect();
       \endcode
    \i Disconnect everything connected to a specific signal:
       \code
       disconnect(myObject, SIGNAL(mySignal()), 0, 0);
       \endcode
       equivalent to the non-static overloaded function
       \code
       myObject->disconnect(SIGNAL(mySignal()));
       \endcode
    \i Disconnect a specific receiver:
       \code
       disconnect(myObject, 0, myReceiver, 0);
       \endcode
       equivalent to the non-static overloaded function
       \code
       myObject->disconnect(myReceiver);
       \endcode
    \endlist

    0 may be used as a wildcard, meaning "any signal", "any receiving
    object", or "any slot in the receiving object", respectively.

    The \a sender may never be 0. (You cannot disconnect signals from
    more than one object in a single call.)

    If \a signal is 0, it disconnects \a receiver and \a member from
    any signal. If not, only the specified signal is disconnected.

    If \a receiver is 0, it disconnects anything connected to \a
    signal. If not, slots in objects other than \a receiver are not
    disconnected.

    If \a member is 0, it disconnects anything that is connected to \a
    receiver. If not, only slots named \a member will be disconnected,
    and all other slots are left alone. The \a member must be 0 if \a
    receiver is left out, so you cannot disconnect a
    specifically-named slot on all objects.

    \sa connect()
*/
bool QObject::disconnect(const QObject *sender, const char *signal,
                         const QObject *receiver, const char *member)
{
    if (sender == 0 || (receiver == 0 && member != 0)) {
        qWarning("Object::disconnect: Unexpected null parameter");
        return false;
    }

    if (!sender->d->connections)
        return false;

    QByteArray signal_name;
    bool signal_found = false;
    if (signal) {
        signal_name = QMetaObject::normalizedSignature(signal);
        signal = signal_name;
#ifndef QT_NO_DEBUG
        if (!check_signal_macro(sender, signal, "disconnect", "unbind"))
            return false;
#endif
        signal++; // skip code
    }

    QByteArray member_name;
    int membcode = -1;
    bool member_found = false;
    if (member) {
        member_name = QMetaObject::normalizedSignature(member);
        member = member_name;
        membcode = member[0] - '0';
#ifndef QT_NO_DEBUG
        if (!check_member_code(membcode, receiver, member, "disconnect"))
            return false;
#endif
        member++; // skip code
    }

    /* We now iterate through all the sender's and receiver's meta
     * objects in order to also disconnect possibly shadowed signals
     * and slots with the same signature.
    */
    bool res = false;
    const QMetaObject *smeta = sender->metaObject();
    do {
        int signal_index = -1;
        if (signal) {
            signal_index = smeta->indexOfSignal(signal);
            if (signal_index < smeta->signalOffset())
                continue;
            signal_found = true;
        }

        if (!member) {
            res |= QMetaObject::disconnect(sender,
                                          signal_index,
                                          receiver,
                                          -1, -1);
        } else {
            const QMetaObject *rmeta = receiver->metaObject();
            do {
                int member_index = -1;
                switch (membcode) {
                case QSLOT_CODE:
                    member_index = rmeta->indexOfSlot(member);
                    if (member_index >= 0)
                        while (member_index < rmeta->slotOffset())
                            rmeta = rmeta->superClass();
                    break;
                case QSIGNAL_CODE:
                    member_index = rmeta->indexOfSignal(member);
                    if (member_index >= 0)
                        while (member_index < rmeta->signalOffset())
                            rmeta = rmeta->superClass();
                    break;
                }
                if (member_index < 0)
                    break;
                res |= QMetaObject::disconnect(sender,
                                              signal_index,
                                              receiver,
                                              membcode, member_index);
                member_found = true;
            } while ((rmeta = rmeta->superClass()));
        }
    } while (signal && (smeta = smeta->superClass()));

#ifndef QT_NO_DEBUG
    if (signal && !signal_found) {
        err_member_notfound(QSIGNAL_CODE, sender, signal, "disconnect");
        err_info_about_objects("disconnect", sender, receiver);
    } else if (member && !member_found) {
        err_member_notfound(membcode, receiver, member, "disconnect");
        err_info_about_objects("disconnect", sender, receiver);
    }
#endif
    if (res)
        const_cast<QObject*>(sender)->disconnectNotify(signal);
    return res;
}


/*!
    \threadsafe

    \fn bool QObject::disconnect(const char *signal, const QObject *receiver, const char *member)
    \overload

    Disconnects \a signal from \a member of \a receiver.

    A signal-slot connection is removed when either of the objects
    involved are destroyed.
*/

/*!
    \fn bool QObject::disconnect(const QObject *receiver, const char *member)
    \overload

    Disconnects all signals in this object from \a receiver's \a
    member.

    A signal-slot connection is removed when either of the objects
    involved are destroyed.
*/


/*!
    \fn void QObject::connectNotify(const char *signal)

    This virtual function is called when something has been connected
    to \a signal in this object.

    \warning This function violates the object-oriented principle of
    modularity. However, it might be useful when you need to perform
    expensive initialization only if something is connected to a
    signal.

    \sa connect(), disconnectNotify()
*/

void QObject::connectNotify(const char *)
{
}

/*!
    \fn void QObject::disconnectNotify(const char *signal)

    This virtual function is called when something has been
    disconnected from \a signal in this object.

    \warning This function violates the object-oriented principle of
    modularity. However, it might be useful for optimizing access to
    expensive resources.

    \sa disconnect(), connectNotify()
*/

void QObject::disconnectNotify(const char *)
{
}

/*!\internal

  \a types is a 0-terminated vector of meta types for queued
  connections.
*/
bool QMetaObject::connect(const QObject *sender, int signal_index,
                          const QObject *receiver,
                          int membcode, int member_index,
                          int type, int *types)
{
    if (membcode != QSLOT_CODE && membcode != QSIGNAL_CODE)
        return false;

    QObject *s = const_cast<QObject*>(sender);
    QObject *r = const_cast<QObject*>(receiver);
    s->d->addConnection(signal_index, r, (member_index<<1)+membcode-1, type, types);
    r->d->refSender(s);
    return true;
}


/*!\internal
 */
bool QMetaObject::disconnect(const QObject *sender, int signal_index,
                             const QObject *receiver, int membcode, int member_index)
{
    if (!sender || !sender->d->connections
         || (member_index >= 0
              && (membcode != QSLOT_CODE && membcode != QSIGNAL_CODE)))
        return false;
    QObject *s = const_cast<QObject*>(sender);
    QObject *r = const_cast<QObject*>(receiver);

    QSpinLockLocker locker(&s->d->connections->lock);
    bool success = false;
    for (int i = 0; i < s->d->connections->count; ++i) {
        QObjectPrivate::Connections::Connection &c = s->d->connections->connections[i];
        if (c.receiver && c.signal != GUARDED_SIGNAL
            && (signal_index < 0 || signal_index == c.signal)
            && (r == 0 || (c.receiver == r
                           && (member_index < 0
                               || (member_index<<1)+membcode-1 == c.member)))) {
            c.receiver->d->derefSender(s);
            c.receiver = 0;
            s->d->connections->dirty = true;
            if (c.types && c.types != &DIRECT_CONNECTION_ONLY) {
                qFree(c.types);
                c.types = 0;
            }
            success = true;
        }
    }
    return success;
}

/*!\internal
 */
void QMetaObject::connectSlotsByName(const QObject *o)
{
    if (!o)
        return;
    const QMetaObject *mo = o->metaObject();
    Q_ASSERT(mo);
    const QObjectList list(o->findChildren(0));
    for (int i = 0; i < mo->slotCount(); ++i) {
        const char *sig = mo->slot(i).signature();
        Q_ASSERT(sig);
        if (sig[0] != 'o' || sig[1] != 'n' || sig[2] != '_')
            continue;
        bool foundIt = false;
        for(int j = 0; j < list.count(); ++j) {
            const QObject *co = list.at(j);
            const char *objName = co->objectName(0);
            int len = qstrlen(objName);
            if (!len
                || qstrncmp(sig + 3, objName, len)
                || sig[len+3] != '_')
                continue;
            int sigIndex = co->metaObject()->indexOfSignal(sig + len + 4);
            if (sigIndex < 0)
                continue;
            if (QMetaObject::connect(co, sigIndex, o, QSLOT_CODE, i)) {
                foundIt = true;
                break;
            }
        }
        if (!foundIt)
            qWarning("QMetaObject::connectSlotsByName(): No matching signal for %s", sig);
    }
}

/*!\internal
 */
void QMetaObject::activate(QObject * const obj, int signal_index, void **argv)
{
    if (obj->d->blockSig || !obj->d->connections)
        return;
    int i = 0;
    QObjectPrivate::Connections::Connection *c = 0, *nc = 0;
    bool first = true;
    void *static_argv[] = { 0 };
    if (!argv)
        argv = static_argv;
    for (; first || c != 0; c = nc) {
        obj->d->connections->lock.acquire();
        obj->d->connections->active = true;
        if (first) {
            first = false;
            c = obj->d->findConnection(signal_index, i);
            if (!c) {
                obj->d->connections->lock.release();
                break;
            }
        }
        // find the next connection before activating the current
        // connection
        nc = obj->d->findConnection(signal_index, i);
        // determine if this connection should be sent immediately or
        // put into the event queue
        const bool queued = (c->type == Qt::QueuedConnection
                             || (c->type == Qt::AutoConnection
                                 && c->receiver->thread() != obj->thread()));
        if (queued && !c->types && c->types != &DIRECT_CONNECTION_ONLY) {
            QMetaMember m = obj->metaObject()->signal(signal_index);
            c->types = QObjectPrivate::queuedConnectionTypes(m.signature());
            if (!c->types) // cannot queue arguments
                c->types = &DIRECT_CONNECTION_ONLY;
            if (c->types == &DIRECT_CONNECTION_ONLY) { // cannot activate
                obj->d->connections->lock.release();
                continue;
            }
        }

        obj->d->connections->active = false;
        obj->d->connections->lock.release();

        if (queued) { // QueuedConnection
            int nargs = 1; // include return type
            while (c->types[nargs-1]) { ++nargs; }
            int *types = (int *) qMalloc(nargs*sizeof(int));
            void **args = (void **) qMalloc(nargs*sizeof(void *));
            types[0] = 0; // return type
            args[0] = 0; // return value
            for (int n = 1; n < nargs; ++n)
                args[n] = QMetaType::copy((types[n] = c->types[n-1]), argv[n]);
            QCoreApplication::postEvent(c->receiver,
                                        new QMetaCallEvent((c->member & 1)
                                                           ? QEvent::EmitSignal
                                                           : QEvent::InvokeSlot,
                                                           c->member >> 1, obj,
                                                           nargs, types, args));
        } else { // DirectConnection
            QObjectPrivate::Senders *senders = c->receiver->d->senders;
            QObject *sender = QObjectPrivate::setCurrentSender(senders, obj);
#if defined(QT_NO_EXCEPTIONS)
            c->receiver->qt_metacall((Call)((c->member & 1) + 1), c->member >> 1, argv);
#else
            try {
                c->receiver->qt_metacall((Call)((c->member & 1) + 1), c->member >> 1, argv);
            } catch (...) {
                QObjectPrivate::resetCurrentSender(senders, sender);
                throw;
            }
#endif
            QObjectPrivate::resetCurrentSender(senders, sender);
        }
    }
}

/*!\internal
 */
void QMetaObject::activate(QObject *obj, const QMetaObject *m, int local_signal_index, void **argv)
{
    activate(obj, m->signalOffset() + local_signal_index, argv);
}

/*****************************************************************************
  Properties
 *****************************************************************************/

#ifndef QT_NO_PROPERTIES

/*!
    Sets the value of the object's \a name property to \a value.

    Returns true if the operation was successful; otherwise returns
    false.

    Information about all available properties is provided through the
    metaObject().

    \sa property(), metaObject()
*/
bool QObject::setProperty(const char *name, const QCoreVariant &value)
{
    const QMetaObject* meta = metaObject();
    if (!name || !meta)
        return false;

    int id = meta->indexOfProperty(name);
    QMetaProperty p = meta->property(id);
#ifndef QT_NO_DEBUG
    if (!p.isWritable())
        qWarning("%s::setProperty(\"%s\", value) failed: property invalid,"
                  " read-only or does not exist", className(), name);
#endif
    return p.write(this, value);
}

/*!
    Returns the value of the object's \a name property.

    If no such property exists, the returned variant is invalid.

    Information about all available properties is provided through the
    metaObject().

    \sa setProperty(), QCoreVariant::isValid(), metaObject()
*/
QCoreVariant QObject::property(const char *name) const
{
    const QMetaObject* meta = metaObject();
    if (!name || !meta)
        return QCoreVariant();

    int id = meta->indexOfProperty(name);
    QMetaProperty p = meta->property(id);
#ifndef QT_NO_DEBUG
    if (!p.isReadable())
        qWarning("%s::property(\"%s\") failed:"
                  " property invalid or does not exist",
                  className(), name);
#endif
    return p.read(this);
}

#endif // QT_NO_PROPERTIES


/*****************************************************************************
  QObject debugging output routines.
 *****************************************************************************/

static void dumpRecursive(int level, QObject *object)
{
#if defined(QT_DEBUG)
    if (object) {
        QByteArray buf;
        buf.fill('\t', level/2);
        if (level % 2)
            buf += "    ";
        const char *name = object->objectName();
        QString flags="";
#if 0
        if (qApp->focusWidget() == object)
            flags += 'F';
        if (object->isWidgetType()) {
            QWidget * w = (QWidget *)object;
            if (w->isVisible()) {
                QString t("<%1,%2,%3,%4>");
                flags += t.arg(w->x()).arg(w->y()).arg(w->width()).arg(w->height());
            } else {
                flags += 'I';
            }
        }
#endif
        qDebug("%s%s::%s %s", (const char*)buf, object->className(), name,
            flags.latin1());
        QObjectList children = object->children();
        if (!children.isEmpty()) {
            for (int i = 0; i < children.size(); ++i)
                dumpRecursive(level+1, children.at(i));
        }
    }
#else
    Q_UNUSED(level)
    Q_UNUSED(object)
#endif
}

/*!
    Dumps a tree of children to the debug output.

    This function is useful for debugging, but does nothing if the
    library has been compiled in release mode (i.e. without debugging
    information).
*/

void QObject::dumpObjectTree()
{
    dumpRecursive(0, this);
}

/*!
    Dumps information about signal connections, etc. for this object
    to the debug output.

    This function is useful for debugging, but does nothing if the
    library has been compiled in release mode (i.e. without debugging
    information).
*/

void QObject::dumpObjectInfo()
{
#if defined(QT_DEBUG)
    qDebug("OBJECT %s::%s", className(), objectName("unnamed"));
    //#### signals and slots info missing
#endif
}

#ifndef QT_NO_USERDATA
/*!\internal
 */
uint QObject::registerUserData()
{
    static int user_data_registration = 0;
    return user_data_registration++;
}

/*!\internal
 */
QObjectUserData::~QObjectUserData()
{
}

/*!\internal
 */
void QObject::setUserData(uint id, QObjectUserData* data)
{
    d->userData.insert(id, data);
}

/*!\internal
 */
QObjectUserData* QObject::userData(uint id) const
{
    if ((int)id < d->userData.size())
        return d->userData.at(id);
    return 0;
}

#endif // QT_NO_USERDATA


#if !defined(Q_OS_MAC) || QT_MACOSX_VERSION >= 0x1030
#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QObject *o) {
    if (!o)
        return dbg << "QObject(0x0) ";
    dbg.nospace() << o->className() << "(" << (void *)o;
    if (o->objectName(0))
        dbg << ", name = \"" << o->objectName(0) << '\"';
    dbg << ')';
    return dbg.space();
}
#endif
#endif
