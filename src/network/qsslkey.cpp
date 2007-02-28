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
    \class QSslKey
    \brief The QSslKey class provides an interface for private and public keys.
    \since 4.3

    \reentrant
    \ingroup io
    \module network

    QSslKey provides a simple API for managing keys.

    \sa QSslSocket, QSslCertificate, QSslCipher
*/

/*!
    \enum QSslKey::Type

    Describes the two types of keys QSslKey supports.
    
    \value PrivateKey A private key.
    \value PublicKey A public key.
*/

/*!
    \enum QSslKey::Algorithm

    Describes the algorithm for the key.

    \value Rsa The RSA algorithm.
    \value Dsa The DSA algorithm.
*/

#include "qsslkey.h"

#include <QtCore/qbytearray.h>
#ifndef QT_NO_DEBUG
#include <QtCore/qdebug.h>
#endif
#include <QtCore/qpair.h>

class QSslKeyPrivate
{
public:
    inline QSslKeyPrivate()
    { clear(); }
    inline void clear()
    {
        isNull = true;
    }

    bool isNull;
    int keyLength;
    QByteArray key;
    QSslKey::Type type;
    QSslKey::Algorithm algorithm;
};

/*!
    Constructs a QSslKey by parsing \a encoded. You can call isNull() later to
    check if \a encoded contained a valid key or not.
*/
QSslKey::QSslKey(const QByteArray &encoded)
    : d(new QSslKeyPrivate)
{
    // ### unimplemented; need to dearm encoded first, then extract the key.
    Q_UNUSED(encoded);
}

/*!
    Constructs an identical copy of \a other.
*/
QSslKey::QSslKey(const QSslKey &other)
    : d(new QSslKeyPrivate)
{
    *d = *other.d;
}

/*!
    Destroys the QSslKey object.
*/
QSslKey::~QSslKey()
{
    delete d;
}

/*!
    Copies the contents of \a other into this key, making the two keys
    identical.

    Returns a reference to this QSslKey.
*/
QSslKey &QSslKey::operator=(const QSslKey &other)
{
    *d = *other.d;
    return *this;
}

/*!
    Returns true if this is a null key; otherwise, false is returned.

    \sa clear()
*/
bool QSslKey::isNull() const
{
    return d->isNull;
}

/*!
    Clears the contents of this key, making it a null key.

    \sa isNull()
*/
void QSslKey::clear()
{
    d->clear();
}

/*!
    Returns the length of the key in bits.
*/
int QSslKey::length() const
{
    return d->keyLength;
}

/*!
    Returns a pointer to key bits.

    \sa keyLength()
*/
const uchar *QSslKey::data() const
{
    return (const uchar *)d->key.constData();
}

/*!
    Returns the type of the key (i.e., PublicKey or PrivateKey).
*/
QSslKey::Type QSslKey::type() const
{
    return d->type;
}

/*!
    Returns the key algorithm.
*/
QSslKey::Algorithm QSslKey::algorithm() const
{
    return d->algorithm;
}

/*!
    Returns the key in DER encoding, optionally encrypted and protected by \a
    passPhrase.
*/
QByteArray QSslKey::toDer(const QByteArray &passPhrase) const
{
    // ### unimplemented
    Q_UNUSED(passPhrase);
    return QByteArray();
}

/*!
    Returns the key in PEM encoding, optionally encrypted and protected by \a
    passPhrase.
*/
QByteArray QSslKey::toPem(const QByteArray &passPhrase) const
{
    // ### unimplemented
    Q_UNUSED(passPhrase);
    return QByteArray();
}

/*!
    Generates and returns a new pair of keys (the first is a PrivateKey, and
    the second is the PublicKey). \a algorithm specifies what algorithm to use
    when generting the keys, and \a keyLength specifies the number of bits.

    This function can be time consuming, and will block the calling thread.
*/
QPair<QSslKey, QSslKey> QSslKey::generateKeyPair(Algorithm algorithm, int keyLength)
{
    // ### unimplemented
    Q_UNUSED(algorithm);
    Q_UNUSED(keyLength);
    return QPair<QSslKey, QSslKey>();
}

#ifndef QT_NO_DEBUG
class QDebug;
QDebug operator<<(QDebug debug, const QSslKey &key)
{
    debug << "QSslKey("
          << (key.type() == QSslKey::PublicKey ? "PublicKey" : "PrivateKey")
          << ", " << (key.algorithm() == QSslKey::Rsa ? "RSA" : "DSA")
          << ", " << key.length()
          << ")";
    return debug;
}
#endif
