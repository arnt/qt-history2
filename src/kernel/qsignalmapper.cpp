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
#include "qptrdict.h"

struct QSignalMapperRec {
    QSignalMapperRec()
    {
	has_int = 0;
	str_id = QString::null;
    }

    uint has_int:1;

    int int_id;
    QString str_id;
    // extendable to other types of identification
};

class QSignalMapperData {
public:
    QSignalMapperData()
    {
	dict.setAutoDelete( TRUE );
    }

    QPtrDict<QSignalMapperRec> dict;
};

/*!
    \class QSignalMapper qsignalmapper.h
    \brief The QSignalMapper class bundles signals from identifiable senders.

    \ingroup io

    This class collects a set of parameterless signals, and re-emits
    them with integer or string parameters corresponding to the object
    that sent the signal.
*/

/*!
    Constructs a QSignalMapper called \a name, with parent \a parent.
    Like all QObjects, it will be deleted when the parent is deleted.
*/
QSignalMapper::QSignalMapper( QObject* parent, const char* name ) :
    QObject( parent, name )
{
    d = new QSignalMapperData;
}

/*!
    Destroys the QSignalMapper.
*/
QSignalMapper::~QSignalMapper()
{
    delete d;
}

/*!
    Adds a mapping so that when map() is signaled from the given \a
    sender, the signal mapped(\a identifier) is emitted.

    There may be at most one integer identifier for each object.
*/
void QSignalMapper::setMapping( const QObject* sender, int identifier )
{
    QSignalMapperRec* rec = getRec(sender);
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
void QSignalMapper::setMapping( const QObject* sender, const QString &identifier )
{
    QSignalMapperRec* rec = getRec(sender);
    rec->str_id = identifier;
}

/*!
    Removes all mappings for \a sender. This is done automatically
    when mapped objects are destroyed.
*/
void QSignalMapper::removeMappings( const QObject* sender )
{
    d->dict.remove((void*)sender);
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
    const QObject* s = sender();
    QSignalMapperRec* rec = d->dict.find( (void*)s );
    if ( rec ) {
	if ( rec->has_int )
	    emit mapped( rec->int_id );
	if ( !rec->str_id.isEmpty() )
	    emit mapped( rec->str_id );
    }
}

QSignalMapperRec* QSignalMapper::getRec( const QObject* sender )
{
    QSignalMapperRec* rec = d->dict.find( (void*)sender );
    if (!rec) {
	rec = new QSignalMapperRec;
	d->dict.insert( (void*)sender, rec );
	connect( sender, SIGNAL(destroyed()), this, SLOT(removeMapping()) );
    }
    return rec;
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
