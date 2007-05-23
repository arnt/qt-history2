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

/****************************************************************************
**
** In addition, as a special exception, Trolltech gives permission to link
** the code of its release of Qt with the OpenSSL project's "OpenSSL" library
** (or modified versions of the "OpenSSL" library that use the same license
** as the original version), and distribute the linked executables.
**
** You must comply with the GNU General Public License version 2 in all
** respects for all of the code used other than the "OpenSSL" code.  If you
** modify this file, you may extend this exception to your version of the file,
** but you are not obligated to do so.  If you do not wish to do so, delete
** this exception statement from your version of this file.
**
****************************************************************************/

/*!
    \class QSslCipher
    \brief The QSslCipher class represents an SSL cryptographic cipher.
    \since 4.3

    \reentrant
    \ingroup io
    \module network

    QSslCipher stores information about one cryptographic cipher. It
    is most commonly used with QSslSocket, either for configuring
    which ciphers the socket can use, or for displaying the socket's
    ciphers to the user.

    \sa QSslSocket, QSslKey
*/

#include "qsslcipher.h"
#include "qsslcipher_p.h"
#include "qsslsocket.h"

#ifndef QT_NO_DEBUG
#include <QtCore/qdebug.h>
#endif

/*!
    Constructs an empty QSslCipher object.
*/
QSslCipher::QSslCipher()
    : d(new QSslCipherPrivate)
{
}

/*!
    Constructs a QSslCipher object for the cipher determined by \a
    name and \a protocol. The constructor accepts only supported
    ciphers (i.e., the \a name and \a protocol must identify a cipher
    in the list of ciphers returned by
    QSslSocket::supportedCiphers()).

    You can call isNull() after construction to check if \a name and
    \a protocol correctly identified a supported cipher.
*/
QSslCipher::QSslCipher(const QString &name, QSsl::SslProtocol protocol)
{
    foreach (const QSslCipher &cipher, QSslSocket::supportedCiphers()) {
        if (cipher.name() == name && cipher.protocol() == protocol) {
            *this = cipher;
            return;
        }
    }
}

/*!
    Constructs an identical copy of the \a other cipher.
*/
QSslCipher::QSslCipher(const QSslCipher &other)
    : d(new QSslCipherPrivate)
{
    *d = *other.d;
}

/*!
    Destroys the QSslCipher object.
*/
QSslCipher::~QSslCipher()
{
    delete d;
}

/*!
    Copies the contents of \a other into this cipher, making the two
    ciphers identical.
*/
QSslCipher &QSslCipher::operator=(const QSslCipher &other)
{
    *d = *other.d;
    return *this;
}

/*!
    Returns true if this cipher is the same as \a other; otherwise,
    false is returned.
*/
bool QSslCipher::operator==(const QSslCipher &other) const
{
    return d->name == other.d->name && d->protocol == other.d->protocol;
}

/*!
    \fn bool QSslCipher::operator!=(const QSslCipher &other) const

    Returns true if this cipher is not the same as \a other;
    otherwise, false is returned.
*/

/*!
    Returns true if this is a null cipher; otherwise returns false.
*/
bool QSslCipher::isNull() const
{
    return d->isNull;
}

/*!
    Returns the name of the cipher, or an empty QString if this is a null
    cipher.

    \sa isNull()
*/
QString QSslCipher::name() const
{
    return d->name;
}

/*!
    Returns the number of bits supported by the cipher.

    \sa usedBits()
*/
int QSslCipher::supportedBits()const
{
    return d->supportedBits;
}

/*!
    Returns the number of bits used by the cipher.

    \sa supportedBits()
*/
int QSslCipher::usedBits() const
{
    return d->bits;
}

/*!
    Returns the cipher's key exchange method as a QString.
*/
QString QSslCipher::keyExchangeMethod() const
{
    return d->keyExchangeMethod;
}
 
/*!
    Returns the cipher's authentication method as a QString.
*/
QString QSslCipher::authenticationMethod() const
{
    return d->authenticationMethod;
}

/*!
    Returns the cipher's encryption method as a QString.
*/
QString QSslCipher::encryptionMethod() const
{
    return d->encryptionMethod;
}

/*!
    Returns the cipher's protocol as a QString.

    \sa protocol()
*/
QString QSslCipher::protocolString() const
{
    return d->protocolString;
}

/*!
    Returns the cipher's protocol type, or \l QSsl::UnknownProtocol if
    QSslCipher is unable to determine the protocol (protocolString() may
    contain more information).

    \sa protocolString()
*/
QSsl::SslProtocol QSslCipher::protocol() const
{
    return d->protocol;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const QSslCipher &cipher)
{
    debug << "QSslCipher(name=" << qPrintable(cipher.name())
          << ", bits=" << cipher.usedBits()
          << ", proto=" << qPrintable(cipher.protocolString())
          << ")";
    return debug;
}
#endif
