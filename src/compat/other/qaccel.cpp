/****************************************************************************
**
** Implementation of QAccel class.
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

#include "qaccel.h"

#ifndef QT_NO_ACCEL

#include "qsignal.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qevent.h"
#include "qlist.h"
#include "qwhatsthis.h"
#include "qstatusbar.h"
#include "q3dockwindow.h"
#include "private/qapplication_p.h"
#define d d_func()
#define q q_func()

/*!
    \class QAccel qaccel.h
    \brief The QAccel class handles keyboard accelerator and shortcut keys.

    \ingroup misc

    A keyboard accelerator triggers an action when a certain key
    combination is pressed. The accelerator handles all keyboard
    activity for all the children of one top-level widget, so it is
    not affected by the keyboard focus.

    In most cases, you will not need to use this class directly. Use
    the QAction class to create actions with accelerators that can be
    used in both menus and toolbars. If you're only interested in
    menus use QMenuData::insertItem() or QMenuData::setAccel() to make
    accelerators for operations that are also available on menus. Many
    widgets automatically generate accelerators, such as QButton,
    QGroupBox, QLabel (with QLabel::setBuddy()), QMenuBar and QTabBar.
    Example:
    \code
        QPushButton p("&Exit", parent); // automatic shortcut Qt::ALT+Qt::Key_E
        QMenu *fileMenu = new fileMenu(parent);
        fileMenu->addAction("Undo", parent, SLOT(undo()), Qt::CTRL+Qt::Key_Z);
    \endcode

    A QAccel contains a list of accelerator items that can be
    manipulated using insertItem(), removeItem(), clear(), key() and
    findKey().

    Each accelerator item consists of an identifier and a \l
    QKeySequence. A single key sequence consists of a keyboard code
    combined with modifiers (\c Qt::META, \c Qt::SHIFT, \c Qt::CTRL or \c Qt::ALT).
    For example, \c{Qt::CTRL + Qt::Key_P} could be a shortcut for
    printing a document. The key codes are listed in \c
    qnamespace.h. As an alternative, use the
    unicode code point of the character. For example, \c{'A'}
    gives the same accelerator as \c Qt::Key_A and \c{0x410} gives an
    accelerator for the cyrillic letter 'A' (which is different from the latin 'A').

    When an accelerator key is pressed, the accelerator sends out the
    signal activated() with a number that identifies this particular
    accelerator item. Accelerator items can also be individually
    connected, so that two different keys will activate two different
    slots (see connectItem() and disconnectItem()).

    The activated() signal is \e not emitted when two or more
    accelerators match the same key.  Instead, the first matching
    accelerator sends out the activatedAmbiguously() signal. By
    pressing the key multiple times, users can navigate between all
    matching accelerators. Some standard controls like QPushButton and
    QCheckBox connect the activatedAmbiguously() signal to the
    harmless setFocus() slot, whereas activated() is connected to a
    slot invoking the button's action.        Most controls, like QLabel and
    QTabBar, treat activated() and activatedAmbiguously() as
    equivalent.

    Use setEnabled() to enable or disable all the items in an
    accelerator, or setItemEnabled() to enable or disable individual
    items. An item is active only when both the QAccel and the item
    itself are enabled.

    The function setWhatsThis() specifies a help text that appears
    when the user presses an accelerator key in What's This mode.

    The accelerator will be deleted when \e parent is deleted,
    and will consume relevant key events until then.

    Please note that the accelerator
    \code
        accelerator->insertItem(QKeySequence("M"));
    \endcode
    can be triggered with both the 'M' key, and with Shift+M,
    unless a second accelerator is defined for the Shift+M
    combination.


    Example:
    \code
 QAccel *a = new QAccel(myWindow);           // create accels for myWindow
        a->connectItem(a->insertItem(Qt::Key_P+Qt::CTRL), // adds Ctrl+P accelerator
                        myWindow,                   // connected to myWindow's
                        SLOT(printDoc()));           // printDoc() slot
    \endcode

    \sa QKeyEvent QWidget::keyPressEvent()
    QLabel::setBuddy() QKeySequence
    \link guibooks.html#fowler GUI Design Handbook: Keyboard Shortcuts \endlink.
*/

