#include "qshortcut.h"
#include "private/qobject_p.h"
#include "qwhatsthis.h"

#define d d_func()
#define p p_func()

/*!
    \class QShortcut qshortcut.h
    \brief The QShortcut class is blah blah blah..

    More blah blah blah...
*/

/*!
    \internal
    Private data accessed through d-pointer.
*/
class QShortcutPrivate : public QObjectPrivate
{
public:
    QKeySequence sc_sequence;
    bool sc_enabled;
    int sc_id;
    QString sc_whatsthis;
};

/*!
    Constructs a QShortcut object called \a name, with parent
    \a parent. Since no shortcut key is specified, the
    shortcut will not emit any signals.
*/
QShortcut::QShortcut(QWidget *parent, const char *name)
    : QObject(*new QShortcutPrivate, parent)
{
    Q_ASSERT(parent != 0);
    init(parent, name);
}

/*!
    Constructs a QShortcut object called \a name, with parent
    \a parent. The object will operate on the parent, and watch
    for QShortcutEvents that match the key sequence \a key.
    Depending on the ambiguity of this event, the shortcut will
    call the \a member function, or the \a ambiguousMember
    function.
*/
QShortcut::QShortcut(const QKeySequence &key, QWidget *parent,
                     const char *member, const char *ambiguousMember,
                     const char *name)
    : QObject(*new QShortcutPrivate, parent)
{
    Q_ASSERT(parent != 0);
    init(parent, name);
    d->sc_sequence = key;
    d->sc_id = parent->grabShortcut(key);
    connect(this, SIGNAL(activated()), parent, member);
    connect(this, SIGNAL(activatedAmbiguously()), parent, ambiguousMember);
}

/*!
    \internal
*/
QShortcut::QShortcut(QShortcutPrivate &dd, QWidget *parent)
    : QObject(dd, parent)
{
}

/*!
    Destroys the shortcut.
*/
QShortcut::~QShortcut()
{
    QWidget *parent = parentWidget();
    parent->releaseShortcut(d->sc_sequence, d->sc_id);
    parent->removeEventFilter(this);
}

/*!
    \internal
    Initializes the object, and hooks into the eventloop of
    the parent.
*/
void QShortcut::init(QWidget *parent, const char *name)
{
    setObjectName(name);
    parent->installEventFilter(this);
}

/*!
    \property QShortcut::key
    \brief Sets the shortcut key to \key.

    \a key is a key sequence with an optional combination of SHIFT,
    CTRL and ALT.

    \code
        setKey(Key_D);              // gets a unique negative id < -1
        setKey(CTRL + Key_P);       // Ctrl+P, e.g. to print document
        setKey(0x3b1);              // greek letter alpha
        setKey(0);                  // no signal emitet
        setKey('q');                // 'q', e.g. to quit
        setKey("Ctrl+P");           // Ctrl+P, e.g. to print document
        setKey(QKeySequence());     // no signal emitet
    \endcode
    \sa key(), QKeySequence
*/
void QShortcut::setKey(const QKeySequence &key)
{
    QWidget *parent = parentWidget();
    parent->releaseShortcut(d->sc_sequence, d->sc_id);
    d->sc_sequence = key;
    d->sc_id = parent->grabShortcut(key);
}

/*!
    Returns the key sequence of the shortcut.
    \sq setKey(), QKeySequence
*/
QKeySequence QShortcut::key() const
{
    return d->sc_sequence;
}

/*!
    \property QShortcut::enabled
    \brief Whether the shortcut is enabled

    An enabled shortcut emits the activated() or activatedAmbiguously()
    signal on a QShortcutEvent matching its keysequence.

    If the application is in WhatsThis mode the shortcut will not emit
    the signals, but show the WhatsThis text.

    \sa isEnabled(), whatsThis()
*/
void QShortcut::setEnabled(bool enable)
{
    d->sc_enabled = enable;
    parentWidget()->enableShortcut(enable, d->sc_sequence, d->sc_id);
}

/*!
    Returns TRUE if this shortcut is enabled, an will emit the
    corresponding signals on a QShortcutEvent; otherwise
    returns FALSE.

    \sa setEnabled() enabled
*/
bool QShortcut::isEnabled() const
{
    return d->sc_enabled;
}

/*!
    \property QShortcut::whatsthis
    \brief Sets the What's This help text for the shortcut to \a text.

    The text will be shown when the application is in What's This mode
    and the user hits the shortcut.

    To set What's This help on a menu item (with or without an
    shortcut key), use QMenuData::setWhatsThis().

    \sa whatsThis(), QWhatsThis::inWhatsThisMode(),
    QMenuData::setWhatsThis(), QAction::setWhatsThis()
*/
void QShortcut::setWhatsThis(const QString &text)
{
    d->sc_whatsthis = text;
}

/*!
    Returns the What's This help text for the shortcut.

    \sa setWhatsThis()
*/
QString QShortcut::whatsThis() const
{
    return d->sc_whatsthis;
}

/*!
    Returns the id of the shortcut.
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
        QShortcutEvent *se = (QShortcutEvent*)e;
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
