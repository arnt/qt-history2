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

    bool operator==(const QAuthenticator &other) const;
    inline bool operator!=(const QAuthenticator &other) const { return !operator==(other); }
    
    QString user() const;
    void setUser(const QString &user);

    QString password() const;
    void setPassword(const QString &password);

    QString realm() const;

    bool isNull() const;
    void detach();
private:
    friend class QAuthenticatorPrivate;
    QAuthenticatorPrivate *d;
};

QT_END_HEADER

#endif