struct QAccelItem
{
    QAccelItem(const QKeySequence &k, int i)
    { key = k; id = i; enabled = true; signal = 0; }
    ~QAccelItem() { delete signal; }

    int id;
    QKeySequence key;
    bool enabled;
    QSignalEmitter *signal;
    QString whatsthis;
};


typedef QList<QAccelItem *> QAccelList;

class QAccelPrivate : public QObjectPrivate
{
public:
    QAccelPrivate(QAccel* p);
    ~QAccelPrivate();

    QAccelList aitems;
    QPointer<QWidget> watch;
    bool ignorewhatsthis;
    bool enabled;
    // ### should not be needed anymore
    QAccel* parent;

    void activate(QAccelItem* item);
    void activateAmbiguously(QAccelItem* item);
};

class QAccelManager : public Qt {
public:
    static QAccelManager* self() { return self_ptr ? self_ptr : new QAccelManager; }
    void registerAccel(QAccelPrivate* a) { accels.append(a); }
    void unregisterAccel(QAccelPrivate* a) { accels.removeAll(a); if (accels.isEmpty()) delete this; }
    bool tryAccelEvent(QWidget* w, QKeyEvent* e);
    bool dispatchAccelEvent(QWidget* w, QKeyEvent* e);
    bool tryComposeUnicode(QWidget* w, QKeyEvent* e);

private:
    QAccelManager()
        : currentState(QKeySequence::NoMatch), clash(-1), metaComposeUnicode(false),composedUnicode(0)
        { setFuncPtr(); self_ptr = this; }
    ~QAccelManager() { self_ptr = 0; }
    void setFuncPtr();

    bool correctSubWindow(QWidget *w, QAccelPrivate* priv);
    QKeySequence::SequenceMatch match(QKeyEvent* e, QAccelItem* item, QKeySequence& temp);
    int translateModifiers(Qt::ButtonState state);

    QList<QAccelPrivate *> accels;
    static QAccelManager* self_ptr;
    QKeySequence::SequenceMatch currentState;
    QKeySequence intermediate;
    int clash;
    bool metaComposeUnicode;
    int composedUnicode;
};
QAccelManager* QAccelManager::self_ptr = 0;

bool Q_COMPAT_EXPORT qt_tryAccelEvent(QWidget *w, QKeyEvent *e){
    return QAccelManager::self()->tryAccelEvent(w, e);
}

bool Q_COMPAT_EXPORT qt_dispatchAccelEvent(QWidget *w, QKeyEvent *e){
    return QAccelManager::self()->dispatchAccelEvent(w, e);
}

bool Q_COMPAT_EXPORT qt_tryComposeUnicode(QWidget *w, QKeyEvent *e){
    return QAccelManager::self()->tryComposeUnicode(w, e);
}

void QAccelManager::setFuncPtr() {
    if (qApp->d->qt_compat_used)
        return;
    QApplicationPrivate *data = static_cast<QApplicationPrivate*>(qApp->d_ptr);
    data->qt_tryAccelEvent = qt_tryAccelEvent;
    data->qt_tryComposeUnicode = qt_tryComposeUnicode;
    data->qt_dispatchAccelEvent = qt_dispatchAccelEvent;
    data->qt_compat_used = true;
}


#ifdef Q_WS_MAC
static bool qt_accel_no_shortcuts = true;
#else
static bool qt_accel_no_shortcuts = false;
#endif
void Q_COMPAT_EXPORT qt_setAccelAutoShortcuts(bool b) { qt_accel_no_shortcuts = !b; }

/*
    \internal
    Returns true if the accel is in the current subwindow, else false.
*/
bool QAccelManager::correctSubWindow(QWidget* w, QAccelPrivate* priv) {
    if (!priv->watch || !priv->watch->isVisible() || !priv->watch->isEnabled())
        return false;
    QWidget* tlw = w->topLevelWidget();
    QWidget* wtlw = priv->watch->topLevelWidget();

    /* if we live in a floating dock window, keep our parent's
     * accelerators working */
    if (tlw->isDialog() && tlw->parentWidget() && ::qt_cast<Q3DockWindow*>(tlw))
        return tlw->parentWidget()->topLevelWidget() == wtlw;

    if (wtlw  != tlw)
        return false;

    /* if we live in a MDI subwindow, ignore the event if we are
       not the active document window */
    QWidget* sw = priv->watch;
    while (sw && !sw->testWFlags(Qt::WSubWindow) && !sw->isTopLevel())
        sw = sw->parentWidget();
    if (sw)  { // we are in a subwindow indeed
        QWidget* fw = w;
        while (fw && fw != sw && !fw->isTopLevel())
            fw = fw->parentWidget();
        if (fw != sw) // focus widget not in our subwindow
            return false;
    }
    return true;
}

