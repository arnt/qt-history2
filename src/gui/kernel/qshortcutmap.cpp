#include "qshortcutmap_p.h"
#include "private/qobject_p.h"
#include "qkeysequence.h"
#include "qdebug.h"
#include "qevent.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qdockwindow.h"
#include "qworkspace.h"
#include "qvector.h"

// To enable verbose output uncomment below
//#define Debug_QShortcutMap

#if defined(Debug_QShortcutMap)
#pragma message("*** QShortcut debug mode")
#endif

#define d d_func()
#define p p_func()

/*! \internal
    Entry data for QShortcutMap
    Contains:
        Keysequence for entry
        Pointer to parent owning the sequence
*/
struct QShortcutEntry
{
    QShortcutEntry()
    { owner = 0; id = 0; enabled = false; }

    QShortcutEntry(const QWidget *w, const QKeySequence &k, int id)
    { owner = w; keyseq = k; this->id = id; enabled = true; }

    bool operator<(const QShortcutEntry &f) const
    { return keyseq < f.keyseq; }

    QKeySequence keyseq;
    const QWidget *owner;
    int id      : 31;
    bool enabled : 1;
};

/*! \internal
    QDebug operator<< for easy debug output of the shortcut entries.
*/
#ifndef QT_NO_DEBUG
QDebug &operator<<(QDebug &dbg, const QShortcutEntry *se) {
    if (!se)
        return dbg << "QShortcutEntry(0x0)";
    dbg.nospace()
        << "QShortcutEntry(" << se->keyseq
        << "), id(" << se->id << "), enabled(" << se->enabled << ") owner(" << se->owner << ")";
    return dbg.space();
}
#endif // QT_NO_DEBUG

/*! \internal
    Private data for QShortcutMap
*/
class QShortcutMapPrivate
{
    Q_DECLARE_PUBLIC(QShortcutMap);

public:
    QShortcutMapPrivate(QShortcutMap* parent)
        : q_ptr(parent), currentId(0), ambigCount(0), currentState(Qt::NoMatch)
    { identicals.reserve(10); }
    QShortcutMap *q_ptr;                        // Private's parent

    QList<QShortcutEntry> sequences;            // All sequences!

    int currentId;                              // Global shortcut ID number
    int ambigCount;                             // Index of last enabled ambiguous dispatch
    Qt::SequenceMatch currentState;
    QKeySequence currentSequence;               // Sequence for the current state
    QKeySequence prevSequence;                  // Sequence for the previous identical match
    QVector<const QShortcutEntry*> identicals;  // Last identical matches
};


/*! \internal
    QShortcutMap constructor.
*/
QShortcutMap::QShortcutMap()
{
    d_ptr = new QShortcutMapPrivate(this);
    Q_ASSERT(d_ptr != 0);
    resetState();
}

/*! \internal
    QShortcutMap destructor.
*/
QShortcutMap::~QShortcutMap()
{
    delete d_ptr;
    d_ptr = 0;
}

/*! \internal
    Adds a shortcut to the global map.
    Returns the id of the newly added shortcut.
*/
 int QShortcutMap::addShortcut(const QWidget *owner, const QKeySequence &key)
{
    Q_ASSERT(owner);
    Q_ASSERT(!key.isEmpty());

    QShortcutEntry newEntry(owner, key, --(d->currentId));
    QList<QShortcutEntry>::iterator it
        = qUpperBound(d->sequences.begin(), d->sequences.end(), newEntry);
    d->sequences.insert(it, newEntry); // Insert sorted
#if defined(Debug_QShortcutMap)
    qDebug().nospace()
        << "QShortcutMap::addShortcut(" << owner << ", " << key << ") = " << d->currentId;
#endif
    return d->currentId;
}

