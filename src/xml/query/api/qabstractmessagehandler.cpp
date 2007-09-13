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

#include <QMutex>

#include "qabstractmessagehandler.h"

QT_BEGIN_NAMESPACE

class QAbstractMessageHandlerPrivate
{
public:
    QMutex mutex;
};

/*!
  \class QAbstractMessageHandler
  \threadsafe
  \since 4.4

  \brief A callback for receiving messages.

  QAbstractMessageHandler is an abstract base class that provides an interface for
  receiving messages. Typically one subclass QAbstractMessageHandler
  and implement the handleMessage() function and subsequently pass a pointer to the code that
  generates the messages.

  The user of QAbstractMessageHandler calls message(). message() forwards the arguments
  on to handleMessage() which the sub-class has implemented, and while doing so seralizes
  the calls. This means QAbstractMessageHandler is thread safe.

  QAbstractMessageHandler isn't tied to a particular use case. This means that the
  interpretation of the arguments passed to message(), depends on the context
  in which it is used, and must therefore be specified in that context.

  QAbstractMessageHandler subclasses QSharedData, meaning it is a reference counted class. The advantage
  of this is that one never has to consider ownership since it is always used using
  smart pointers and one should never manually delete a QAbstractMessageHandler instance, or allocate it
  on the stack. Creating such a smart pointer is done using the Ptr typedef:

  \code
  QAbstractMessageHandler::Ptr myPointer(new MyMessageHandler());
  \endcode

 */

/*!
  Constructs a QAbstractMessageHandler.
 */
QAbstractMessageHandler::QAbstractMessageHandler() : d(new QAbstractMessageHandlerPrivate())
{
}

/*!
  Destructs this QAbstractMessageHandler.
 */
QAbstractMessageHandler::~QAbstractMessageHandler()
{
    delete d;
}

/*!
  How \a type and \a description, its format and content, are interpreted
  must be defined by the context in which this message handler is used. \a identifier
  is a URI that identifies the message, and therefore is the key to how the other
  arguments should be interpreted.

  \a sourceLocation identifies where the message was generated.

  Since \a identifier is globally unique, messages from arbitrary sources can safely
  be identified.

  It is the caller's responsibility to guarantee that \a identifier is a valid QUrl,
  or a default constructed QUrl.

  This function unconditionally calls handleMessage().
 */
void QAbstractMessageHandler::message(QtMsgType type,
                                      const QString &description,
                                      const QUrl &identifier,
                                      const QSourceLocation &sourceLocation)
{
    QMutexLocker(&d->mutex);
    handleMessage(type, description, identifier, sourceLocation);
}

/*!
  \fn void QAbstractMessageHandler::handleMessage(QtMsgType type,
                                            const QString &description,
                                            const QUrl &identifier = QUrl(),
                                            const QSourceLocation &sourceLocation = QSourceLocation()) = 0

  This function is re-implemented by the sub-class. It's called by message() and receives its arguments,
  \a type, \a description, \a identifier and \a sourceLocation from message() unmodified.
 */

QT_END_NAMESPACE

// vim: et:ts=4:sw=4:sts=4
