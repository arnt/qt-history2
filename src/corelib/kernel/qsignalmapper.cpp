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

class QSignalMapperPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QSignalMapper)
public:
    void senderDestroyed() {
        Q_Q(QSignalMapper);
        q->removeMappings(q->sender());
    }
    QHash<QObject *, int> intHash;
    QHash<QObject *, QString> stringHash;
    QHash<QObject *, QWidget*> widgetHash;

};


/*!
    \class QSignalMapper
    \brief The QSignalMapper class bundles signals from identifiable senders.

    \ingroup io
    \mainclass

    This class collects a set of parameterless signals, and re-emits
    them with integer, string or widget parameters corresponding to
    the object that sent the signal.

    The class supports the mapping of particular strings or integers
    with particular objects using setMapping(). The objects' signals
    can then be connected to the map() slot which will emit the
    mapped() signal with the string or integer associated with the
    original signalling object. Mappings can be removed later using
    removeMappings().

    Example: Suppose we want to create a custom widget that contains
    a group of buttons (like a tool palette). One approach is to
    connect each button's clicked() signal to its own custom slot;
    but in this example we want to connect all the buttons to a
    single slot and parameterize the slot by the button that was
    clicked.

    Here's the definition of a simple custom widget that has a single
    signal, clicked(), which is emitted with the caption of the button
    that was clicked:

    \quotefromfile snippets/qsignalmapper/buttonwidget.h
    \skipto QWidget
    \printuntil QSignalMapper
    \printuntil };

    The only function that we need to implement is the constructor:

    \quotefromfile snippets/qsignalmapper/buttonwidget.cpp
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

#ifdef QT3_SUPPORT
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
    Adds a mapping so that when map() is signalled from the given \a
    sender, the signal mapped(\a id) is emitted.

    There may be at most one integer ID for each object.

    \sa mapping()
*/
void QSignalMapper::setMapping(QObject *sender, int id)
{
    Q_D(QSignalMapper);
    d->intHash.insert(sender, id);
    connect(sender, SIGNAL(destroyed()), this, SLOT(senderDestroyed()));
}

/*!
    \overload

    Adds a mapping so that when map() is signalled from the given \a
    sender, the signal mapped(\a text ) is emitted.

    There may be at most one text for each object.
*/
void QSignalMapper::setMapping(QObject *sender, const QString &text)
{
    Q_D(QSignalMapper);
    d->stringHash.insert(sender, text);
    connect(sender, SIGNAL(destroyed()), this, SLOT(senderDestroyed()));
}

/*!
    \overload

    Adds a mapping so that when map() is signalled from the given \a
    sender, the signal mapped(\a widget ) is emitted.

    There may be at most one widget for each object.
*/
void QSignalMapper::setMapping(QObject *sender, QWidget *widget)
{
    Q_D(QSignalMapper);
    d->widgetHash.insert(sender, widget);
    connect(sender, SIGNAL(destroyed()), this, SLOT(senderDestroyed()));
}


/*!
    Returns the sender QObject that is associated with the given \a
    id.

    \sa setMapping()
*/
QObject *QSignalMapper::mapping(int id) const
{
    Q_D(const QSignalMapper);
    return d->intHash.key(id);
}

/*!
    \overload
*/
QObject *QSignalMapper::mapping(const QString &id) const
{
    Q_D(const QSignalMapper);
    return d->stringHash.key(id);
}

/*!
    \overload
*/
QObject *QSignalMapper::mapping(QWidget *widget) const
{
    Q_D(const QSignalMapper);
    return d->widgetHash.key(widget);
}

/*!
    Removes all mappings for \a sender.

    This is done automatically when mapped objects are destroyed.
*/
void QSignalMapper::removeMappings(QObject *sender)
{
    Q_D(QSignalMapper);

    d->intHash.remove(sender);
    d->stringHash.remove(sender);
    d->widgetHash.remove(sender);
}

/*!
    This slot emits signals based on which object sends signals to it.
*/
void QSignalMapper::map() { map(sender()); }

/*!
    This slot emits signals based on the \a sender object.
*/
void QSignalMapper::map(QObject *sender)
{
    Q_D(QSignalMapper);
    if (d->intHash.contains(sender))
        emit mapped(d->intHash.value(sender));
    if (d->stringHash.contains(sender))
        emit mapped(d->stringHash.value(sender));
    if (d->widgetHash.contains(sender))
        emit mapped(d->widgetHash.value(sender));
}


/*!
    \fn void QSignalMapper::mapped(int i)

    This signal is emitted when map() is signalled from an object that
    has an integer mapping set. The object's mapped integer is passed
    in \a i.

    \sa setMapping()
*/

/*!
    \fn void QSignalMapper::mapped(const QString &text)
    \overload

    This signal is emitted when map() is signalled from an object that
    has a string mapping set. The object's mapped string is passed in
    \a text.

    \sa setMapping()
*/

/*!
    \fn void QSignalMapper::mapped(QWidget *widget)
    \overload

    This signal is emitted when map() is signalled from an object that
    has a widget mapping set. The object's mapped widget is passed in
    \a widget.

    \sa setMapping()
*/

#include "moc_qsignalmapper.cpp"
#endif // QT_NO_SIGNALMAPPER
