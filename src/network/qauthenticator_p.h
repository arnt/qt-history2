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

#ifndef QAUTHENTICATOR_P_H
#define QAUTHENTICATOR_P_H

#include <qhash.h>
#include <qbytearray.h>
#include <qstring.h>
#include <qauthenticator.h>

class QHttpResponseHeader;

class QAuthenticatorPrivate
{
public:
    enum Method { None, Basic, Plain, Login, Ntlm, CramMd5, DigestMd5 };
    QAuthenticatorPrivate();

    QAtomic ref;
    QString user;
    QString password;
    QHash<QByteArray, QByteArray> options;
    Method method;
    QString realm;
    QByteArray challenge;

    enum Phase {
        Start,
        Phase2, 
        Done
    };
    Phase phase;

    // digest specific
    QByteArray cnonce;
    int nonceCount;

    // ntlm specific
    QString workstation;
    
    QByteArray calculateResponse(const QByteArray &requestLine);

    inline static QAuthenticatorPrivate *getPrivate(QAuthenticator &auth) { return auth.d; }
    inline static const QAuthenticatorPrivate *getPrivate(const QAuthenticator &auth) { return auth.d; }

    QByteArray digestMd5Response(const QByteArray &challenge, const QByteArray &requestLine);
    static QHash<QByteArray, QByteArray> parseDigestAuthenticationChallenge(const QByteArray &challenge);

    void parseHttpResponse(const QHttpResponseHeader &, bool isProxy);

};


#endif
