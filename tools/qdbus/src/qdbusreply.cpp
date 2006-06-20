/* qdbusreply.cpp QDBusReply object - a reply from D-Bus
 *
 * Copyright (C) 2006 Trolltech AS. All rights reserved.
 *    Author: Thiago Macieira <thiago.macieira@trolltech.com>
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "qdbusreply.h"

/*!
    \class QDBusReply
    \brief The reply for a method call to a remote object.

    A QDBusReply object is a subset of the QDBusMessage object that represents a method call's
    reply. It contains only the first output argument or the error code and is used by
    QDBusInterface-derived classes to allow returning the error code as the function's return
    argument.

    It can be used in the following manner:
    \code
        QDBusReply<QString> reply = interface->call("RemoteMethod");
        if (reply.isSuccess())
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
    undefined. It may be undistinguishable from a valid return value.

    QDBusReply objects are used for remote calls that have no output arguments or return values
    (i.e., they have a "void" return type). In this case, you can only test if the reply succeeded
    or not, by calling isError() and isSuccess(), and inspecting the error condition by calling
    error(). You cannot call value().

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
    \fn QDBusReply::isError() const
    Returns true if this reply is an error reply. You can extract the error contents using the
    error() function.
*/

/*!
    \fn QDBusReply::isSuccess() const
    Returns true if this reply is a normal error reply (not an error). You can extract the returned
    value with value()
*/

/*!
    \fn QDBusReply::error()
    Returns the error code that was returned from the remote function call. If the remote call did
    not return an error (i.e., if it succeeded), then the QDBusError object that is returned will
    not be a valid error code (QDBusError::isValid() will return false).
*/

/*!
    \fn QDBusReply::value()
    Returns the remote function's calls return value. If the remote call returned with an error,
    the return value of this function is undefined and may be undistinguishable from a valid return
    value.

    This function is not available if the remote call returns \c void.
*/

/*!
    \fn QDBusReply::operator Type()
    Returns the same as value().
    
    This function is not available if the remote call returns \c void.
*/

/*!
    \internal
    \fn QDBusReply::fromVariant(const QDBusReply<QVariant> &variantReply)
    Converts the QDBusReply<QVariant> object to this type by converting the variant contained in
    \a variantReply to the template's type and copying the error condition.

    If the QVariant in variantReply is not convertible to this type, it will assume an undefined
    value.
*/

void qDBusReplyFill(const QDBusMessage &reply, QDBusError &error, QVariant &data)
{
    error = reply;
    if (error.isValid()) {
        data = QVariant();      // clear it
        return;
    }

    if (reply.count() >= 1 && reply.at(0).userType() == data.userType()) {
        data = reply.at(0);
    } else {
        error = QDBusError(QDBusError::InvalidSignature,
                           QLatin1String("Unexpected reply signature"));
        data = QVariant();      // clear it
    }
}
