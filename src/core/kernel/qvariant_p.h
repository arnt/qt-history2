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
template <typename T>
inline static const T *v_cast(const QVariant::Private *d, T * = 0)
{
    return ((sizeof(T) > sizeof(QVariant::Private::Data))
            // this is really a static_cast, but gcc 2.95 complains about it.
            ? reinterpret_cast<const T*>(d->data.shared->ptr)
            : reinterpret_cast<const T*>(&d->data.ptr));
}

template <typename T>
inline static T *v_cast(QVariant::Private *d, T * = 0)
{
    return ((sizeof(T) > sizeof(QVariant::Private::Data))
            // this is really a static_cast, but gcc 2.95 complains about it.
            ? reinterpret_cast<T*>(d->data.shared->ptr)
            : reinterpret_cast<T*>(&d->data.ptr));
}

// constructs an empty variant
template <class T>
inline static void v_construct(QVariant::Private *x, T* = 0)
{
    if (sizeof(T) > sizeof(QVariant::Private::Data)) {
        x->data.shared = new QVariant::PrivateShared(new T);
        x->is_shared = true;
    } else {
        new (&x->data.ptr) T;
    }
}

// copy-constructs a new variant
template <class T>
inline static void v_construct(QVariant::Private *x, const void *copy, T * = 0)
{
    if (sizeof(T) > sizeof(QVariant::Private::Data)) {
        x->data.shared = new QVariant::PrivateShared(new T(*static_cast<const T *>(copy)));
        x->is_shared = true;
    } else {
        new (&x->data.ptr) T(*static_cast<const T *>(copy));
    }
}

// deletes the internal structures
template <class T>
inline static void v_clear(QVariant::Private *d, T* = 0)
{
    if (sizeof(T) > sizeof(QVariant::Private::Data)) {
        delete v_cast<T>(d);
        delete d->data.shared;
    } else {
        reinterpret_cast<T *>(&d->data.ptr)->~T();
    }
}

#endif

