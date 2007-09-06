/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the $MODULE$ of the Qt Toolkit.
 * **
 * ** $TROLLTECH_DUAL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

#include "qabstractxmlpushcallback.h"

/*!
  \class QAbstractXmlPushCallback
  \brief A push-based, streaming event interface for the XPath Data Model.
  \reentrant
  \since 4.4

  QAbstractXmlPushCallback allows a stream of events representing an instance of
  the XPath Data Model -- essentially XML --
  to be retrieved through a callback. The QAbstractXmlPushCallback subclass retrieves the events,
  and the agent calling the QAbstractXmlPushCallback provides the events.

  The caller of the QAbstractXmlPushCallback lives to a certain obligation of in what order the various
  members are called, in what states and so on. This is identical to as documented
  for QAbstractXmlPullProvider where QAbstractXmlPushCallback::Event maps to members in QAbstractXmlPushCallback
  in expected ways. For instance, the obligations for when QAbstractXmlPullProvider::StartElement can occur,
  also mandates when QAbstractXmlPullProvider::startElement() can be called.

  When sending events to this QAbstractXmlPushCallback, make sure the events are delivered in
  the manner that this class mandates.
 */

/*!
  Constructs a QAbstractXmlPushCallback instance.
 */
QAbstractXmlPushCallback::QAbstractXmlPushCallback()
{
}

/*!
  Destructs this QAbstractXmlPushCallback.
 */
QAbstractXmlPushCallback::~QAbstractXmlPushCallback()
{
}

/*!
  \fn void QAbstractXmlPushCallback::startElement(const QXmlName &name) = 0;

  Signals the start of an element by name \a name.
 */

/*!
  \fn void QAbstractXmlPushCallback::endElement() = 0;

  Signals the end of the most recently received element.
*/

/*!
  Signals an attribute by name \a name, and value \a value.

  \fn void QAbstractXmlPushCallback::attribute(const QXmlName &name,
                                            const QString &value) = 0;
 */

/*!
  \fn void QAbstractXmlPushCallback::comment(const QString &value) = 0;

  Signals a comment whose value is \a value.
 */

/*!
  \fn void QAbstractXmlPushCallback::characters(const QString &value) = 0;

  Signals a text node with content \a value. Adjacent text nodes may not occur.
 */

/*!
  \fn void QAbstractXmlPushCallback::startDocument() = 0;

  Signals the start of a document node.
 */

/*!
  \fn void QAbstractXmlPushCallback::endDocument() = 0;

  Signals the end of a document node.
 */

/*!
  \fn void QAbstractXmlPushCallback::processingInstruction(const QXmlName &target,
                                                        const QString &value) = 0;

 Signals a processing instruction with \a target (the name) and content \a value.

 The caller guarantees that \a value does not contain the string "?>".
 */

/*!
  \fn void QAbstractXmlPushCallback::atomicValue(const QVariant &value) = 0;

  Signals an atomic \a value.
 */

/*!
  \fn virtual void QAbstractXmlPushCallback::namespaceBinding(const QXmlName &name) = 0;

  Signals a namespace binding. In \a name, the namespace URI is the namespace URI, and the local name
  is the prefix the binding binds to.
 */

// vim: et:ts=4:sw=4:sts=4
