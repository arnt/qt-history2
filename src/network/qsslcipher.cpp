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

#include "qsslcipher.h"
#include "qsslcipher_p.h"
#include "qsslsocket.h"

#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/qdebug.h>
#endif

QSslCipher::QSslCipher()
    : d(new QSslCipherPrivate)
{
}

QSslCipher::QSslCipher(const QString &name, QSsl::SslProtocol protocol)
    : d(new QSslCipherPrivate)
{
    foreach (const QSslCipher &cipher, QSslSocket::supportedCiphers()) {
        if (cipher.name() == name && cipher.protocol() == protocol) {
            *this = cipher;
            return;
        }
    }
}

QSslCipher::QSslCipher(const QSslCipher &other)
    : d(new QSslCipherPrivate)
{
    *d = *other.d;
}

QSslCipher::~QSslCipher()
{
    delete d;
}

QSslCipher &QSslCipher::operator=(const QSslCipher &other)
{
    *d = *other.d;
    return *this;
}

bool QSslCipher::operator==(const QSslCipher &other) const
{
    return d->name == other.d->name && d->protocol == other.d->protocol;
}

bool QSslCipher::isNull() const
{
    return d->isNull;
}

QString QSslCipher::name() const
{
    return d->name;
}

int QSslCipher::supportedBits()const
{
    return d->supportedBits;
}

int QSslCipher::usedBits() const
{
    return d->bits;
}

QString QSslCipher::keyExchangeMethod() const
{
    return d->keyExchangeMethod;
}

QString QSslCipher::authenticationMethod() const
{
    return d->authenticationMethod;
}

QString QSslCipher::encryptionMethod() const
{
    return d->encryptionMethod;
}

QString QSslCipher::protocolString() const
{
    return d->protocolString;
}

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
