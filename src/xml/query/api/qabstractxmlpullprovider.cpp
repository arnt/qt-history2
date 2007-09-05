/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.
 * **
 * ** $TROLLTECH_GPL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#include <QHash>

#include "qxmlname.h"
#include "qabstractxmlpullprovider.h"

/*!
  \class QAbstractXmlPullProvider
  \brief A pull-based, stream interface for the XPath Data Model.
  \reentrant
  \since 4.4

  QAbstractXmlPullProvider allows a stream of items from the XPath Data Model -- essentially XML --
  to be iterated over. The subclass of QAbstractXmlPullProvider provides the events, and the
  user calling next() and so on, consumes them. QAbstractXmlPullProvider can be considered
  a forward-only, non-reversible iterator. 

  Note that the content the events describes, are not necessarily a well-formed XML document, but
  rather an instance of the XPath Data model, to be specific. For instance, maybe a pull provider
  returns two atomic values, followed by an element tree, and at the end two document nodes.

  If you are subclassing QAbstractXmlPullProvider, be careful to correctly implement
  the behaviors, as described for the individual members and events.

  \sa QAbstractXmlPullProvider::Event
 */

/*!
  \enum QAbstractXmlPullProvider::Event
  \value StartOfInput The value QAbstractXmlPullProvider::current() returns before the first call to next().
  \value AtomicValue an atomic value such as an \c xs:integer, \c xs:hexBinary, or \c xs:dateTime. Atomic values
         can only be top level items.
  \value StartDocument Signals the start of a document node. Note that a QAbstractXmlPullProvider can provide
         a sequence of document nodes.
  \value EndDocument Signals the end of a document node. StartDocument and EndDocument are always balanced
         and always top-level events. For instance, StartDocument can never appear after any StartElement
         events that hasn't been balanced by the corresponding amount of EndElement events.
  \value StartElement Signals an element start tag.
  \value EndElement Signals the end of an element. StartElement and EndElement events are always balanced.
  \value Text Signals a text node. Adjacent text nodes cannot occur.
  \value ProcessingInstruction A processing instruction. Its name is returned from name(), and its value in stringValue().
  \value Comment a comment node. Its value can be retrieved with stingValue().
  \value Attribute Signals an attribute node. Attribute events can only appear after Namespace events, or
         if no such are sent, after the StartElement. In addition they must appear sequentially,
         and each name must be unique. The ordering of attribute events is undefined and insignificant.
  \value Namespace Signals a namespace binding. They occur very infrequently and are not needed for attributes
         and elements. Namespace events can only appear after the StartElement event. The
         ordering of namespace events is undefined and insignificant.
  \value EndOfInput When next() is called after the last event, EndOfInput is returned.

  \sa QAbstractXmlPullProvider::current()
 */

/*!
  Constucts a QAbstractXmlPullProvider instance.
 */
QAbstractXmlPullProvider::QAbstractXmlPullProvider()
{
}

/*!
  Destructs this QAbstractXmlPullProvider.
 */
QAbstractXmlPullProvider::~QAbstractXmlPullProvider()
{
}

/*!
  \fn Event QAbstractXmlPullProvider::next() = 0;
  Advances this QAbstractXmlPullProvider, and returns the new event.

  \sa current()
 */

/*!
  \fn Event QAbstractXmlPullProvider::current() const = 0;
  Returns the event that next() returned the last time it was called. It doesn't
  alter this QAbstractXmlPullProvider.

  current() may not modify this QAbstractXmlPullProvider's state. Subsequent calls to current()
  must return the same value.

  \sa QAbstractXmlPullProvider::Event
 */

/*!
  \fn QName QAbstractXmlPullProvider::name() const = 0;
  If the current event is StartElement,
  EndElement, ProcessingInstruction, Attribute, or Namespace, the node's name is returned.

  If the current event is ProcessingInstruction,
  the processing instruction target is in in the local name.

  If the current event is Namespace, the name's namespace URI is the namespace, and
  the local name is the prefix the name is binding to.

  In all other cases, an invalid QName is returned.
 */

/*!
  \fn QVariant QAbstractXmlPullProvider::atomicValue() const = 0;

  If current() event is AtomicValue, the atomic value is returned as a QVariant.
  In all other cases, this function returns a null QVariant.
 */

/*!
 \fn QString QAbstractXmlPullProvider::stringValue() const = 0;

  If current() is Text, the text node's value is returned.

  If the current() event is Comment, its value is returned. The subclasser guarantees
  it does not contain the string "-->".

  If the current() event is ProcessingInstruction, its data is returned. The subclasser
  guarantees the data does not contain the string "?>".

  In other cases, it returns a default constructed string.
  */

/*!
  This is a convenience function that skip all namespace nodes, and returns 
  the following attributes, if any, in a hash.

  When calling, the current event must be StartElement. An assert checks this, when
  building in debug mode.
 */
QHash<QXmlName, QString> QAbstractXmlPullProvider::attributes()
{
    Q_ASSERT_X(current() == StartElement, Q_FUNC_INFO,
               "This function can only be called after the beginning of an element.");

    /* Skip the namespace nodes. */
    while(next() == Namespace)
    {
    }

    /* Do we have any attributes at all? */
    if(current() != Attribute)
        return QHash<QXmlName, QString>();

    QHash<QXmlName, QString> result;
    result.reserve(3);

    do
    {
        result.insert(name(), stringValue());
    }
    while(next() == Attribute);

    return result;
}

// vim: et:ts=4:sw=4:sts=4
