#include "qshortcut.h"
#include "private/qobject_p.h"

#include <qevent.h>
#include <qwhatsthis.h>

#define d d_func()
#define p p_func()

/*!
    \class QShortcut qshortcut.h
    \brief The QShortcut class is used to create keyboard shortcuts.

    When the user types the \link QKeySequence key sequence\endlink
    for a given shortcut, the shortcut's activated() signal is
    emitted. (In the case of ambiguity, the activatedAmbiguously()
    signal is emitted.) A shortcut is "listened for" by Qt's event
    loop when the shortcut's parent widget is receiving events.

    A shortcut's key sequence can be set with setKey() and retrieved
    with key(). A shortcut can be enabled or disabled with
    setEnabled(), and can have What's This? help text set with
    setWhatsThis().
*/

/*
    \internal
    Private data accessed through d-pointer.
*/
class QShortcutPrivate : public QObjectPrivate
{
public:
    QShortcutPrivate() : sc_enabled(true), sc_id(0) {}
    QKeySequence sc_sequence;
    bool sc_enabled;
    int sc_id;
    QString sc_whatsthis;
};

/*!
    Constructs a QShortcut object with parent \a parent. Since no
    shortcut key sequence is specified, the shortcut will not emit any
    signals.

    \sa setKey()
*/
QShortcut::QShortcut(QWidget *parent)
    : QObject(*new QShortcutPrivate, parent)
{
    Q_ASSERT(parent != 0);
    parent->installEventFilter(this);
}

/*!
    Constructs a QShortcut object with parent \a parent. The shortcut
    operates on its parent, listening for \l{QShortcutEvent}s that
    match the key sequence \a key. Depending on the ambiguity of the
    event, the shortcut will call the \a member function, or the \a
    ambiguousMember function.
*/
QShortcut::QShortcut(const QKeySequence &key, QWidget *parent,
                     const char *member, const char *ambiguousMember)
    : QObject(*new QShortcutPrivate, parent)
{
    Q_ASSERT(parent != 0);
    parent->installEventFilter(this);
    d->sc_sequence = key;
    d->sc_id = parent->grabShortcut(key);
    connect(this, SIGNAL(activated()), parent, member);
    connect(this, SIGNAL(activatedAmbiguously()), parent, ambiguousMember);
}

/*!
    Destroys the shortcut.
*/
QShortcut::~QShortcut()
{
    QWidget *parent = parentWidget();
    parent->releaseShortcut(d->sc_id);
    parent->removeEventFilter(this);
}

/*!
    \property QShortcut::key
    \brief The shortcut's key sequence.

    This is a key sequence with an optional combination of Shift, Ctrl
    and Alt.

    \code
        setKey(0);                  // no signal emitted
        setKey(QKeySequence());     // no signal emitted
        setKey(0x3b1);              // Greek letter alpha
        setKey(Key_D);              // 'd', e.g. to delete
        setKey('q');                // 'q', e.g. to quit
        setKey(CTRL + Key_P);       // Ctrl+P, e.g. to print document
        setKey("Ctrl+P");           // Ctrl+P, e.g. to print document
    \endcode

    \sa key(), QKeySequence
*/
void QShortcut::setKey(const QKeySequence &key)
{
    QWidget *parent = parentWidget();
    parent->releaseShortcut(d->sc_id);
    d->sc_sequence = key;
    d->sc_id = parent->grabShortcut(key);
    if (!d->sc_enabled)
        parent->setShortcutEnabled(d->sc_id, false);
}

/*!
    Returns the shortcut's key sequence.

    \sa setKey(), QKeySequence
*/
QKeySequence QShortcut::key() const
{
    return d->sc_sequence;
}

/*!
    \property QShortcut::enabled
    \brief Whether the shortcut is enabled

    An enabled shortcut emits the activated() or
    activatedAmbiguously() signal when a QShortcutEvent occurs that
    matches the shortcut's key() sequence.

    If the application is in WhatsThis mode the shortcut will not emit
    the signals, but will show the Whats This? text instead.

    \sa isEnabled(), whatsThis()
*/
void QShortcut::setEnabled(bool enable)
{
    d->sc_enabled = enable;
    parentWidget()->setShortcutEnabled(d->sc_id, enable);
}

/*!
    Returns true if this shortcut is enabled, and will emit the
    corresponding signals on a QShortcutEvent (assuming that the key()
    sequence is not empty); otherwise returns false.

    \sa setEnabled() enabled
*/
bool QShortcut::isEnabled() const
{
    return d->sc_enabled;
}

/*!
    \property QShortcut::whatsthis
    \brief The shortcut's What's This? help text.

    The text will be shown this text when the application is in What's
    This? mode and the user types the shortcut key() sequence.

    To set What's This help on a menu item (with or without a
    shortcut key), use QMenuData::setWhatsThis().

    \sa whatsThis(), QWhatsThis::inWhatsThisMode(),
    QMenuData::setWhatsThis(), QAction::setWhatsThis()
*/
void QShortcut::setWhatsThis(const QString &text)
{
    d->sc_whatsthis = text;
}

/*!
    Returns the shortcut's What's This? help text.

    \sa setWhatsThis()
*/
QString QShortcut::whatsThis() const
{
    return d->sc_whatsthis;
}

/*!
    Returns the shortcut's id.
*/
int QShortcut::id() const
{
    return d->sc_id;
}

/*!
    \internal
*/
bool QShortcut::eventFilter(QObject *o, QEvent *e)
{
    bool handled = false;
    if (o == parent() && d->sc_enabled && e->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        if (se->shortcutId() == d->sc_id
            && se->key() == d->sc_sequence){
#ifndef QT_NO_WHATSTHIS
            if (QWhatsThis::inWhatsThisMode()) {
                QWhatsThis::showText(QCursor::pos(), d->sc_whatsthis);
                handled = true;
            } else
#endif
            if (se->isAmbiguous())
                emit activatedAmbiguously();
            else
                emit activated();
            handled = true;
        }
    }
    return handled;
}