inline int QAccelManager::translateModifiers(Qt::ButtonState state)
{
    int result = 0;
    if (state & Qt::ShiftButton)
        result |= Qt::SHIFT;
    if (state & Qt::ControlButton)
        result |= Qt::CTRL;
    if (state & Qt::MetaButton)
        result |= Qt::META;
    if (state & Qt::AltButton)
        result |= Qt::ALT;
    return result;
}

/*
    \internal
    Matches the current intermediate key sequence + the latest
    keyevent, with and AccelItem. Returns Identical,
    PartialMatch or NoMatch, and fills \a temp with the
    resulting key sequence.
*/
QKeySequence::SequenceMatch QAccelManager::match(QKeyEvent *e, QAccelItem* item, QKeySequence& temp)
{
    QKeySequence::SequenceMatch result = QKeySequence::NoMatch;
    int index = intermediate.count();
    temp = intermediate;

    int modifier = translateModifiers(e->state());

    if (e->key() && e->key() != Qt::Key_unknown) {
        int key = e->key()  | modifier;
        if (e->key() == Key_BackTab) {
            /*
            In QApplication, we map shift+tab to shift+backtab.
            This code here reverts the mapping in a way that keeps
            backtab and shift+tab accelerators working, in that
            order, meaning backtab has priority.*/
            key &= ~Qt::SHIFT;

            temp.setKey(key, index);
            if (QKeySequence::NoMatch != (result = temp.matches(item->key)))
                return result;
            if (e->state() & Qt::ShiftButton)
                key |= Qt::SHIFT;
            key = Qt::Key_Tab | (key & Qt::MODIFIER_MASK);
            temp.setKey(key, index);
            if (QKeySequence::NoMatch != (result = temp.matches(item->key)))
                return result;
        } else {
            temp.setKey(key, index);
            if (QKeySequence::NoMatch != (result = temp.matches(item->key)))
                return result;
        }

        if (key == Key_BackTab) {
            if (e->state() & Qt::ShiftButton)
                key |= Qt::SHIFT;
            temp.setKey(key, index);
            if (QKeySequence::NoMatch != (result = temp.matches(item->key)))
                return result;
        }
    }
    if (!e->text().isEmpty()) {
        temp.setKey((int)e->text().unicode()->toUpper().unicode() | modifier, index);
        result = temp.matches(item->key);
    }
    return result;
}

bool QAccelManager::tryAccelEvent(QWidget* w, QKeyEvent* e)
{
    if (QKeySequence::NoMatch == currentState) {
        e->t = QEvent::AccelOverride;
        e->ignore();
        QApplication::sendSpontaneousEvent(w, e);
        if (e->isAccepted())
            return false;
    }
    e->t = QEvent::Accel;
    e->ignore();
    QApplication::sendSpontaneousEvent(w, e);
    return e->isAccepted();
}

