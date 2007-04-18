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

#ifndef QSSLKEY_P_H
#define QSSLKEY_P_H

#include "qsslkey.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qsslcertificate.cpp.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#include <openssl/rsa.h>
#include <openssl/dsa.h>

class QSslKeyPrivate
{
public:
    inline QSslKeyPrivate()
        : rsa(0)
        , dsa(0)
    {
        clear();
        ref = 1;
    }

    inline ~QSslKeyPrivate()
    { clear(); }

    void clear(bool deep = true);

    void decodePem(const QByteArray &pem, const QByteArray &passPhrase,
                   bool deepClear = true);
    QByteArray pemHeader() const;
    QByteArray pemFooter() const;
    QByteArray pemFromDer(const QByteArray &der) const;
    QByteArray derFromPem(const QByteArray &pem) const;

    bool isNull;
    QSsl::KeyType type;
    QSsl::Algorithm algorithm;
    RSA *rsa;
    DSA *dsa;

    QAtomic ref;
};

#endif // QSSLKEY_P_H
