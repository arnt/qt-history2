#include <qauthenticator.h>
#include <qauthenticator_p.h>
#include <qdebug.h>
#include <qhash.h>
#include <qbytearray.h>
#include <qcryptographichash.h>
#include <qhttp.h>

QAuthenticator::QAuthenticator()
    : d(0)
{
}

QAuthenticator::~QAuthenticator()
{
    if (d && !d->ref.deref())
        delete d;
}


QAuthenticator::QAuthenticator(const QAuthenticator &other)
    : d(other.d)
{
    if (d)
        d->ref.ref();
}

QAuthenticator &QAuthenticator::operator=(const QAuthenticator &other)
{
    if (d == other.d)
        return *this;
    QAuthenticatorPrivate *x = other.d;
    x = qAtomicSetPtr(&d, x);
    if (x && !x->ref.deref())
        delete x;
    return *this;
}

QString QAuthenticator::user() const
{
    return d ? d->user : QString();
}

void QAuthenticator::setUser(const QString &user)
{
    detach();
    d->user = user;
}

QString QAuthenticator::password() const
{
    return d ? d->password : QString();
}

void QAuthenticator::setPassword(const QString &password)
{
    detach();
    d->password = password;
}

void QAuthenticator::detach()
{
    if (!d) {
        d = new QAuthenticatorPrivate;
        d->ref.ref();
        return;
    }
    
    if (d->ref.ref() == 1)
        return;
    QAuthenticatorPrivate *x = new QAuthenticatorPrivate(*d);
    x = qAtomicSetPtr(&d, x);
    if (!x->ref.deref())
        delete x;
}

bool QAuthenticator::isNull() const
{
    return !d;
}

QAuthenticatorPrivate::QAuthenticatorPrivate()
    : ref(0)
    , method(None)
    , nonceCount(0)
{
    cnonce = QCryptographicHash::hash(QByteArray::number(rand(), 16) + QByteArray::number(rand(), 16),
                                      QCryptographicHash::Md5).toHex();
    nonceCount = 0;
}

QByteArray QAuthenticatorPrivate::calculateResponse(const QByteArray &requestLine)
{
    QByteArray response;
    const char *methodString;
    switch(method) {
    case QAuthenticatorPrivate::None:
        methodString = "";
        break;
    case QAuthenticatorPrivate::Plain:
        response = '\0' + user.toUtf8() + '\0' + password.toUtf8();
        break;
    case QAuthenticatorPrivate::Basic:
        methodString = "Basic ";
        response = user.toLatin1() + ':' + password.toLatin1();
        response = response.toBase64();
        break;
    case QAuthenticatorPrivate::Login:
        if (challenge.contains("VXNlciBOYW1lAA=="))
            response = user.toUtf8().toBase64();
        else if (challenge.contains("UGFzc3dvcmQA"))
            response = password.toUtf8().toBase64();
        break;
    case QAuthenticatorPrivate::CramMd5:
        break;
    case QAuthenticatorPrivate::DigestMd5:
        methodString = "Digest ";
        response = digestMd5Response(challenge, requestLine);
        break;
    case QAuthenticatorPrivate::Ntlm:
        methodString = "NTLM ";
        break;
    }
    return QByteArray(methodString) + response;
}

QHash<QByteArray, QByteArray> QAuthenticatorPrivate::parseDigestAuthenticationChallenge(const QByteArray &challenge)
{
    QHash<QByteArray, QByteArray> options;
    // parse the challenge
    const char *d = challenge.constData();
    const char *end = d + challenge.length();
    while (d < end) {
        while (d < end && (*d == ' ' || *d == '\n' || *d == '\r'))
            ++d;
        const char *start = d;
        while (d < end && *d != '=')
            ++d;
        QByteArray key = QByteArray(start, d - start);
        ++d;
        if (d >= end)
            break;
        bool quote = (*d == '"');
        if (quote) 
            ++d;
        if (d >= end)
            break;
        start = d;
        QByteArray value;
        while (d < end) {
            bool backslash = false;
            if (*d == '\\' && d < end - 1) {
                ++d;
                backslash = true;
            }
            if (!backslash) {
                if (quote) {
                    if (*d == '"')
                        break;
                } else {
                    if (*d == ',')
                        break;
                }
            }
            value += *d;
            ++d;
        }
        while (d < end && *d != ',')
            ++d;
        ++d;
        options[key] = value;
    }

    QByteArray qop = options.value("qop");
    if (!qop.isEmpty()) {
        QList<QByteArray> qopoptions = qop.split(',');
        if (!qopoptions.contains("auth"))
            return QHash<QByteArray, QByteArray>();
        // #### can't do auth-int currently
//         if (qop.contains("auth-int"))
//             qop = "auth-int";
//         else if (qop.contains("auth"))
//             qop = "auth";
//         else
//             qop = QByteArray();
        options["qop"] = "auth";
    }
    
    return options;
}

/*
  Digest MD5 implementation
  
  Code taken from RFC 2617

  Currently we don't support the full SASL authentication mechanism (which includes cyphers)
*/


