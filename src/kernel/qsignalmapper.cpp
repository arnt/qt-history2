/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignalmapper.cpp#3 $
**
** Implementation of QSignalMapper class
**
** Created : 980503
**
** Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qsignalmapper.h"
#include "qptrdict.h"

struct QSignalMapperRec {
    QSignalMapperRec()
    {
	has_int = 0;
	str_id = 0;
    }

    int has_int:1;

    int int_id;
    const char* str_id;
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
  \brief A QSignalMapper object bundles signals from identifiable senders.

  Collects a set of parameterless signals, re-emitting them with an
  integer or string parameters corresponding to the object which sent the
  signal.
*/

/*!
  Constructs a QSignalMapper.  Like all QObjects, it will be deleted when the 
  parent is deleted.
*/
QSignalMapper::QSignalMapper( QObject* parent, const char* name ) :
    QObject( parent, name )
{
    d = new QSignalMapperData;
}

/*!
  Destructs the QSignalMapper.
*/
QSignalMapper::~QSignalMapper()
{
    delete d;
}

/*!
  Adds a mapping such that when map() is signalled from the given
  sender, the signal mapper(identifier) is emitted.

  There may be at most one integer identifier for each object.
*/
void QSignalMapper::setMapping( const QObject* sender, int identifier )
{
    QSignalMapperRec* rec = getRec(sender);
    rec->int_id = identifier;
    rec->has_int = 1;
}

/*!
  Adds a mapping such that when map() is signalled from the given
  sender, the signal mapper(identifier) is emitted.

  There may be at most one string identifier for each object, and
  it may not be null.
*/
void QSignalMapper::setMapping( const QObject* sender, const char* identifier )
{
    QSignalMapperRec* rec = getRec(sender);
    rec->str_id = identifier;
}

/*!
  Removes all mappings for \a sender.  This is done automatically
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
  This slot emits signals based on which object sends signals
  to it.
*/
void QSignalMapper::map()
{
    const QObject* s = sender();
    QSignalMapperRec* rec = d->dict.find( (void*)s );
    if ( rec ) {
	if ( rec->has_int )
	    emit mapped( rec->int_id );
	if ( rec->str_id )
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