bool QAccelManager::tryComposeUnicode(QWidget* w, QKeyEvent* e)
{
    if (metaComposeUnicode) {
        int value = e->key() - Qt::Key_0;
        // Ignore acceloverrides so we don't trigger
        // accels on keypad when Meta compose is on
        if ((e->type() == QEvent::AccelOverride) &&
             (e->state() == Qt::Keypad + Qt::MetaButton)) {
            e->accept();
        // Meta compose start/continue
        } else if ((e->type() == QEvent::KeyPress) &&
             (e->state() == Qt::Keypad + Qt::MetaButton)) {
            if (value >= 0 && value <= 9) {
                composedUnicode *= 10;
                composedUnicode += value;
                return true;
            } else {
                // Composing interrupted, dispatch!
                if (composedUnicode) {
                    QChar ch(composedUnicode);
                    QString s(ch);
                    QKeyEvent kep(QEvent::KeyPress, 0, 0, s);
                    QKeyEvent ker(QEvent::KeyRelease, 0, 0, s);
                    QApplication::sendEvent(w, &kep);
                    QApplication::sendEvent(w, &ker);
                }
                composedUnicode = 0;
                return true;
            }
        // Meta compose end, dispatch
        } else if ((e->type() == QEvent::KeyRelease) &&
                    (e->key() == Qt::Key_Meta) &&
                    (composedUnicode != 0)) {
            if ((composedUnicode > 0) &&
                 (composedUnicode < 0xFFFE)) {
                QChar ch(composedUnicode);
                QString s(ch);
                QKeyEvent kep(QEvent::KeyPress, 0, 0, s);
                QKeyEvent ker(QEvent::KeyRelease, 0, 0, s);
                QApplication::sendEvent(w, &kep);
                QApplication::sendEvent(w, &ker);
            }
            composedUnicode = 0;
            return true;
        }
    }
    return false;
}

/*
    \internal
    Checks for possible accelerators, if no widget
    ate the keypres, or we are in the middle of a
    partial key sequence.
*/
bool QAccelManager::dispatchAccelEvent(QWidget* w, QKeyEvent* e)
{
#ifndef QT_NO_STATUSBAR
    // Needs to be declared and used here because of "goto doclash"
    QStatusBar* mainStatusBar = 0;
#endif

    // Modifiers can NOT be accelerators...
    if (e->key() >= Qt::Key_Shift &&
         e->key() <= Qt::Key_Alt)
         return false;

    QKeySequence::SequenceMatch result = QKeySequence::NoMatch;
    QKeySequence tocheck, partial;
    QAccelPrivate* accel = 0;
    QAccelItem* item = 0;
    QAccelPrivate* firstaccel = 0;
    QAccelItem* firstitem = 0;
    QAccelPrivate* lastaccel = 0;
    QAccelItem* lastitem = 0;

    QKeyEvent pe = *e;
    int n = -1;
    int hasShift = (e->state()&Qt::ShiftButton)?1:0;
    bool identicalDisabled = false;
    bool matchFound = false;
    qDebug("Search old system for shortcut");
    do {
        matchFound = false;
        for (int i = 0; i < accels.size(); ++i) {
            accel = accels.at(i);
            qDebug("Accel: %p", &accel);
            if (correctSubWindow(w, accel)) {
                qDebug("    correct subwindow");
                if (accel->enabled) {
                    for (int j = accel->aitems.size(); j > 0;) {
                        --j;
                        item = accel->aitems.at(j);
                        qDebug("        Item: %s", ((QString)item->key).latin1());
                        result = match(&pe, item, tocheck);
                        if (QKeySequence::ExactMatch == result) {
                            if (item->enabled) {
                                if (!firstaccel) {
                                    firstaccel = accel;
                                    firstitem = item;
                                }
                                lastaccel = accel;
                                lastitem = item;
                                n++;
                                matchFound = true;
                                if (n > qMax(clash,0))
                                    goto doclash;
                            } else {
                                identicalDisabled = true;
                            }
                        }
                        if (item->enabled && QKeySequence::PartialMatch == result) {
                            partial = tocheck;
                            matchFound = true;
                        }
                    }
                } else {
                    for (int j = accel->aitems.size(); j > 0;) {
                        --j;
                        item = accel->aitems.at(j);
                        if (QKeySequence::ExactMatch == match(&pe, item, tocheck))
                            identicalDisabled = true;
                    }
                }
            }
        }
        pe = QKeyEvent(QEvent::Accel, pe.key(), pe.state()&~Qt::ShiftButton, pe.text());
    } while (hasShift-- && !matchFound && !identicalDisabled);

#ifndef QT_NO_STATUSBAR
    mainStatusBar = (QStatusBar*) w->topLevelWidget()->child(0, "QStatusBar");
#endif
    if (n < 0) { // no match found
        currentState = partial.count() ? QKeySequence::PartialMatch : QKeySequence::NoMatch;
#ifndef QT_NO_STATUSBAR
        // Only display message if we are, or were, in a partial match
        if (mainStatusBar && (QKeySequence::PartialMatch == currentState || intermediate.count())) {
            if (currentState == QKeySequence::PartialMatch) {
                mainStatusBar->message((QString)partial + ", ...", 0);
            } else if (!identicalDisabled) {
                QString message = QAccel::tr("%1, %2 not defined").
                    arg((QString)intermediate).
                    arg(QKeySequence::encodeString(e->key() | translateModifiers(e->state())));
                mainStatusBar->message(message, 2000);
                // Since we're a NoMatch, reset the clash count
                clash = -1;
            } else {
                mainStatusBar->clear();
            }
        }
#endif

        bool eatKey = (QKeySequence::PartialMatch == currentState || intermediate.count());
        intermediate = partial;
        if (eatKey)
            e->accept();
        return eatKey;
    } else if (n == 0) { // found exactly one match
        clash = -1; // reset
#ifndef QT_NO_STATUSBAR
        if (currentState == QKeySequence::PartialMatch && mainStatusBar)
                mainStatusBar->clear();
#endif
        currentState = QKeySequence::NoMatch; // Free sequence keylock
        intermediate = QKeySequence();
        lastaccel->activate(lastitem);
        qDebug("activate(lastitem)");
        e->accept();
        return true;
    }

 doclash: // found more than one match
#ifndef QT_NO_STATUSBAR
    if (!mainStatusBar) // if "goto doclash", we need to get statusbar again.
        mainStatusBar = (QStatusBar*) w->topLevelWidget()->child(0, "QStatusBar");
#endif

    QString message = QAccel::tr("Ambiguous \"%1\" not handled").arg((QString)tocheck);
    if (clash >= 0 && n > clash) { // pick next  match
        intermediate = QKeySequence();
        currentState = QKeySequence::NoMatch; // Free sequence keylock
        clash++;
#ifndef QT_NO_STATUSBAR
        if (mainStatusBar &&
             !lastitem->signal &&
             !(lastaccel->parent->receivers(SIGNAL(activatedAmbiguously(int)))))
            mainStatusBar->message(message, 2000);
#endif
        lastaccel->activateAmbiguously(lastitem);
        qDebug("activateAmbiguously(lastitem)");
    } else { // start (or wrap) with the first matching
        intermediate = QKeySequence();
        currentState = QKeySequence::NoMatch; // Free sequence keylock
        clash = 0;
#ifndef QT_NO_STATUSBAR
        if (mainStatusBar &&
             !firstitem->signal &&
             !(firstaccel->parent->receivers(SIGNAL(activatedAmbiguously(int)))))
            mainStatusBar->message(message, 2000);
#endif
        firstaccel->activateAmbiguously(firstitem);
        qDebug("activateAmbiguously(firstitem)");
    }
    e->accept();
    return true;
}