/*! \internal
    Removes a shortcut from the global map.
    If \a owner is 0, all entries in the map with the keysequence specified
    is removed. If \a key is null, all sequences for \a owner is removed from
    the map. If \a id is 0, any identical \a key sequences owned by \a owner
    are removed.
    Returns the number of sequences removed from the map.
*/
int QShortcutMap::removeShortcut(const QWidget *owner, int id, const QKeySequence &key)
{
    int itemsRemoved = 0;
    bool allOwners = (owner == 0);
    bool allKeys = key.isEmpty();

    // Special case, remove everything
    if (allOwners && allKeys && id == 0) {
        itemsRemoved = d->sequences.size();
        d->sequences.clear();
        return itemsRemoved;
    }

    int i = d->sequences.size()-1;
    while (i>=0)
    {
        const QShortcutEntry &entry = d->sequences.at(i);
        bool removeThis = allOwners ? true : entry.owner == owner;
        removeThis &= allKeys ? true : entry.keyseq == key;
        removeThis &= id == 0 ? true : entry.id == id;
        if (removeThis) {
            d->sequences.removeAt(i);
            ++itemsRemoved;
        }
        --i;
    }
#if defined(Debug_QShortcutMap)
    qDebug().nospace()
        << "QShortcutMap::removeShortcut(" << owner << ", " << key << ", "
        << id << ") = " << itemsRemoved;
#endif
    return itemsRemoved;
}

/*! \internal
    Changes the enable state of a shortcut to \a enable.
    If \a owner is 0, all entries in the map with the keysequence specified
    is removed. If \a key is null, all sequences for \a owner is removed from
    the map. If \a id is 0, any identical \a key sequences owned by \a owner
    are changed.
    Returns the number of sequences which are matched in the map.
*/
int QShortcutMap::setShortcutEnabled(const QWidget *owner, int id, bool enable,
                                     const QKeySequence &key)
{
    int itemsChanged = 0;
    bool allOwners = (owner == 0);
    bool allKeys = key.isEmpty();

    int i = d->sequences.size()-1;
    while (i>=0)
    {
        QShortcutEntry entry = d->sequences.at(i);
        bool removeThis = allOwners ? true : entry.owner == owner;
        removeThis &= allKeys ? true : entry.keyseq == key;
        removeThis &= id == 0 ? true : entry.id == id;
        if (removeThis) {
            d->sequences[i].enabled = enable;
            ++itemsChanged;
        }
        --i;
    }
#if defined(Debug_QShortcutMap)
    qDebug().nospace()
        << "QShortcutMap::setShortcutEnabled(" << enable << ", " << owner << ", "
        << key << ", " << id << ") = " << itemsChanged;
#endif
    return itemsChanged;
}

/*! \internal
    Resets the state of the statemachine to NoMatch
*/
void QShortcutMap::resetState()
{
    d->currentState = Qt::NoMatch;
    clearSequence(d->currentSequence);
    d->identicals.resize(0);
}

/*! \internal
    Returns the current state of the statemachine
*/
Qt::SequenceMatch QShortcutMap::state()
{
    return d->currentState;
}

/*! \internal
    Uses ShortcutOverride event to see if any widgets want to override
    the event. If not, uses nextState(QKeyEvent) to check for a grabbed
    Shortcut, and dispatchEvent() is found an identical.
    \sa nextState dispatchEvent
*/
bool QShortcutMap::tryShortcutEvent(QWidget *w, QKeyEvent *e)
{
    if (d->currentState == Qt::NoMatch) {
        ushort orgType = e->t;
        e->t = QEvent::ShortcutOverride;
        e->ignore();
        QApplication::sendSpontaneousEvent(w, e);
        e->t = orgType;
        if (e->isAccepted())
            return false;
    }
    switch(nextState(e)) {
    case Qt::NoMatch:
        return false;
    case Qt::Identical:
        dispatchEvent();
        resetState();
    default:
	break;
    }
    return true;
}

