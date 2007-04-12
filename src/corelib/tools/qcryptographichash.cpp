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

#include <qcryptographichash.h>

#include "../../3rdparty/md5/md5.h"
#include "../../3rdparty/md5/md5.cpp"
#include "../../3rdparty/md4/md4.h"
#include "../../3rdparty/md4/md4.cpp"
#include "../../3rdparty/sha1/sha1.cpp"

class QCryptographicHashPrivate
{
public:
    QCryptographicHash::Algorithm method;
    union {
        MD5Context md5Context;
        md4_context md4Context;
        Sha1State sha1Context;
    };
};

/*!
  \class QCryptographicHash

  \brief The QCryptographicHash class provides a way to generate cryptographic hashes.

  \since 4.3

  \ingroup tools
  \reentrant

  QCryptographicHash can be used to generate cryptographic hashes of binary or text data.
  
  Currently Md4 and Md5 are supported.
*/

/*!
  \enum QCryptographicHash::Algorithm

  \value Md4 Generate an Md4 hash sum
  \value Md5 Generate an Md5 hash sum
  \value Sha1 Generate an Sha1 hash sum
*/

/*!
  Constructs an object that can be used to create a cryptographic hash from data using \a method.
*/
QCryptographicHash::QCryptographicHash(Algorithm method)
    : d(new QCryptographicHashPrivate)
{
    d->method = method;
    reset();
}

/*!
  Destroys the object.
*/
QCryptographicHash::~QCryptographicHash()
{
    delete d;
}

/*!
  Resets the object.
*/
void QCryptographicHash::reset()
{
    switch (d->method) {
    case Md4:
        md4_init(&d->md4Context);
        break;
    case Md5:
        MD5Init(&d->md5Context);
        break;
    case Sha1:
        sha1InitState(&d->sha1Context);
        break;
    }
}

/*!
    Adds the first \a length chars of \a data to the cryptographic
    hash.
*/
void QCryptographicHash::addData(const char *data, int length)
{
    switch (d->method) {
    case Md4:
        md4_update(&d->md4Context, (const unsigned char *)data, length);
        break;
    case Md5:
        MD5Update(&d->md5Context, (const unsigned char *)data, length);
        break;
    case Sha1:
        sha1Update(&d->sha1Context, (const unsigned char *)data, length);
        break;
    }    
}

/*!
  /overload
*/
void QCryptographicHash::addData(const QByteArray &data)
{
    addData(data.constData(), data.length());
}

/*!
  Returns the final hash value.
*/
QByteArray QCryptographicHash::result() const
{
    QByteArray result;
    switch (d->method) {
    case Md4:
        result.resize(MD4_RESULTLEN);
        md4_final(&d->md4Context, (unsigned char *)result.data());
        break;
    case Md5:
        result.resize(16);
        MD5Final(&d->md5Context, (unsigned char *)result.data());
        break;
    case Sha1:
        result.resize(20);
        sha1FinalizeState(&d->sha1Context);
        sha1ToHash(&d->sha1Context, (unsigned char *)result.data());
    }
    return result;
}

/*!
  Returns the hash of \a data using \a method.
*/
QByteArray QCryptographicHash::hash(const QByteArray &data, Algorithm method)
{
    QCryptographicHash hash(method);
    hash.addData(data);
    return hash.result();
}