QAccelPrivate::QAccelPrivate(QAccel *p)
    : parent(p)
{
    QAccelManager::self()->registerAccel(this);
    ignorewhatsthis = false;
    // Make sure that QApplications starts QAccel processing
}

QAccelPrivate::~QAccelPrivate()
{
    while (!aitems.isEmpty())
        delete aitems.takeFirst();
    QAccelManager::self()->unregisterAccel(this);
}

static QAccelItem *find_id(const QAccelList &list, int id)
{
    for (int i = 0; i < list.size(); ++i) {
        register QAccelItem *item = list.at(i);
        if (item->id == id)
            return item;
    }
    return 0;
}

static QAccelItem *find_key(const QAccelList &list, const QKeySequence &key)
{
    for (int i = 0; i < list.size(); ++i) {
        register QAccelItem *item = list.at(i);
        if (item->key == key)
            return item;
    }
    return 0;
}

/*!
    Constructs a QAccel object called \a name, with parent \a parent.
    The accelerator operates on \a parent.
*/

QAccel::QAccel(QWidget *parent, const char *name)
    : QObject(*new QAccelPrivate(this), parent)
{
    if (name)
        setObjectName(name);
    d->enabled = true;
    d->watch = parent;
    if (!d->watch)
        qWarning("QAccel: An accelerator must have a parent or a watch widget");
}

