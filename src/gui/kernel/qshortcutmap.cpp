#include "qshortcutmap_p.h"
#include "private/qobject_p.h"
#include "qkeysequence.h"
#include "qdebug.h"
#include "qevent.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qworkspace.h"
#include "qvector.h"
#include "qdockwindow.h"
#include "qmenu.h"

// To enable verbose output uncomment below
//#define Debug_QShortcutMap

#define d d_func()
#define p p_func()

/* \internal
    Entry data for QShortcutMap
    Contains:
        Keysequence for entry
        Pointer to parent owning the sequence
*/
struct QShortcutEntry
{
    QShortcutEntry()
        : keyseq(0), context(Qt::ShortcutOnActiveWindow), enabled(false), id(0), owner(0), monitor(0)
    {}

    QShortcutEntry(const QWidget *w, const QObject *m,
                   const QKeySequence &k, Qt::ShortcutContext c, int i)
        : keyseq(k), context(c), enabled(true), id(i), owner(w), monitor(m)
    {}

    bool operator<(const QShortcutEntry &f) const
    { return keyseq < f.keyseq; }

    QKeySequence keyseq;
    Qt::ShortcutContext context;
    bool enabled : 1;
    int id : 31;
    const QWidget *owner;
    const QObject *monitor;
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

/* \internal
    Private data for QShortcutMap
*/
class QShortcutMapPrivate
{
    Q_DECLARE_PUBLIC(QShortcutMap)

public:
    QShortcutMapPrivate(QShortcutMap* parent)
        : q_ptr(parent), currentId(0), ambigCount(0), currentState(QKeySequence::NoMatch)
    { identicals.reserve(10); }
    QShortcutMap *q_ptr;                        // Private's parent

    QList<QShortcutEntry> sequences;            // All sequences!

