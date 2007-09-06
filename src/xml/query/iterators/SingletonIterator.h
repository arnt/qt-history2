/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_SingletonIterator_H
#define Patternist_SingletonIterator_H

#include "Iterator.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short An Iterator over exactly one item.
     *
     * SingletonIterator's constructor takes one value which is
     * the item it forms an Iterator over. Other Iterator instances can
     * also form an Iterator with one in length, but by that SingletonIterator
     * has this as it only task, it means it is efficient at it.
     *
     * Having to represent single items in Iterators is relatively common.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_iterators
     */
    template<typename T>
    class SingletonIterator : public Iterator<T>
    {
    public:
        /**
         * Creates an iterator over @p item.
         *
         * @note item may not be @c null. Use the EmptyIterator for
         * the empty sequence
         */
        SingletonIterator(const T &item) : m_item(item),
                                           m_position(0)
        {
            Q_ASSERT(item);
        }

        virtual T next()
        {
            switch(m_position)
            {
                case 0:
                {
                    ++m_position;
                    return m_item;
                }
                case 1:
                {
                    m_position = -1;
                    return T();
                }
                default:
                {
                    Q_ASSERT(m_position == -1);
                    return T();
                }
            }
        }

        virtual T current() const
        {
            if(m_position == 1)
                return m_item;
            else
                return T();
        }

        virtual xsInteger position() const
        {
            return m_position;
        }

        /**
         * @returns always Cardinality::exactlyOne()
         */
        virtual Cardinality cardinality()
        {
            return Cardinality::exactlyOne();
        }

        /**
         * @returns a copy of this instance, rewinded to the beginning.
         */
        virtual typename Iterator<T>::Ptr toReversed()
        {
            return typename Iterator<T>::Ptr(new SingletonIterator<T>(m_item));
        }

        /**
         * @returns always 1
         */
        virtual xsInteger count()
        {
            return 1;
        }

        virtual typename Iterator<T>::Ptr copy() const
        {
            return typename Iterator<T>::Ptr(new SingletonIterator(m_item));
        }

    private:
        const T m_item;
        qint8 m_position;
    };

    /**
     * @short An object generator for SingletonIterator.
     *
     * makeSingletonIterator() is a convenience function for avoiding specifying
     * the full template instantiation for SingletonIterator. Conceptually, it
     * is identical to Qt's qMakePair().
     *
     * @relates SingletonIterator
     */
    template<typename T>
    inline
    typename SingletonIterator<T>::Ptr
    makeSingletonIterator(const T &item)
    {
        return typename SingletonIterator<T>::Ptr(new SingletonIterator<T>(item));
    }
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
