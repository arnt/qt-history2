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

#include "qshortcutmap_p.h"
#include "private/qobject_p.h"
#include "qkeysequence.h"
#include "qdebug.h"
#include "qevent.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qvector.h"
#include "qmenu.h"
#include "qshortcut.h"
#include "qapplication_p.h"
#include <private/qaction_p.h>

// To enable verbose output uncomment below
//#define Debug_QShortcutMap

/* \internal
    Entry data for QShortcutMap
    Contains:
        Keysequence for entry
        Pointer to parent owning the sequence
*/
struct QShortcutEntry
{
    QShortcutEntry()
        : keyseq(0), context(Qt::WindowShortcut), enabled(false), id(0), owner(0)
    {}

    QShortcutEntry(QObject *o, const QKeySequence &k, Qt::ShortcutContext c, int i)
        : keyseq(k), context(c), enabled(true), id(i), owner(o)
    {}

    bool operator<(const QShortcutEntry &f) const
    { return keyseq < f.keyseq; }

    QKeySequence keyseq;
    Qt::ShortcutContext context;
    bool enabled : 1;
    signed int id : 31;
    QObject *owner;
};

#ifndef QT_NO_DEBUG
/*! \internal
    QDebug operator<< for easy debug output of the shortcut entries.
*/
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
int QShortcutMap::addShortcut(QObject *owner, const QKeySequence &key, Qt::ShortcutContext context)
{
    Q_ASSERT_X(owner, "QShortcutMap::addShortcut", "All shortcuts need an owner");
    Q_ASSERT_X(!key.isEmpty(), "QShortcutMap::addShortcut", "Cannot add keyless shortcuts to map");
    Q_D(QShortcutMap);
    
    QShortcutEntry newEntry(owner, key, context, --(d->currentId));
    QList<QShortcutEntry>::iterator it = qUpperBound(d->sequences.begin(), d->sequences.end(), newEntry);
    d->sequences.insert(it, newEntry); // Insert sorted
#if defined(Debug_QShortcutMap)
    qDebug().nospace()
        << "QShortcutMap::addShortcut(" << owner << ", "
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

int QShortcutMap::removeShortcut(int id, QObject *owner, const QKeySequence &key)
{
    Q_D(QShortcutMap);
    int itemsRemoved = 0;
    bool allOwners = (owner == 0);
    bool allKeys = key.isEmpty();
    bool allIds = id == 0;

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
        int entryId = entry.id;
        if ((allOwners || entry.owner == owner)
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
        << "QShortcutMap::removeShortcut(" << id << ", " << owner << ", "
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
int QShortcutMap::setShortcutEnabled(bool enable, int id, QObject *owner, const QKeySequence &key)
{
    Q_D(QShortcutMap);
    int itemsChanged = 0;
    bool allOwners = (owner == 0);
    bool allKeys = key.isEmpty();
    bool allIds = id == 0;

    int i = d->sequences.size()-1;
    while (i>=0)
    {
        QShortcutEntry entry = d->sequences.at(i);
        if ((allOwners || entry.owner == owner)
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
        << owner << ", " << key << ") = " << itemsChanged;
#endif
    return itemsChanged;
}


/*! \internal
    Resets the state of the statemachine to NoMatch
*/
void QShortcutMap::resetState()
{
    Q_D(QShortcutMap);
    d->currentState = QKeySequence::NoMatch;
    clearSequence(d->currentSequence);
}

/*! \internal
    Returns the current state of the statemachine
*/
QKeySequence::SequenceMatch QShortcutMap::state()
{
    Q_D(QShortcutMap);
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
    Q_D(QShortcutMap);
    bool wasAccepted = e->isAccepted();
    if (d->currentState == QKeySequence::NoMatch) {
        ushort orgType = e->t;
        e->t = QEvent::ShortcutOverride;
        e->ignore();
        QApplication::sendSpontaneousEvent(w, e);
        e->t = orgType;
        if (e->isAccepted()) {
            if (!wasAccepted)
                e->ignore();
            return false;
        }
    }

    QKeySequence::SequenceMatch result = nextState(e);
    bool stateWasAccepted = e->isAccepted();
    if (wasAccepted)
        e->accept();
    else
        e->ignore();

    switch(result) {
    case QKeySequence::NoMatch:
        return stateWasAccepted;
    case QKeySequence::ExactMatch:
        resetState();
        dispatchEvent();
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
    Q_D(QShortcutMap);
    // Modifiers can NOT be shortcuts...
    if (e->key() >= Qt::Key_Shift &&
        e->key() <= Qt::Key_Alt)
        return d->currentState;

    QKeySequence::SequenceMatch result = QKeySequence::NoMatch;

    // We start fresh each time..
    d->identicals.resize(0);

    result = find(e);
    if (result == QKeySequence::NoMatch && e->modifiers() & Qt::ShiftModifier) {
        // If Shift + Key_Backtab, also try Shift + Qt::Key_Tab
        if (e->key() == Qt::Key_Backtab) {
            QKeyEvent pe = QKeyEvent(e->type(), Qt::Key_Tab, e->modifiers(), e->text());
            result = find(&pe);
        }
        // If still no result, try removing the Shift modifier
        if (result == QKeySequence::NoMatch) {
            QKeyEvent pe = QKeyEvent(e->type(), e->key(),
                                     e->modifiers()&~Qt::ShiftModifier, e->text());
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
    Q_D(QShortcutMap);
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

    QList<QShortcutEntry>::ConstIterator itEnd = d->sequences.constEnd();
    QList<QShortcutEntry>::ConstIterator it =
        qLowerBound(d->sequences.constBegin(), itEnd, newEntry);

    bool partialFound = false;
    bool identicalDisabledFound = false;
    QKeySequence::SequenceMatch result = QKeySequence::NoMatch;
    do {
        if (it == itEnd)
            break;
        result = newEntry.keyseq.matches((*it).keyseq);
        if (result != QKeySequence::NoMatch && correctContext(*it)) {
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
    Q_D(QShortcutMap);
    seq.setKey(d->currentSequence[0], 0);
    seq.setKey(d->currentSequence[1], 1);
    seq.setKey(d->currentSequence[2], 2);
    seq.setKey(d->currentSequence[3], 3);
    int index = d->currentSequence.count();
    int modifier = translateModifiers(e->modifiers());

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
    Q_ASSERT_X(item.owner, "QShortcutMap", "Shortcut has no owner. Illegal map state!");

    QWidget *active_window = qApp->activeWindow();

    // popups do not become the active window,
    // so we fake it here to get the correct context
    // for the shortcut system.
    if (qApp->activePopupWidget())
        active_window = qApp->activePopupWidget();
    
    if (!active_window)
        return false;
 
    if (QAction *a = qobject_cast<QAction *>(item.owner))
        return correctContext(item.context, a, active_window);

    QWidget *w = qobject_cast<QWidget *>(item.owner);
    if (!w) {
        QShortcut *s = qobject_cast<QShortcut *>(item.owner);
        w = s->parentWidget();
    }
    return correctContext(item.context, w, active_window);
}

bool QShortcutMap::correctContext(Qt::ShortcutContext context, QWidget *w, QWidget *active_window)
{
    if (!w->isVisible() || !w->isEnabled())
        return false;

    if (context == Qt::ApplicationShortcut)
        return QApplicationPrivate::tryModalHelper(w, 0); // true, unless w is shadowed by a modal dialog

    if (context == Qt::WidgetShortcut)
        return w == QApplication::focusWidget();

    QWidget *tlw = w->window();

    /* if a floating tool window is active, keep shortcuts on the
     * parent working */
    if (active_window != tlw && active_window && active_window->windowType() == Qt::Tool && active_window->parentWidget()) {
        active_window = active_window->parentWidget()->window();
    }

    if (active_window  != tlw)
        return false;

    /* if we live in a MDI subwindow, ignore the event if we are
       not the active document window */
    const QWidget* sw = w;
    while (sw && !(sw->windowType() == Qt::SubWindow) && !sw->isWindow())
        sw = sw->parentWidget();
    if (sw && (sw->windowType() == Qt::SubWindow)) {
        QWidget *focus_widget = QApplication::focusWidget();
        while (focus_widget && focus_widget != sw)
            focus_widget = focus_widget->parentWidget();
        return sw == focus_widget;
    }

#if defined(Debug_QShortcutMap)
    qDebug().nospace() << "..true [Pass-through]";
#endif
    return true;
}


bool QShortcutMap::correctContext(Qt::ShortcutContext context, QAction *a, QWidget *active_window)
{
    const QList<QWidget *> &widgets = a->d_func()->widgets;
    for (int i = 0; i < widgets.size(); ++i) {
        QWidget *w = widgets.at(i);
        if (QMenu *menu = qobject_cast<QMenu *>(w)) {
            QAction *a = menu->menuAction();
            if (correctContext(context, a, active_window))
                return true;
        } else {
            if (correctContext(context, w, active_window))
                return true;
        }
    }
    QWidget *parent = a->parentWidget();
    if (parent && !widgets.contains(parent)) {
        if (QMenu *menu = qobject_cast<QMenu *>(parent)) {
            QAction *a = menu->menuAction();
            if (correctContext(context, a, active_window))
                return true;
        } else {
            if (correctContext(context, parent, active_window))
                return true;
        }
    }
    return false;
}


/*! \internal
    Converts keyboard button states into modifier states
*/
int QShortcutMap::translateModifiers(Qt::KeyboardModifiers modifiers)
{
    int result = 0;
    if (modifiers & Qt::ShiftModifier)
        result |= Qt::SHIFT;
    if (modifiers & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (modifiers & Qt::MetaModifier)
        result |= Qt::META;
    if (modifiers & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}

/*! \internal
    Returns the vector of QShortcutEntry's matching the last Identical state.
*/
QVector<const QShortcutEntry*> QShortcutMap::matches() const
{
    Q_D(const QShortcutMap);
    return d->identicals;
}

/*! \internal
    Dispatches QShortcutEvents to widgets who grabbed the matched key sequence.
*/
void QShortcutMap::dispatchEvent()
{
    Q_D(QShortcutMap);
    if (!d->identicals.size())
        return;

    const QKeySequence &curKey = d->identicals.at(0)->keyseq;
    if (d->prevSequence != curKey) {
        d->ambigCount = 0;
        d->prevSequence = curKey;
    }
    // Find next
    const QShortcutEntry *current = 0, *next = 0;
    int i = 0, enabledShortcuts = 0;
    while(i < d->identicals.size()) {
        current = d->identicals.at(i);
        if (current->enabled || !next){
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
        << (QString)curKey << "\", " << next->id << ", "
        << (bool)(enabledShortcuts>1) << ") to object(" << next->owner << ")";
#endif
    QShortcutEvent se(curKey, next->id, enabledShortcuts>1);
    QApplication::sendEvent(const_cast<QObject *>(next->owner), &se);
}

/* \internal
    QShortcutMap dump function, only available when Debug_QShortcutMap is
    defined.
*/
#if defined(Dump_QShortcutMap)
void QShortcutMap::dumpMap() const
{
    Q_D(const QShortcutMap);
    for (int i = 0; i < d->sequences.size(); ++i)
        qDebug().nospace() << &(d->sequences.at(i));
}
#endif