/*!
    Constructs a QAccel object called \a name, that operates on \a
    watch, and is a child of \a parent.

    This constructor is not needed for normal application programming.
*/
QAccel::QAccel(QWidget* watch, QObject *parent, const char *name)
    : QObject(*new QAccelPrivate(this), parent)
{
    if (name)
        setObjectName(name);
    d->enabled = true;
    d->watch = watch;
    if (!d->watch)
        qWarning("QAccel: An accelerator must have a parent or a watch widget");
}

/*!
    Destroys the accelerator object and frees all allocated resources.
*/

QAccel::~QAccel()
{
}


/*!
    \fn void QAccel::activated(int id)

    This signal is emitted when an accelerator key is pressed. \a id
    is a number that identifies this particular accelerator item.

    \sa activatedAmbiguously()
*/

/*!
    \fn void QAccel::activatedAmbiguously(int id)

    This signal is emitted when an accelerator key is pressed. \a id
    is a number that identifies this particular accelerator item.

    \sa activated()
*/


/*!
    Returns true if the accelerator is enabled; otherwise returns
    false.

    \sa setEnabled(), isItemEnabled()
*/

bool QAccel::isEnabled() const
{
    return d->enabled;
}


/*!
    Enables the accelerator if \a enable is true, or disables it if \a
    enable is false.

    Individual keys can also be enabled or disabled using
    setItemEnabled(). To work, a key must be an enabled item in an
    enabled QAccel.

    \sa isEnabled(), setItemEnabled()
*/

void QAccel::setEnabled(bool enable)
{
    d->enabled = enable;
}


/*!
    Returns the number of accelerator items in this accelerator.
*/

uint QAccel::count() const
{
    return d->aitems.count();
}


static int get_seq_id()
{
    static int seq_no = -2;  // -1 is used as return value in findKey()
    return seq_no--;
}

/*!
    Inserts an accelerator item and returns the item's identifier.

    \a key is a key code and an optional combination of Qt::SHIFT, Qt::CTRL
    and Qt::ALT. \a id is the accelerator item id.

    If \a id is negative, then the item will be assigned a unique
    negative identifier less than -1.

    \code
        QAccel *a = new QAccel(myWindow);           // create accels for myWindow
        a->insertItem(Qt::CTRL + Qt::Key_P, 200);           // Ctrl+P, e.g. to print document
        a->insertItem(Qt::ALT + Qt::Key_X, 201);           // Alt+X, e.g. to quit
        a->insertItem('q', 202);                               // 'q', e.g. to quit
        a->insertItem(0x3b1, 203);                               // greek letter alpha
        a->insertItem(Qt::Key_D);                           // gets a unique negative id < -1
        a->insertItem(Qt::CTRL + Qt::SHIFT + Qt::Key_P);           // gets a unique negative id < -1
    \endcode
*/

int QAccel::insertItem(const QKeySequence& key, int id)
{
    if (id == -1)
        id = get_seq_id();
    d->aitems.insert(0, new QAccelItem(key,id));
    return id;
}

/*!
    Removes the accelerator item with the identifier \a id.
*/

void QAccel::removeItem(int id)
{
    for (int i = 0; i < d->aitems.size(); ++i) {
        register QAccelItem *item = d->aitems.at(i);
        if (item->id == id) {
            delete d->aitems.takeAt(i);
            return;
        }
    }
}


/*!
    Removes all accelerator items.
*/

void QAccel::clear()
{
    while (!d->aitems.isEmpty())
        delete d->aitems.takeFirst();
}


/*!
    Returns the key sequence of the accelerator item with identifier
    \a id, or an invalid key sequence (0) if the id cannot be found.
*/

QKeySequence QAccel::key(int id)
{
    QAccelItem *item = find_id(d->aitems, id);
    return item ? item->key : QKeySequence(0);
}


/*!
    Returns the identifier of the accelerator item with the key code
    \a key, or -1 if the item cannot be found.
*/

int QAccel::findKey(const QKeySequence& key) const
{
    QAccelItem *item = find_key(d->aitems, key);
    return item ? item->id : -1;
}


/*!
    Returns true if the accelerator item with the identifier \a id is
    enabled. Returns false if the item is disabled or cannot be found.

    \sa setItemEnabled(), isEnabled()
*/

bool QAccel::isItemEnabled(int id) const
{
    QAccelItem *item = find_id(d->aitems, id);
    return item ? item->enabled : false;
}