/*! \internal
    Returns the next state of the statemachine
    If return value is SequenceMatch::Identical, then a call to matches()
    will return a QObjects* list of all matching objects for the last matching
    sequence.
*/
Qt::SequenceMatch QShortcutMap::nextState(QKeyEvent *e)
{
    // Modifiers can NOT be accelerators...
    if (e->key() >= Qt::Key_Shift &&
        e->key() <= Qt::Key_Alt)
        return d->currentState;

    Qt::SequenceMatch result = Qt::NoMatch;

    // We start fresh each time..
    d->identicals.resize(0);

    result = find(e);
    if (result == Qt::NoMatch
        && e->state() & Qt::ShiftButton) {
        // If Shift + Key_BackTab, also try Shift + Key_Tab
        if (e->key() == Qt::Key_BackTab) {
            QKeyEvent pe = QKeyEvent(e->type(), Qt::Key_Tab, e->state(), e->text());
            result = find(&pe);
        }
        // If still no result, try removing the Shift modifier
        if (result == Qt::NoMatch) {
            QKeyEvent pe = QKeyEvent(e->type(), e->key(), e->state()&~Qt::ShiftButton, e->text());
            result = find(&pe);
        }
    }

    // Should we eat this key press?
    if (d->currentState != Qt::NoMatch)
        e->accept();
    // Does the new state require us to clean up?
    if (result == Qt::NoMatch)
        clearSequence(d->currentSequence);
    d->currentState = result;

#if defined(Debug_QShortcutMap)
    qDebug().nospace() << "QShortcutMap::nextState(" << e << ") = " << result;
#endif
    return result;
}

/*! \internal
    Returns the next state of the statemachine, based
    on the new key event \a e.
    Matches are appended to the vector of identicals,
    which can be access through matches().
    \sa matches
*/
Qt::SequenceMatch QShortcutMap::find(QKeyEvent *e)
{
    if (!d->sequences.count())
        return Qt::NoMatch;

    static QShortcutEntry newEntry;
    createNewSequence(e, newEntry.keyseq);

    // Should never happen, but will if e->key() == Key_unknown, and
    // e->text == "", in which case we want to break into debugger
    if (newEntry.keyseq == d->currentSequence) {
        Q_ASSERT(e->key() != Qt::Key_unknown || e->text().length());
        return Qt::NoMatch;
    }

    QList<QShortcutEntry>::ConstIterator it =
        qLowerBound(d->sequences.constBegin(), d->sequences.constEnd(), newEntry);

    QList<QShortcutEntry>::ConstIterator itEnd = d->sequences.constEnd();
    bool partialFound = false;
    Qt::SequenceMatch result = Qt::NoMatch;
    do {
        if (it == itEnd)
            break;
        result = newEntry.keyseq.matches((*it).keyseq);
        if (correctSubWindow((*it).owner)) {
            if (result == Qt::Identical) {
                d->identicals.append(&*it);
            } else if (result == Qt::PartialMatch) {
                if (d->identicals.size())
                    break;  // We don't need partials, if we have identicals
                partialFound = true;
            }
        }
        ++it;
    } while (result != Qt::NoMatch);

    if (d->identicals.size()) {
        result = Qt::Identical;
    } else if (partialFound) {
        result = Qt::PartialMatch;
    } else {
        clearSequence(d->currentSequence);
        result = Qt::NoMatch;
    }
    if (result != Qt::NoMatch)
        d->currentSequence = newEntry.keyseq;
    return result;
}

/*! \internal
    Clears \a seq to an empty QKeySequence.
    Same as doing
    \code
        key = QKeySequence();
    \endcode
*/
void QShortcutMap::clearSequence(QKeySequence &seq)
{
    seq.setKey(0, 0);
    seq.setKey(0, 1);
    seq.setKey(0, 2);
    seq.setKey(0, 3);
}

/*! \internal
    Alters \a seq to the new sequence state, based on the
    current sequence state, and the new key event \a e.
*/
void QShortcutMap::createNewSequence(QKeyEvent *e, QKeySequence &seq)
{
    seq.setKey(d->currentSequence[0], 0);
    seq.setKey(d->currentSequence[1], 1);
    seq.setKey(d->currentSequence[2], 2);
    seq.setKey(d->currentSequence[3], 3);
    int index = d->currentSequence.count();
    int modifier = translateModifiers(e->state());

    // Use the key code, if possible to prevent Ctrl+<Key> text problems,
    // else use unicode of text
    if (e->key() && e->key() != Qt::Key_unknown)
        seq.setKey(e->key() | modifier, index);
    else
        seq.setKey((int)e->text().unicode()->toUpper().unicode() | modifier, index);
}

