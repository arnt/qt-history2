/****************************************************************************
** $Id: //depot/qt/main/src/tools/qcollect.cpp#10 $
**
** Implementation of base class for all collection classes
**
** Author  : Haavard Nord
** Created : 920820
**
** Copyright (C) 1992-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcollect.h"

RCSTAG("$Id: //depot/qt/main/src/tools/qcollect.cpp#10 $")


/*----------------------------------------------------------------------------
  \class QCollection qcollect.h
  \brief This is the base class of all Qt collections.

  \ingroup collection

  The QCollection class is an abstract base class for the Qt \link
  collection.html collection classes\endlink QDict, QList etc. via QGDict,
  QGList etc.

  A QCollection knows only about the number of objects in the collection and
  the \link setAutoDelete() deletion strategy\endlink.

  A collection is implemented using the \c GCI (generic collection item)
  type, which is a \c void*.  The template (or macro) classes that
  create the real collections cast the \c GCI to the required type.

  \sa \link collection.html Collection Classes\endlink
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn QCollection::QCollection()
  Constructs a collection. The constructor is protected because QCollection
  is an abstract class.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QCollection::~QCollection()
  Destroys the collection. The destructor is protected because QCollection
  is an abstract class.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn bool QCollection::autoDelete() const
  Returns the setting of the auto-delete option (default is FALSE).
  \sa setAutoDelete()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QCollection::setAutoDelete( bool enable )
  Sets the auto-delete option of the collection.

  Enabling auto-delete (\e enable is TRUE) will delete objects that
  are removed from the collection.  This can be useful if the collection
  has the only reference to the objects.

  Disabling auto-delete (\e enable is FALSE) will \e not delete objects
  that are removed from the collection.  This is useful if the objects
  are part of many collections.

  The default setting is FALSE.

  \sa autoDelete()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn virtual uint QCollection::count() const = 0
  Returns the number of objects in the collection.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn virtual void QCollection::clear() = 0
  Removes all objects from the collection.  The objects will be deleted
  if auto-delete has been enabled.
  \sa setAutoDelete()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Virtual function that creates a copy of an object that is about to
  be inserted into the collection.

  The default implementation returns the \e d pointer, i.e. no copy
  is made.

  This function is seldom reimplemented in the collection template
  classes. It is not common practice to make a copy of something
  that is being inserted.

  \sa deleteItem()
 ----------------------------------------------------------------------------*/

GCI QCollection::newItem( GCI d )
{
    return d;					// just return reference
}

/*----------------------------------------------------------------------------
  Virtual function that deletes an item that is about to be removed from
  the collection.

  The default implementation deletes \e d pointer if and only if
  auto-delete has been enabled.

  This function is always reimplemented in the collection template
  classes.

  \sa newItem(), setAutoDelete()
 ----------------------------------------------------------------------------*/

void QCollection::deleteItem( GCI d )
{
    if ( del_item )
	delete d;				// default operation
}