/*!
    Enables the accelerator item with the identifier \a id if \a
    enable is true, and disables item \a id if \a enable is false.

    To work, an item must be enabled and be in an enabled QAccel.

    \sa isItemEnabled(), isEnabled()
*/

void QAccel::setItemEnabled(int id, bool enable)
{
    QAccelItem *item = find_id(d->aitems, id);
    if (item)
        item->enabled = enable;
}


/*!
    Connects the accelerator item \a id to the slot \a member of \a
    receiver.

    \code
        a->connectItem(201, mainView, SLOT(quit()));
    \endcode

    Of course, you can also set a signal as \a member.

    Normally accelerators are connected to slots which then receive
    the \c activated(int id) signal with the id of the accelerator
    item that was activated. If you choose to connect a specific
    accelerator item using this function, the \c activated() signal is
    emitted if the associated key sequence is pressed but no \c
    activated(int id) signal is emitted.

    \sa disconnectItem()
*/

bool QAccel::connectItem(int id, const QObject *receiver, const char *member)
{
    QAccelItem *item = find_id(d->aitems, id);
    if (item) {
        if (!item->signal) {
            item->signal = new QSignalEmitter;
        }
        return item->signal->connect(receiver, member);
    }
    return false;
}

/*!
    Disconnects an accelerator item with id \a id from the function
    called \a member in the \a receiver object.

    \sa connectItem()
*/

bool QAccel::disconnectItem(int id, const QObject *receiver,
                             const char *member)
{
    QAccelItem *item = find_id(d->aitems, id);
    if (item && item->signal)
        return item->signal->disconnect(receiver, member);
    return false;
}

void QAccelPrivate::activate(QAccelItem* item)
{
#ifndef QT_NO_WHATSTHIS
    if (QWhatsThis::inWhatsThisMode() && !ignorewhatsthis) {
        QWhatsThis::showText(QCursor::pos(), item->whatsthis);
        return;
    }
#endif
    qDebug("QAccelPrivate::activate(item)");
    if (item->signal)
        item->signal->activate();
    else
        emit parent->activated(item->id);
}

void QAccelPrivate::activateAmbiguously(QAccelItem* item)
{
    qDebug("QAccelPrivate::activateAmbiguously(item)");
    if (item->signal)
        item->signal->activate();
    else
        emit parent->activatedAmbiguously(item->id);
}


/*!
    Returns the shortcut key sequence for \a str, or an invalid key
    sequence (0) if \a str has no shortcut sequence.

    For example, shortcutKey("E&amp;xit") returns Qt::ALT+Qt::Key_X,
    shortcutKey("&amp;Quit") returns ALT+Key_Q and shortcutKey("Quit")
    returns 0. (In code that does not inherit the Qt namespace class,
    you must write e.g. Qt::ALT+Qt::Key_Q.)

    We provide a \link accelerators.html list of common accelerators
    \endlink in English. At the time of writing, Microsoft and Open
    Group do not appear to have issued equivalent recommendations for
    other languages.
*/

QKeySequence QAccel::shortcutKey(const QString &str)
{
    if(qt_accel_no_shortcuts)
        return QKeySequence();
    return QKeySequence::mnemonic(str);
}

/*!
    Sets a What's This help text for the accelerator item \a id to \a
    text.

    The text will be shown when the application is in What's This mode
    and the user hits the accelerator key.

    To set What's This help on a menu item (with or without an
    accelerator key), use QMenuData::setWhatsThis().

    \sa whatsThis(), QWhatsThis::inWhatsThisMode(), QAction::setWhatsThis()
*/
void QAccel::setWhatsThis(int id, const QString& text)
{

    QAccelItem *item = find_id(d->aitems, id);
    if (item)
        item->whatsthis = text;
}

/*!
    Returns the What's This help text for the specified item \a id or
    QString::null if no text has been specified.

    \sa setWhatsThis()
*/
QString QAccel::whatsThis(int id) const
{

    QAccelItem *item = find_id(d->aitems, id);
    return item? item->whatsthis : QString();
}

/*!\internal */
void QAccel::setIgnoreWhatsThis(bool b)
{
    d->ignorewhatsthis = b;
}

/*!\internal */
bool QAccel::ignoreWhatsThis() const
{
    return d->ignorewhatsthis;
}