/*! \internal
    Returns true if the widget \a w is a logical sub window of the current
    top-level widget.
*/
bool QShortcutMap::correctSubWindow(const QWidget* w) {
    QWidget *wtlw = qApp->activeWindow();
    Q_ASSERT(wtlw != 0);

    if (!w->isVisible() || !w->isEnabled() || !wtlw)
        return false;

    QWidget *tlw = w->topLevelWidget();
    wtlw = wtlw->topLevelWidget();

    /* if we live in a floating dock window, keep our parent's
     * accelerators working */
    if (tlw->isDialog() && tlw->parentWidget() && ::qt_cast<QDockWindow*>(tlw))
        return tlw->parentWidget()->topLevelWidget() == wtlw;

    if (wtlw  != tlw)
        return false;

#ifndef QT_NO_WORKSPACE
    /* if we live in a MDI subwindow, ignore the event if we are
       not the active document window */
    const QWidget* sw = w;
    while (sw && !sw->testWFlags(Qt::WSubWindow) && !sw->isTopLevel())
        sw = sw->parentWidget();
    if (sw && sw->testWFlags(Qt::WSubWindow)) {
        QWidget *actW = ::qt_cast<QWorkspace*>(sw->parentWidget())->activeWindow();
        // If workspace has no active window return false
        if (!actW)
            return false;
        // Return true, if shortcut belongs to widget
        // inside active workspace child window
        return sw == actW->parentWidget();
    }
#endif
    return true;
}

/*! \internal
    Converts keyboard button states into modifier states
*/
int QShortcutMap::translateModifiers(Qt::ButtonState state)
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

/*! \internal
    Returns the vector of QShortcutEntry's matching the last Identical state.
*/
QVector<const QShortcutEntry*> QShortcutMap::matches() const
{
    return d->identicals;
}

/*! \internal
    Dispatches QShortcutEvents to widgets who grabbed the matched key sequence.
*/
void QShortcutMap::dispatchEvent()
{
    if (d->currentState != Qt::Identical)
        return;

    Q_ASSERT(d->identicals.size());
    if (d->prevSequence != d->currentSequence) {
        d->ambigCount = 0;
        d->prevSequence = d->currentSequence;
    }
    // Find next
    const QShortcutEntry *current = 0, *next = 0;
    int i = 0, enabledShortcuts = 0;
    while(i < d->identicals.size()) {
        current = d->identicals.at(i);
        if (current->enabled){
            ++enabledShortcuts;
            if (enabledShortcuts > d->ambigCount + 1)
                break;
            next = current;
        }
        ++i;
    }
    d->ambigCount = (d->identicals.size() == i ? 0 : d->ambigCount + 1);
    if (!next)
        return;
    // Dispatch next enabled
#if defined(Debug_QShortcutMap)
    qDebug().nospace()
        << "QShortcutMap::dispatchEvent(): Sending QShortcutEvent(\""
        << (QString)d->currentSequence << "\", " << next->id << ", "
        << (bool)(enabledShortcuts>1) << ") to widget(" << next->owner << ")";
#endif
    QShortcutEvent se(d->currentSequence, next->id, enabledShortcuts>1);
    QApplication::sendEvent(const_cast<QWidget*>(next->owner), &se);
}

/*! \internal
    QShortcutMap dump function, only available when Debug_QShortcutMap is
    defined.
*/
#if defined(Dump_QShortcutMap)
void QShortcutMap::dumpMap() const
{
    for (int i = 0; i < d->sequences.size(); ++i)
        qDebug().nospace() << &(d->sequences.at(i));
}
#endif
