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

#include "qsignalmapper.h"
#ifndef QT_NO_SIGNALMAPPER
#include "qhash.h"
#include "qobject_p.h"

struct Rec
{
    bool has_int : 1;
    bool has_str : 1;

    int int_id;
    QString str_id;
    // extensible to other types of identification

    inline Rec() : has_int(false), has_str(false) {}
};

class QSignalMapperPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QSignalMapper)
public:
    QHash<const QObject *, Rec> hash;
};

static Rec &getRec(QObject *signalMapper, QHash<const QObject *, Rec> &hash, const QObject *sender)
{
    QHash<const QObject *, Rec>::iterator i = hash.find(sender);
    if (i == hash.constEnd()) {
        i = hash.insert(sender, Rec());
        QObject::connect(sender, SIGNAL(destroyed()), signalMapper, SLOT(removeMapping()));
    }
    return *i;
}

/*!
    \class QSignalMapper
    \brief The QSignalMapper class bundles signals from identifiable senders.

    \ingroup io
    \mainclass

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
QSignalMapper::QSignalMapper(QObject* parent)
    : QObject(*new QSignalMapperPrivate, parent)
{
}

#ifdef QT_COMPAT
/*!
    \overload
    \obsolete
 */
QSignalMapper::QSignalMapper(QObject *parent, const char *name)
    : QObject(*new QSignalMapperPrivate, parent)
{
    setObjectName(QString::fromAscii(name));
}
#endif

/*!
    Destroys the QSignalMapper.
*/
QSignalMapper::~QSignalMapper()
{
}

/*!
    Adds a mapping so that when map() is signaled from the given \a
    sender, the signal mapped(\a id) is emitted.

    There may be at most one integer ID for each object.

    \sa mapping()
*/
void QSignalMapper::setMapping(const QObject *sender, int id)
{
    Q_D(QSignalMapper);

    Rec &rec = getRec(this, d->hash, sender);
    rec.has_int = true;
    rec.int_id = id;
}

/*!
    \overload

    Adds a mapping so that when map() is signaled from the given \a
    sender, the signal mapper(\a id) is emitted.

    There may be at most one string ID for each object, and it
    may not be empty.
*/
void QSignalMapper::setMapping(const QObject *sender, const QString &id)
{
    Q_D(QSignalMapper);

    Rec &rec = getRec(this, d->hash, sender);
    rec.has_str = true;
    rec.str_id = id;
}


/*!
    Returns the sender QObject that is associated with the given \a
    id.

    \sa setMapping()
*/
QObject *QSignalMapper::mapping(int id) const
{
    Q_D(const QSignalMapper);

    QHash<const QObject *, Rec>::const_iterator i = d->hash.begin();
    while (i != d->hash.end()) {
        if (i->has_int && i->int_id == id) {
	    // a const_cast would be best, but certain versions of aCC
	    // claim that it can not be used on volatile pointers.
            return (QObject *)i.key();
	}
        ++i;
    }
    return 0;
}

/*!
    \overload
*/
QObject *QSignalMapper::mapping(const QString &id) const
{
    Q_D(const QSignalMapper);

    QHash<const QObject *, Rec>::const_iterator i = d->hash.begin();
    while (i != d->hash.end()) {
        if (i->has_str && i->str_id == id) {
	    // a const_cast would be best, but certain versions of aCC
	    // claim that it can not be used on volatile pointers.
            return (QObject *)i.key();
	}
        ++i;
    }
    return 0;
}

/*!
    Removes all mappings for \a sender.

    This is done automatically when mapped objects are destroyed.
*/
void QSignalMapper::removeMappings(const QObject *sender)
{
    Q_D(QSignalMapper);

    d->hash.remove(sender);
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
    Q_D(QSignalMapper);

    QHash<const QObject *, Rec>::iterator i = d->hash.find(sender());
    if (i != d->hash.constEnd()) {
        if (i->has_int)
            emit mapped(i->int_id);
        if (i->has_str)
            emit mapped(i->str_id);
    }
}

/*!
    \fn void QSignalMapper::mapped(int i)

    This signal is emitted when map() is signaled from an object that
    has an integer mapping set. The object's mapped integer is passed
    in \a i.

    \sa setMapping()
*/

/*!
    \fn void QSignalMapper::mapped(const QString &text)
    \overload

    This signal is emitted when map() is signaled from an object that
    has a string mapping set. The object's mapped string is passed in
    \a text.

    \sa setMapping()
*/

#endif // QT_NO_SIGNALMAPPER
