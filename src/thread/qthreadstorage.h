/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTHREADSTORAGE_H
#define QTHREADSTORAGE_H

#include "qglobal.h"

class Q_EXPORT QThreadStorageData
{
public:
    QThreadStorageData(void (*func)(void *));
    ~QThreadStorageData();

    void** get() const;
    void** set(void* p);

    static void finish(void**);
    int id;
    bool constructed;

    bool ensure_constructed(void (*func)(void *));
};


// pointer specialization
template <typename T>
static inline
T *&qThreadStorage_localData(QThreadStorageData &d, T **)
{
    void **v = d.get();
    if (!v) v = d.set(0);
    return *(reinterpret_cast<T**>(v));
}

template <typename T>
static inline
T *qThreadStorage_localData_const(const QThreadStorageData &d, T **)
{
    void **v = d.get();
    return v ? *(reinterpret_cast<T**>(v)) : 0;
}

template <typename T>
static inline
void qThreadStorage_setLocalData(QThreadStorageData &d, T **t)
{ (void) d.set(*t); }

#ifndef QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION

// value-based specialization
template <typename T>
static inline
T &qThreadStorage_localData(QThreadStorageData &d, T *)
{
    void **v = d.get();
    if (!v) v = d.set(new T());
    return *(reinterpret_cast<T*>(*v));
}

template <typename T>
static inline
T qThreadStorage_localData_const(const QThreadStorageData &d, T *)
{
    void **v = d.get();
    return v ? *(reinterpret_cast<T*>(*v)) : T();
}

template <typename T>
static inline
void qThreadStorage_setLocalData(QThreadStorageData &d, T *t)
{ (void) d.set(new T(*t)); }

#endif // QT_NO_PARTIAL_TEMPLATE_SPECIALIZATION

template <class T>
class QThreadStorage
{
private:
    QThreadStorageData d;

#if defined(Q_DISABLE_COPY)
    // disable copy constructor and operator=
    QThreadStorage(const QThreadStorage &);
    QThreadStorage &operator=(const QThreadStorage &);
#endif // Q_DISABLE_COPY

    static void deleteData(void *x) { delete reinterpret_cast<T*>(x); }

public:
    inline QThreadStorage() : d(deleteData) { }
    inline ~QThreadStorage() { }

    inline bool hasLocalData() const
    { return d.get() != 0; }

    inline T& localData()
    { return qThreadStorage_localData(d, reinterpret_cast<T*>(0)); }
    inline const T localData() const
    { return qThreadStorage_localData_const(d, reinterpret_cast<T*>(0)); }

    inline void setLocalData(T t)
    { qThreadStorage_setLocalData(d, &t); }

    inline bool ensure_constructed()
    { return d.ensure_constructed(deleteData); }
};

#endif // QTHREADSTORAGE_H