/* calculate request-digest/response-digest as per HTTP Digest spec */
static QByteArray digestMd5Response(
    const QByteArray &alg,
    const QByteArray &userName,
    const QByteArray &realm,
    const QByteArray &password,
    const QByteArray &nonce,       /* nonce from server */
    const QByteArray &nonceCount,  /* 8 hex digits */
    const QByteArray &cNonce,      /* client nonce */
    const QByteArray &qop,         /* qop-value: "", "auth", "auth-int" */
    const QByteArray &method,      /* method from the request */
    const QByteArray &digestUri,   /* requested URL */
    const QByteArray &hEntity       /* H(entity body) if qop="auth-int" */
    )
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(userName);
    hash.addData(":", 1);
    hash.addData(realm);
    hash.addData(":", 1);
    hash.addData(password);
    QByteArray ha1 = hash.result();
    if (alg.toLower() == "md5-sess") {
        hash.reset();
        hash.addData(ha1);
        hash.addData(":", 1);
        hash.addData(nonce);
        hash.addData(":", 1);
        hash.addData(cNonce);
        ha1 = hash.result();
    };
    ha1 = ha1.toHex();
      
    // calculate H(A2)
    hash.reset();
    hash.addData(method);
    hash.addData(":", 1);
    hash.addData(digestUri);
    if (qop.toLower() == "auth-int") {
        hash.addData(":", 1);
        hash.addData(hEntity);
    }
    QByteArray ha2hex = hash.result().toHex();

    // calculate response
    hash.reset();
    hash.addData(ha1);
    hash.addData(":", 1);
    hash.addData(nonce);
    hash.addData(":", 1);
    if (!qop.isNull()) {
        hash.addData(nonceCount);
        hash.addData(":", 1);
        hash.addData(cNonce);
        hash.addData(":", 1);
        hash.addData(qop);
        hash.addData(":", 1);
    }
    hash.addData(ha2hex);
    return hash.result().toHex();
}

QByteArray QAuthenticatorPrivate::digestMd5Response(const QByteArray &challenge, const QByteArray &requestLine)
{
    QHash<QByteArray,QByteArray> options = parseDigestAuthenticationChallenge(challenge);

    ++nonceCount;
    QByteArray nonceCountString = QByteArray::number(nonceCount, 16);
    while (nonceCountString.length() < 8)
        nonceCountString.prepend('0');
    
    QList<QByteArray> tokens = requestLine.split(' ');
    QByteArray method = tokens.at(0);
    QByteArray path = tokens.at(1);

    QByteArray nonce = options.value("nonce");
    QByteArray opaque = options.value("opaque");
    QByteArray qop = options.value("qop");
    
    qDebug() << "calculating digest: method=" << method << "path=" << path;
    QByteArray response = ::digestMd5Response(options.value("algorithm"), user.toLatin1(),
                                              realm.toLatin1(), password.toLatin1(),
                                              nonce, nonceCountString,
                                              cnonce, qop, method,
                                              path, QByteArray());


    QByteArray credentials;
    credentials += "username=\"" + user.toLatin1() + "\", ";
    credentials += "realm=\"" + realm.toLatin1() + "\", ";
    credentials += "nonce=\"" + nonce + "\", ";
    credentials += "uri=\"" + path + "\", ";
    if (!opaque.isEmpty())
        credentials += "opaque=\"" + opaque + "\", ";
    credentials += "response=\"" + response + "\"";
    if (!options.value("qop").isEmpty()) {
        credentials += ", qop=" + qop + ", ";
        credentials += "nc=" + nonceCountString + ", ";
        credentials += "cnonce=\"" + cnonce + "\"";
    }

    return credentials;
}


void QAuthenticatorPrivate::parseHttpResponse(const QByteArray &httpResponse, bool isProxy, bool *passOk)
{
    QHttpResponseHeader header(QString::fromLatin1(httpResponse));
    QList<QPair<QString, QString> > values = header.values();
    const char *search = isProxy ? "proxy-authenticate" : "www-authenticate";

    bool first = (method == None);
    method = None;
    /*
      Fun from the HTTP 1.1 specs, that we currently ignore:

      User agents are advised to take special care in parsing the WWW-
      Authenticate field value as it might contain more than one challenge,
      or if more than one WWW-Authenticate header field is provided, the
      contents of a challenge itself can contain a comma-separated list of
      authentication parameters.
    */

    QString headerVal;
    for (int i = 0; i < values.size(); ++i) {
        const QPair<QString, QString> &current = values.at(i);
        if (current.first.toLower() != QLatin1String(search))
            continue;
        QString str = current.second;
        if (method < Basic && str.startsWith(QLatin1String("Basic"), Qt::CaseInsensitive)) {
            method = Basic; headerVal = str.mid(6);
        } else if (method < Ntlm && str.startsWith(QLatin1String("NTLM"), Qt::CaseInsensitive)) {
            method = Ntlm;
            headerVal = str.mid(5);
        } else if (method < DigestMd5 && str.startsWith(QLatin1String("Digest"), Qt::CaseInsensitive)) {
            method = DigestMd5;
            headerVal = str.mid(7);
        }
    }

    challenge = headerVal.toLatin1();
    QHash<QByteArray, QByteArray> options = parseDigestAuthenticationChallenge(challenge);

    switch(method) {
    case Basic:
        realm = QString::fromLatin1(options.value("realm"));
        *passOk = first;
        break;
    case Ntlm:
        // #####
        realm = QString();
    case DigestMd5: {
        realm = QString::fromLatin1(options.value("realm"));
        *passOk = first || (options.value("stale").toLower() == "true");
        break;
    }
    default:
        realm = QString();
        challenge = QByteArray();
    }
}