/*!

\page accelerators.html

\title Standard Accelerator Keys

Applications invariably need to define accelerator keys for actions.
Qt fully supports accelerators, for example with \l QAccel::shortcutKey().

Here are Microsoft's recommendations for accelerator keys, with
comments about the Open Group's recommendations where they exist
and differ. For most commands, the Open Group either has no advice or
agrees with Microsoft.

The emboldened letter plus Alt is Microsoft's recommended choice, and
we recommend supporting it. For an Apply button, for example, we
recommend QButton::setText(\link QWidget::tr() tr \endlink("&amp;Apply"));

If you have conflicting commands (e.g. About and Apply buttons in the
same dialog), you must decide for yourself.

\list
\i <b><u>A</u></b>bout
\i Always on <b><u>T</u></b>op
\i <b><u>A</u></b>pply
\i <b><u>B</u></b>ack
\i <b><u>B</u></b>rowse
\i <b><u>C</u></b>lose (CDE: Alt+F4; Alt+F4 is "close window" in Windows)
\i <b><u>C</u></b>opy (CDE: Ctrl+C, Ctrl+Insert)
\i <b><u>C</u></b>opy Here
\i Create <b><u>S</u></b>hortcut
\i Create <b><u>S</u></b>hortcut Here
\i Cu<b><u>t</u></b>
\i <b><u>D</u></b>elete
\i <b><u>E</u></b>dit
\i <b><u>E</u></b>xit (CDE: E<b><u>x</u></b>it)
\i <b><u>E</u></b>xplore
\i <b><u>F</u></b>ile
\i <b><u>F</u></b>ind
\i <b><u>H</u></b>elp
\i Help <b><u>T</u></b>opics
\i <b><u>H</u></b>ide
\i <b><u>I</u></b>nsert
\i Insert <b><u>O</u></b>bject
\i <b><u>L</u></b>ink Here
\i Ma<b><u>x</u></b>imize
\i Mi<b><u>n</u></b>imize
\i <b><u>M</u></b>ove
\i <b><u>M</u></b>ove Here
\i <b><u>N</u></b>ew
\i <b><u>N</u></b>ext
\i <b><u>N</u></b>o
\i <b><u>O</u></b>pen
\i Open <b><u>W</u></b>ith
\i Page Set<b><u>u</u></b>p
\i <b><u>P</u></b>aste
\i Paste <b><u>L</u></b>ink
\i Paste <b><u>S</u></b>hortcut
\i Paste <b><u>S</u></b>pecial
\i <b><u>P</u></b>ause
\i <b><u>P</u></b>lay
\i <b><u>P</u></b>rint
\i <b><u>P</u></b>rint Here
\i P<b><u>r</u></b>operties
\i <b><u>Q</u></b>uick View
\i <b><u>R</u></b>edo (CDE: Ctrl+Y, Shift+Alt+Backspace)
\i <b><u>R</u></b>epeat
\i <b><u>R</u></b>estore
\i <b><u>R</u></b>esume
\i <b><u>R</u></b>etry
\i <b><u>R</u></b>un
\i <b><u>S</u></b>ave
\i Save <b><u>A</u></b>s
\i Select <b><u>A</u></b>ll
\i Se<b><u>n</u></b>d To
\i <b><u>S</u></b>how
\i <b><u>S</u></b>ize
\i S<b><u>p</u></b>lit
\i <b><u>S</u></b>top
\i <b><u>U</u></b>ndo (CDE: Ctrl+Z or Alt+Backspace)
\i <b><u>V</u></b>iew
\i <b><u>W</u></b>hat's This?
\i <b><u>W</u></b>indow
\i <b><u>Y</u></b>es
\endlist

There are also a lot of other keys and actions (that use other
modifier keys than Alt). See the Microsoft and The Open Group
documentation for details.

The \link http://www.amazon.com/exec/obidos/ASIN/0735605661/trolltech/t
Microsoft book \endlink has ISBN 0735605661. The corresponding Open Group
book is very hard to find, rather expensive and we cannot recommend
it. However, if you really want it, OGPubs@opengroup.org might be able
to help. Ask them for ISBN 1859121047.

*/

#endif // QT_NO_ACCEL
