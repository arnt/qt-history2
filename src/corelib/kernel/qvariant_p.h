/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOREVARIANT_P_H
#define QCOREVARIANT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


// takes a type, returns the internal void* pointer casted
// to a pointer of the input type

#ifdef Q_CC_SUN // Sun CC piks the wrong overload, so introduce awful hack

template <typename T>
inline T *v_cast(const QVariant::Private *nd, T * = 0)
{
    QVariant::Private *d = const_cast<QVariant::Private *>(nd);
    return ((sizeof(T) > sizeof(QVariant::Private::Data))
            // this is really a static_cast, but gcc 2.95 complains about it.
            ? reinterpret_cast<T*>(d->data.shared->ptr)
            : reinterpret_cast<T*>(&d->data.ptr));
}

#else // every other compiler in this world

template <typename T>
inline const T *v_cast(const QVariant::Private *d, T * = 0)
{
    return ((sizeof(T) > sizeof(QVariant::Private::Data))
            // this is really a static_cast, but gcc 2.95 complains about it.
            ? reinterpret_cast<const T*>(d->data.shared->ptr)
            : reinterpret_cast<const T*>(&d->data.ptr));
}

template <typename T>
inline T *v_cast(QVariant::Private *d, T * = 0)
{
    return ((sizeof(T) > sizeof(QVariant::Private::Data))
            // this is really a static_cast, but gcc 2.95 complains about it.
            ? reinterpret_cast<T*>(d->data.shared->ptr)
            : reinterpret_cast<T*>(&d->data.ptr));
}

#endif

// constructs a new variant if copy is 0, otherwise copy-constructs
template <class T>
inline void v_construct(QVariant::Private *x, const void *copy, T * = 0)
{
    if (sizeof(T) > sizeof(QVariant::Private::Data)) {
        x->data.shared = copy ? new QVariant::PrivateShared(new T(*static_cast<const T *>(copy)))
                              : new QVariant::PrivateShared(new T);
        x->is_shared = true;
    } else {
        if (copy)
            new (&x->data.ptr) T(*static_cast<const T *>(copy));
        else
            new (&x->data.ptr) T;
    }
}

// deletes the internal structures
template <class T>
inline void v_clear(QVariant::Private *d, T* = 0)
{
    if (sizeof(T) > sizeof(QVariant::Private::Data)) {
        delete v_cast<T>(d);
        delete d->data.shared;
    } else {
        reinterpret_cast<T *>(&d->data.ptr)->~T();
    }
}

#endif

