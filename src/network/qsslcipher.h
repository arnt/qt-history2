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

#ifndef QSSLCIPHER_H
#define QSSLCIPHER_H

#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Network)

#ifndef QT_NO_OPENSSL

class QSslCipherPrivate;
class Q_NETWORK_EXPORT QSslCipher
{
public:
    enum Protocol {
        SslV3,
        SslV2,
        TlsV1,
        Unknown
    };

    QSslCipher();
    QSslCipher(const QString &name, Protocol protocol);
    QSslCipher(const QSslCipher &other);
    ~QSslCipher();
    QSslCipher &operator=(const QSslCipher &other);
    bool operator==(const QSslCipher &other) const;
    inline bool operator!=(const QSslCipher &other) const { return !operator==(other); }

    bool isNull() const;
    QString name() const;
    int supportedBits() const;
    int usedBits() const;

    QString keyExchangeMethod() const;
    QString authenticationMethod() const;
    QString encryptionMethod() const;
    QString protocolString() const;
    Protocol protocol() const;

private:
    QSslCipherPrivate *d;
    friend class QSslSocketBackendPrivate;
};

#ifndef QT_NO_DEBUG_STREAM
class QDebug;
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslCipher &cipher);
#endif

#endif // QT_NO_OPENSSL

QT_END_HEADER

#endif

