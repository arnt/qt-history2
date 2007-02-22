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

#ifndef QCRYPTOGRAPHICSHASH_H
#define QCRYPTOGRAPHICSHASH_H

#include <QtCore/qbytearray.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

class QCryptographicHashPrivate;

class Q_CORE_EXPORT QCryptographicHash
{
public:
    enum HashMethod {
        Md4,
        Md5
    };

    QCryptographicHash(HashMethod method);
    ~QCryptographicHash();

    void reset();

    void addData(const char *data, int length);
    void addData(const QByteArray &data);

    QByteArray result() const;

    static QByteArray hash(const QByteArray &data, HashMethod method);
private:
    Q_DISABLE_COPY(QCryptographicHash);
    QCryptographicHashPrivate *d;
};

QT_END_HEADER

#endif
