/****************************************************************************
**
** Implementation of QSignalMapper class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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
    Q_DECL_PUBLIC(QSignalMapper);
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
*/

/*!
    Constructs a QSignalMapper with parent \a parent.
    Like all QObjects, it will be deleted when the parent is deleted.
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
    \overload void QSignalMapper::mapped(const QString&)

    This signal is emitted when map() is signaled from an object that
    has a string mapping set.

    \sa setMapping()
*/
#endif //QT_NO_SIGNALMAPPER
