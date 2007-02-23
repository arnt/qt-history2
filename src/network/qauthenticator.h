#ifndef QAUTHENTICATOR_H
#define QAUTHENTICATOR_H

#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Network)

class QAuthenticatorPrivate;
class QUrl;

class Q_NETWORK_EXPORT QAuthenticator
{
public:
    QAuthenticator();
    ~QAuthenticator();

    QAuthenticator(const QAuthenticator &other);
    QAuthenticator &operator=(const QAuthenticator &other);

    QString user() const;
    void setUser(const QString &user);

    QString password() const;
    void setPassword(const QString &password);

    bool isNull() const;
    void detach();
private:
    friend class QAuthenticatorPrivate;
    QAuthenticatorPrivate *d;
};


QT_END_HEADER

#endif