    int currentId;                              // Global shortcut ID number
    int ambigCount;                             // Index of last enabled ambiguous dispatch
    QKeySequence::SequenceMatch currentState;
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
int QShortcutMap::addShortcut(const QWidget *owner, const QObject *monitor,
                              const QKeySequence &key, Qt::ShortcutContext context)
{
    Q_ASSERT_X(owner, "QShortcutMap::addShortcut", "All shortcuts need an owner");
    Q_ASSERT_X(!key.isEmpty(), "QShortcutMap::addShortcut", "Cannot add keyless shortcuts to map");
    while (qt_cast<QMenu*>(owner))
        owner = owner->parentWidget();

    QShortcutEntry newEntry(owner, monitor, key, context, --(d->currentId));
    QList<QShortcutEntry>::iterator it
        = qUpperBound(d->sequences.begin(), d->sequences.end(), newEntry);
    d->sequences.insert(it, newEntry); // Insert sorted
#if defined(Debug_QShortcutMap)
    qDebug().nospace()
        << "QShortcutMap::addShortcut(" << owner << ", " << monitor << ", "
        << key << ", " << context << ") = " << d->currentId;
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

int QShortcutMap::removeShortcut(int id, const QWidget *owner, const QObject *monitor,
                                 const QKeySequence &key)
{
    int itemsRemoved = 0;
    bool allOwners = (owner == 0);
    bool allMonitors = (monitor == 0);
    bool allKeys = key.isEmpty();
    bool allIds = id == 0;

    // Special case, remove everything
    if (allOwners && allMonitors && allKeys && id == 0) {
        itemsRemoved = d->sequences.size();
        d->sequences.clear();
        return itemsRemoved;
    }

    while (qt_cast<QMenu*>(owner))
        owner = owner->parentWidget();

    int i = d->sequences.size()-1;
    while (i>=0)
    {
        const QShortcutEntry &entry = d->sequences.at(i);
        int entryId = entry.id;
        if ((allOwners || entry.owner == owner)
            && (allMonitors || entry.monitor == monitor )
            && (allIds || entry.id == id)
            && (allKeys || entry.keyseq == key)) {
            d->sequences.removeAt(i);
            ++itemsRemoved;
        }
        if (id == entryId)
            return itemsRemoved;
        --i;
    }
#if defined(Debug_QShortcutMap)
    qDebug().nospace()
        << "QShortcutMap::removeShortcut(" << id << ", " << owner << ", " << monitor << ", "
        << key << ") = " << itemsRemoved;
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
int QShortcutMap::setShortcutEnabled(bool enable, int id,
                                     const QWidget *owner, const QObject *monitor,
                                     const QKeySequence &key)
{
    int itemsChanged = 0;
    bool allOwners = (owner == 0);
    bool allMonitors = (monitor == 0);
    bool allKeys = key.isEmpty();
    bool allIds = id == 0;

    while (qt_cast<QMenu*>(owner))
        owner = owner->parentWidget();

    int i = d->sequences.size()-1;
    while (i>=0)
    {
        QShortcutEntry entry = d->sequences.at(i);
        if ((allOwners || entry.owner == owner)
            && (allMonitors || entry.monitor == monitor )
            && (allIds || entry.id == id)
            && (allKeys || entry.keyseq == key)) {
            d->sequences[i].enabled = enable;
            ++itemsChanged;
        }
        if (id == entry.id)
            return itemsChanged;
        --i;
    }
#if defined(Debug_QShortcutMap)
    qDebug().nospace()
        << "QShortcutMap::setShortcutEnabled(" << enable << ", " << id << ", "
        << owner << ", " << monitor << ", "
        << key << ") = " << itemsChanged;
#endif
    return itemsChanged;
}


/*! \internal
    Returns the id of the first shortcutentry matching the \a owner, \a monitor and \a key.
    Returns 0, if no matching shortcut entry.
*/
int QShortcutMap::changeMonitor(const QObject *monitor, const QKeySequence &key, bool enabled)
{
    Q_ASSERT_X(monitor != 0, "QShortcutMap::changeMonitorKey", "Must specify monitor");
    QList<QShortcutEntry> newEntries;

    int itemsModified = 0;
    int i = d->sequences.size() - 1;
    while (i>=0)
    {
        QShortcutEntry entry = d->sequences.at(i);
        if (entry.monitor == monitor) {
            if (entry.keyseq == key && entry.enabled == enabled) {
                Q_ASSERT_X(itemsModified == 0,
                           "QShortcutMap::changeMonitorKey",
                           "Invalid function end! Shortcuts removed, but not readded!");
                return 0;
            }
            entry.keyseq = key;
            entry.enabled = enabled;
            newEntries += entry;
            d->sequences.removeAt(i);
            ++itemsModified;
        }
        --i;
    }

    // Add back entries with new keysequence
    if (!key.isEmpty())
        for (int j = 0; j < newEntries.count(); ++j) {
            QShortcutEntry &newEntry = newEntries[j];
            QList<QShortcutEntry>::iterator it
                = qUpperBound(d->sequences.begin(), d->sequences.end(), newEntry);
            d->sequences.insert(it, newEntry); // Insert sorted
        }
#if defined(Debug_QShortcutMap)
    qDebug().nospace()
        << "QShortcutMap::changeMonitorKey(" << monitor << ", " << key << ") = " << itemsModified;
#endif
    return itemsModified;
}

/*! \internal
    Resets the state of the statemachine to NoMatch
*/
void QShortcutMap::resetState()
{
    d->currentState = QKeySequence::NoMatch;
    clearSequence(d->currentSequence);
}

/*! \internal
    Returns the current state of the statemachine
*/
QKeySequence::SequenceMatch QShortcutMap::state()
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
    if (d->currentState == QKeySequence::NoMatch) {
        ushort orgType = e->t;
        e->t = QEvent::ShortcutOverride;
        e->ignore();
        QApplication::sendSpontaneousEvent(w, e);
        e->t = orgType;
        if (e->isAccepted())
            return false;
    }
    QKeySequence::SequenceMatch result = nextState(e);
    switch(result) {
    case QKeySequence::NoMatch:
        return e->isAccepted();
    case QKeySequence::ExactMatch:
        dispatchEvent();
        resetState();
    default:
	break;
    }
    // If nextState is QKeySequence::ExactMatch && identicals.count == 0
    // we've only found disabled shortcuts
    return d->identicals.count() > 0 || result == QKeySequence::PartialMatch;
}

/*! \internal
    Returns the next state of the statemachine
    If return value is SequenceMatch::ExactMatch, then a call to matches()
    will return a QObjects* list of all matching objects for the last matching
    sequence.
*/
QKeySequence::SequenceMatch QShortcutMap::nextState(QKeyEvent *e)
{
    // Modifiers can NOT be shortcuts...
    if (e->key() >= Qt::Key_Shift &&
        e->key() <= Qt::Key_Alt)
        return d->currentState;

    QKeySequence::SequenceMatch result = QKeySequence::NoMatch;

    // We start fresh each time..
    d->identicals.resize(0);

    result = find(e);
    if (result == QKeySequence::NoMatch
        && e->state() & Qt::ShiftButton) {
        // If Shift + Key_BackTab, also try Shift + Qt::Key_Tab
        if (e->key() == Qt::Key_BackTab) {
            QKeyEvent pe = QKeyEvent(e->type(), Qt::Key_Tab, e->state(), e->text());
            result = find(&pe);
        }
        // If still no result, try removing the Shift modifier
        if (result == QKeySequence::NoMatch) {
            QKeyEvent pe = QKeyEvent(e->type(), e->key(), e->state()&~Qt::ShiftButton, e->text());
            result = find(&pe);
        }
    }

    // Should we eat this key press?
    if (d->currentState == QKeySequence::PartialMatch
        || (d->currentState == QKeySequence::ExactMatch && d->identicals.count()))
        e->accept();
    // Does the new state require us to clean up?
    if (result == QKeySequence::NoMatch)
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
QKeySequence::SequenceMatch QShortcutMap::find(QKeyEvent *e)
{
    if (!d->sequences.count())
        return QKeySequence::NoMatch;

    static QShortcutEntry newEntry;
    createNewSequence(e, newEntry.keyseq);

    // Should never happen
    if (newEntry.keyseq == d->currentSequence) {
        Q_ASSERT_X(e->key() != Qt::Key_unknown || e->text().length(),
                   "QShortcutMap::find", "New sequence to find identical to previous");
        return QKeySequence::NoMatch;
    }

    // Looking for new identicals, scrap old
    d->identicals.resize(0);

    QList<QShortcutEntry>::ConstIterator it =
        qLowerBound(d->sequences.constBegin(), d->sequences.constEnd(), newEntry);

    QList<QShortcutEntry>::ConstIterator itEnd = d->sequences.constEnd();
    bool partialFound = false;
    bool identicalDisabledFound = false;
    QKeySequence::SequenceMatch result = QKeySequence::NoMatch;
    do {
        if (it == itEnd)
            break;
        result = newEntry.keyseq.matches((*it).keyseq);
        if (correctContext(*it)) {
            if (result == QKeySequence::ExactMatch) {
                if ((*it).enabled)
                    d->identicals.append(&*it);
                else
                    identicalDisabledFound = true;
            } else if (result == QKeySequence::PartialMatch) {
                // We don't need partials, if we have identicals
                if (d->identicals.size())
                    break;
                // We only care about enabled partials, so we don't consume
                // key events when all partials are disabled!
                partialFound |= (*it).enabled;
            }
        }
        ++it;
    } while (result != QKeySequence::NoMatch);

    if (d->identicals.size()) {
        result = QKeySequence::ExactMatch;
    } else if (partialFound) {
        result = QKeySequence::PartialMatch;
    } else if (identicalDisabledFound) {
        result = QKeySequence::ExactMatch;
    } else {
        clearSequence(d->currentSequence);
        result = QKeySequence::NoMatch;
    }
    if (result != QKeySequence::NoMatch)
        d->currentSequence = newEntry.keyseq;
    return result;
}

/*! \internal
    Clears \a seq to an empty QKeySequence.
    Same as doing (the slower)
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
bool QShortcutMap::correctContext(const QShortcutEntry &item) {
    QWidget *wtlw = qApp->activeWindow();
    Q_ASSERT(wtlw != 0);

    const QWidget *w = item.owner;
    if (!w->isVisible() || !w->isEnabled() || !wtlw)
        return false;

    QWidget *tlw = w->topLevelWidget();
    wtlw = wtlw->topLevelWidget();

    switch (item.context) {
    case Qt::ShortcutOnActiveWindow:
        {
#ifndef QT_NO_MAINWINDOW
            /* if we live in a floating dock window, keep our parent's
             * shortcuts working */
            if (tlw->isDialog() && tlw->parentWidget() && ::qt_cast<QDockWindow*>(tlw))
                return tlw->parentWidget()->topLevelWidget() == wtlw;
#endif

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
        }
    case Qt::ShortcutOnApplication:
        return true;
    case Qt::ShortcutOnFocusWidget:
        return qApp->focusWidget() == item.owner;
    }
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
    if (!d->identicals.size()
        || d->currentState != QKeySequence::ExactMatch)
        return;

    if (d->prevSequence != d->currentSequence) {
        d->ambigCount = 0;
        d->prevSequence = d->currentSequence;
    }
    // Find next
    const QShortcutEntry *current = 0, *next = 0;
    int i = 0, enabledShortcuts = 0;
    while(i < d->identicals.size()) {
        current = d->identicals.at(i);
        if (current->enabled
            && (!next || current->monitor != next->monitor)){
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
    const QObject *sendTo = static_cast<const QObject*>(next->owner);
    if (next->monitor)
        sendTo = next->monitor;
#if defined(Debug_QShortcutMap)
    qDebug().nospace()
        << "QShortcutMap::dispatchEvent(): Sending QShortcutEvent(\""
        << (QString)d->currentSequence << "\", " << next->id << ", "
        << (bool)(enabledShortcuts>1) << ") to object(" << sendTo << ")";
#endif
    QShortcutEvent se(d->currentSequence, next->id, enabledShortcuts>1);
    QApplication::sendEvent(const_cast<QObject*>(sendTo), &se);
}

/* \internal
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
