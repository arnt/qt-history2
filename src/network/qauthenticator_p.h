#ifndef QAUTHENTICATOR_P_H
#define QAUTHENTICATOR_P_H

#include <qhash.h>
#include <qbytearray.h>
#include <qstring.h>
#include <qauthenticator.h>

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

    QByteArray cnonce;
    int nonceCount;
    
    QByteArray calculateResponse(const QByteArray &requestLine);

    inline static QAuthenticatorPrivate *getPrivate(QAuthenticator &auth) { return auth.d; }
    inline static const QAuthenticatorPrivate *getPrivate(const QAuthenticator &auth) { return auth.d; }

    QByteArray digestMd5Response(const QByteArray &challenge, const QByteArray &requestLine);
    static QHash<QByteArray, QByteArray> parseDigestAuthenticationChallenge(const QByteArray &challenge);

    void parseHttpResponse(const QByteArray &httpResponse, bool isProxy, bool *passOk);

};


#endif
