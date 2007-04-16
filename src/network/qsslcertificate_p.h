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

#ifndef QSSLCERTIFICATE_P_H
#define QSSLCERTIFICATE_P_H

#include "qsslcertificate.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "qsslsocket_p.h"
#include <QtCore/qdatetime.h>
#include <QtCore/qmap.h>

#include <openssl/x509.h>

class QSslCertificatePrivate
{
public:
    QSslCertificatePrivate()
        : null(true), x509(0)
    { 
        QSslSocketPrivate::ensureInitialized();
    }

    bool null;
    QByteArray versionString;
    QByteArray serialNumberString;

    QMap<QString, QString> issuerInfo;
    QMap<QString, QString> subjectInfo;
    QDateTime notValidAfter;
    QDateTime notValidBefore;

    X509 *x509;

    static QByteArray QByteArray_from_X509(X509 *x509, bool pemEncoded);
    static QSslCertificate QSslCertificate_from_X509(X509 *x509);
    static QList<QSslCertificate> QSslCertificates_from_QByteArray(const QByteArray &array, int count = -1);

    friend class QSslSocketBackendPrivate;
};

#endif
