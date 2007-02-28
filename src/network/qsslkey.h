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

#ifndef QSSLKEY_H
#define QSSLKEY_H

#include <QtCore/qbytearray.h>

QT_BEGIN_HEADER

QT_MODULE(Network)

#ifndef QT_NO_OPENSSL

template <typename A, typename B> struct QPair;
    
class QSslKeyPrivate;
class Q_NETWORK_EXPORT QSslKey
{
public:
    enum Type {
        PrivateKey,
        PublicKey
    };

    enum Algorithm {
        Rsa,
        Dsa
    };

    QSslKey(const QByteArray &encoded = QByteArray());
    QSslKey(const QSslKey &other);
    ~QSslKey();
    QSslKey &operator=(const QSslKey &other);

    bool isNull() const;
    void clear();

    int length() const;
    const uchar *data() const;
    Type type() const;
    Algorithm algorithm() const;

    QByteArray toPem(const QByteArray &passPhrase = QByteArray()) const;
    QByteArray toDer(const QByteArray &passPhrase = QByteArray()) const;

    static QPair<QSslKey, QSslKey> generateKeyPair(Algorithm algorithm, int keyLength);

private:
    QSslKeyPrivate *d;
};

#ifndef QT_NO_DEBUG
class QDebug;
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslKey &key);
#endif

#endif // QT_NO_OPENSSL

QT_END_HEADER

#endif
