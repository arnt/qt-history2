/****************************************************************************
**
** Implementation of QSignalMapper class.
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

#include "qsignalmapper.h"
#ifndef QT_NO_SIGNALMAPPER
#include "qhash.h"
#include "qobject_p.h"
#define d d_func()
#define q q_func()

struct Rec {
    uint has_int:1;

    int int_id;
    QString str_id;
    // extendable to other types of identification
};


class QSignalMapperPrivate : public QObjectPrivate {
    Q_DECLARE_PUBLIC(QSignalMapper);
public:


    QHash<QObject *, Rec *> dict;
};

static Rec *getRec(QObject *that, QHash<QObject *, Rec *> &hash,
                                         QObject *sender)
{
    QHash<QObject *, Rec *>::Iterator it = hash.find(sender);
    if (it == hash.constEnd()) {
        Rec *rec = new Rec;
        rec->has_int = 0;

        hash.insert(sender, rec);
        QObject::connect(sender, SIGNAL(destroyed()), that, SLOT(removeMapping()));
        return rec;
    }
    return *it;
}


/*!
    \class QSignalMapper qsignalmapper.h
    \brief The QSignalMapper class bundles signals from identifiable senders.

    \ingroup io

    This class collects a set of parameterless signals, and re-emits
    them with integer or string parameters corresponding to the object
    that sent the signal.

    The class supports the mapping of particular strings or integers
    with particular objects using setMapping(). The objects' signals
    can then be connected to the map() slot which will emit the
    mapped() signal with the string or integer associated with the
    original signaling object. Mappings can be removed later using
    removeMappings().

    Example:

    Suppose we want to create a custom widget that contains a group of
    buttons (like a tool palette). One approach is to connect each
    button's clicked() signal to its own custom slot; but in this
    example we want to connect all the buttons to a single slot and
    parameterize the slot by the button that was clicked.

    Here's the definition of a simple custom widget that has a single
    signal, clicked(), which is emitted with the caption of the button
    that was clicked:

    \quotefile qsignalmapper/buttonwidget.h
    \skipto QWidget
    \printuntil QSignalMapper
    \printuntil };

    The only function that we need to implement is the constructor:

    \quotefile qsignalmapper/buttonwidget.cpp
    \skipto ButtonWidget
    \printuntil connect
    \printuntil connect
    \printuntil }

    A list of captions is passed to the constructor. A signal mapper
    is constructed and for each caption in the list a QPushButton is
    created. We connect each button's clicked() signal to the signal
    mapper's map() slot, and create a mapping in the signal mapper
    from each button to the text of its caption. Finally we connect
    the signal mapper's mapped() signal to the custom widget's
    clicked() signal. When the user clicks a button, the custom widget
    will emit a single clicked() signal whose argument is the text of
    the button the user clicked.

*/

/*!
    Constructs a QSignalMapper with parent \a parent.
*/
QSignalMapper::QSignalMapper(QObject* parent) :
    QObject(*new QSignalMapperPrivate, parent)
{
}

/*!
    \overload
    \obsolete
 */
QSignalMapper::QSignalMapper(QObject* parent, const char* name) :
    QObject(*new QSignalMapperPrivate, parent)
{
    if (name)
        setObjectName(name);
}

/*!
    Destroys the QSignalMapper.
*/
QSignalMapper::~QSignalMapper()
{
}

/*!
    Adds a mapping so that when map() is signaled from the given \a
    sender, the signal mapped(\a identifier) is emitted.

    There may be at most one integer identifier for each object.
*/
void QSignalMapper::setMapping(const QObject* sender, int identifier)
{
    Rec *rec = getRec(this, d->dict, const_cast<QObject *>(sender));
    rec->int_id = identifier;
    rec->has_int = 1;
}

/*!
    \overload

    Adds a mapping so that when map() is signaled from the given \a
    sender, the signal mapper(\a identifier) is emitted.

    There may be at most one string identifier for each object, and it
    may not be empty.
*/
void QSignalMapper::setMapping(const QObject* sender, const QString &identifier)
{
    Rec *rec = getRec(this, d->dict, const_cast<QObject *>(sender));
    rec->str_id = identifier;
}

/*!
    Removes all mappings for \a sender. This is done automatically
    when mapped objects are destroyed.
*/
void QSignalMapper::removeMappings(const QObject* sender)
{
    d->dict.remove(const_cast<QObject *>(sender));
}

void QSignalMapper::removeMapping()
{
    removeMappings(sender());
}

/*!
    This slot emits signals based on which object sends signals to it.
*/
void QSignalMapper::map()
{
    QHash<QObject *,Rec *>::Iterator it = d->dict.find(const_cast<QObject *>(sender()));
    if (it != d->dict.constEnd()) {
        Rec *rec = *it;
        if (rec->has_int)
            emit mapped(rec->int_id);
        if (!rec->str_id.isEmpty())
            emit mapped(rec->str_id);
    }
}

/*!
    \fn void QSignalMapper::mapped(int)

    This signal is emitted when map() is signaled from an object that
    has an integer mapping set.

    \sa setMapping()
*/

/*!
    \fn void QSignalMapper::mapped(const QString&)
    \overload

    This signal is emitted when map() is signaled from an object that
    has a string mapping set.

    \sa setMapping()
*/
#endif //QT_NO_SIGNALMAPPER
