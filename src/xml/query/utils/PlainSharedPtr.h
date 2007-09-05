/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_PlainSharedPtr_H
#define Patternist_PlainSharedPtr_H

#include <QtGlobal>

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short A smart pointer to a QSharedData class which
     * never performs deep-copies.
     *
     * @ingroup Patternist
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename T> class PlainSharedPtr
    {
    public:
        inline PlainSharedPtr() : m_ptr(0) {}
        explicit inline PlainSharedPtr(T *ptr) : m_ptr(ptr)
        {
            if(m_ptr)
                m_ptr->ref.ref();
        }

        inline PlainSharedPtr(const PlainSharedPtr &o) : m_ptr(o.m_ptr)
        {
            if(m_ptr)
                m_ptr->ref.ref();
        }

        template<typename X>
        inline PlainSharedPtr(const PlainSharedPtr<X> &o) : m_ptr(static_cast<T *>(o.get()))
        {
            if(m_ptr)
                m_ptr->ref.ref();
        }

        inline ~PlainSharedPtr()
        {
            if(m_ptr && !m_ptr->ref.deref())
                delete m_ptr;
        }

        inline void reset()
        {
            if(m_ptr)
                m_ptr->ref.deref();
            m_ptr = 0;
        }

        inline T *get() const
        {
            return m_ptr;
        }

        inline T &operator*()
        {
            Q_ASSERT(m_ptr);
            return *m_ptr;
        }

        inline const T &operator*() const
        {
            Q_ASSERT(m_ptr);
            return *m_ptr;
        }

        inline T *operator->() const
        {
            Q_ASSERT(m_ptr);
            return m_ptr;
        }

        inline bool operator!() const
        {
            return m_ptr == 0;
        }

        inline operator bool() const
        {
            return m_ptr != 0;
        }

        inline bool operator==(const PlainSharedPtr &other) const
        {
            return m_ptr == other.m_ptr;
        }

        inline bool operator!=(const PlainSharedPtr &other) const
        {
            return m_ptr != other.m_ptr;
        }

        inline friend bool operator==(const PlainSharedPtr &a, const T *b)
        {
            return a.m_ptr == b;
        }

        inline friend bool operator==(const T *a, const PlainSharedPtr &b)
        {
            return a == b.m_ptr;
        }

        inline PlainSharedPtr<T> &operator=(const PlainSharedPtr<T> &o);

    private:
        T* m_ptr;
    };

    template<typename T>
    inline
    PlainSharedPtr<T> &PlainSharedPtr<T>::operator=(const PlainSharedPtr<T> &o)
    {
        if(o.m_ptr != m_ptr)
        {
            T *x = o.m_ptr;

            if(x)
                x->ref.ref();

            if(m_ptr && !m_ptr->ref.deref())
                delete m_ptr;

            m_ptr = x;
        }

        return *this;
    }
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
