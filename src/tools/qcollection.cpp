/****************************************************************************
** $Id: //depot/qt/main/src/tools/qcollection.cpp#27 $
**
** Implementation of base class for all collection classes
**
** Created : 920820
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qcollection.h"

/*!
  \class QCollection qcollection.h
  \brief The QCollection class is the base class of all Qt collections.

  \ingroup collection
  \ingroup tools

  The QCollection class is an abstract base class for the Qt \link
  collection.html collection classes\endlink QDict, QList etc. via QGDict,
  QGList etc.

  A QCollection knows only about the number of objects in the collection and
  the \link setAutoDelete() deletion strategy\endlink.

  A collection is implemented using the \c Item (generic collection item)
  type, which is a \c void*.  The template (or macro) classes that
  create the real collections cast the \c Item to the required type.

  \sa \link collection.html Collection Classes\endlink
*/


/*!
  \fn QCollection::QCollection()

  Constructs a collection. The constructor is protected because
  QCollection is an abstract class.
*/

/*!
  \fn QCollection::QCollection( const QCollection & source )

  Constructs a copy of \a source with autoDelete() set to FALSE. The
  constructor is protected because QCollection is an abstract class.

  Note that if \a source has autoDelete turned on, copying it is a
  good way to get memory leaks, reading freed memory, or both.
*/

/*!
  \fn QCollection::~QCollection()
  Destroys the collection. The destructor is protected because QCollection
  is an abstract class.
*/


/*!
  \fn bool QCollection::autoDelete() const
  Returns the setting of the auto-delete option (default is FALSE).
  \sa setAutoDelete()
*/

/*!
  \fn void QCollection::setAutoDelete( bool enable )
  Sets the auto-delete option of the collection.

  Enabling auto-delete (\e enable is TRUE) will delete objects that
  are removed from the collection.  This can be useful if the
  collection has the only reference to the objects.  (Note that the
  object can still be copied using the copy constructor - copying such
  objects is a good way to get memory leaks, reading freed memory or
  both.)

  Disabling auto-delete (\e enable is FALSE) will \e not delete objects
  that are removed from the collection.	 This is useful if the objects
  are part of many collections.

  The default setting is FALSE.

  \sa autoDelete()
*/


/*!
  \fn virtual uint QCollection::count() const
  Returns the number of objects in the collection.
*/

/*!
  \fn virtual void QCollection::clear()
  Removes all objects from the collection.  The objects will be deleted
  if auto-delete has been enabled.
  \sa setAutoDelete()
*/


/*!
  Virtual function that creates a copy of an object that is about to
  be inserted into the collection.

  The default implementation returns the \e d pointer, i.e. no copy
  is made.

  This function is seldom reimplemented in the collection template
  classes. It is not common practice to make a copy of something
  that is being inserted.

  \sa deleteItem()
*/

QCollection::Item QCollection::newItem( Item d )
{
    return d;					// just return reference
}

/*!
  Virtual function that deletes an item that is about to be removed from
  the collection.

  The default implementation deletes \e d pointer if and only if
  auto-delete has been enabled.

  This function is always reimplemented in the collection template
  classes.

  ***NOTE*** If you reimplement this function you must also reimplement the
  destructor and call the virtual function clear() from your
  destructor. This is due to the way virtual functions and destructors
  work in C++, virtual functions in derived classes cannot be called from
  a destructor. If you do not do this your deleteItem() function will not
  be called if the container is not empty when it is destructed.

  \sa newItem(), setAutoDelete()
*/

void QCollection::deleteItem( Item d )
{
    if ( del_item )
	delete d;				// default operation
}
