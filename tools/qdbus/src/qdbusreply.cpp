/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdbusreply.h"
#include "qdbusmetatype.h"
#include "qdbusmetatype_p.h"
#include <QDebug>

/*!
    \class QDBusReply
    \inmodule QtDBus
    \since 4.2

    \brief The QDBusReply class stores the reply for a method call to a remote object.

    A QDBusReply object is a subset of the QDBusMessage object that represents a method call's
    reply. It contains only the first output argument or the error code and is used by
    QDBusInterface-derived classes to allow returning the error code as the function's return
    argument.

    It can be used in the following manner:
    \code
        QDBusReply<QString> reply = interface->call("RemoteMethod");
        if (reply.isValid())
            // use the returned value
            useValue(reply.value());
        else
            // call failed. Show an error condition.
            showError(reply.error());
    \endcode

    If the remote method call cannot fail, you can skip the error checking:
    \code
        QString reply = interface->call("RemoteMethod");
    \endcode

    However, if it does fail under those conditions, the value returned by QDBusReply::value() is
    a default-constructed value. It may be indistinguishable from a valid return value.

    QDBusReply objects are used for remote calls that have no output
    arguments or return values (i.e., they have a "void" return
    type). Use the isValid() function to test if the reply succeeded.

    \sa QDBusMessage, QDBusInterface
*/

/*!
    \fn QDBusReply::QDBusReply(const QDBusMessage &reply)
    Automatically construct a QDBusReply object from the reply message \a reply, extracting the
    first return value from it if it is a success reply.
*/

/*!
    \fn QDBusReply::QDBusReply(const QDBusError &error)
    Constructs an error reply from the D-Bus error code given by \a error.
*/

/*!
    \fn QDBusReply::operator=(const QDBusReply &other)
    Makes this object be a copy of the object \a other.
*/

/*!
    \fn QDBusReply::operator=(const QDBusError &error)
    Sets this object to contain the error code given by \a error. You
    can later access it with error().
*/

/*!
    \fn QDBusReply::operator=(const QDBusMessage &message)

    Makes this object contain the reply specified by message \a
    message. If \a message is an error message, this function will
    copy the error code and message into this object

    If \a message is a standard reply message and contains at least
    one parameter, it will be copied into this object, as long as it
    is of the correct type. If it's not of the same type as this
    QDBusError object, this function will instead set an error code
    indicating a type mismatch.
*/

/*!
    \fn bool QDBusReply::isValid() const

    Returns true if no error occurred; otherwise, returns false.

    \sa error()
*/

/*!
    \fn QDBusReply::error()

    Returns the error code that was returned from the remote function call. If the remote call did
    not return an error (i.e., if it succeeded), then the QDBusError object that is returned will
    not be a valid error code (QDBusError::isValid() will return false).

    \sa isValid()
*/

/*!
    \fn QDBusReply::value() const
    Returns the remote function's calls return value. If the remote call returned with an error,
    the return value of this function is undefined and may be undistinguishable from a valid return
    value.

    This function is not available if the remote call returns \c void.
*/

/*!
    \fn QDBusReply::operator Type() const
    Returns the same as value().

    This function is not available if the remote call returns \c void.
*/

/*!
    \internal
    Fills in the QDBusReply data \a error and \a data from the reply message \a reply.
*/
void qDBusReplyFill(const QDBusMessage &reply, QDBusError &error, QVariant &data)
{
    error = reply;

    if (error.isValid()) {
        data = QVariant();      // clear it
        return;
    }

    if (reply.arguments().count() >= 1 && reply.arguments().at(0).userType() == data.userType()) {
        data = reply.arguments().at(0);
        return;
    }

    const char *expectedSignature = 0;
    QByteArray receivedSignature;

    if (reply.arguments().count() >= 1 &&
        reply.arguments().at(0).userType() == QDBusMetaTypeId::argument) {
        // compare signatures instead
        QDBusArgument arg = qvariant_cast<QDBusArgument>(reply.arguments().at(0));
        expectedSignature = QDBusMetaType::typeToSignature(data.userType());
        receivedSignature = arg.currentSignature().toLatin1();
        if (receivedSignature == expectedSignature) {
            // matched. Demarshall it
            QDBusMetaType::demarshall(arg, data.userType(), data.data());
            return;
        } else {
            if (!reply.arguments().isEmpty())
                receivedSignature = QDBusMetaType::typeToSignature(reply.arguments().at(0).userType());
            else
                receivedSignature = "no signature";
            expectedSignature = QDBusMetaType::typeToSignature(data.userType());
        }
    }

    // error
    QString errorMsg = QLatin1String("Unexpected reply signature: got \"%1\", "
                                     "expected \"%2\" (%3)");
    error = QDBusError(QDBusError::InvalidSignature,
                      errorMsg.arg(QLatin1String(receivedSignature),
                                   QLatin1String(expectedSignature),
                                   QLatin1String(data.typeName())));
    data = QVariant();      // clear it
}
