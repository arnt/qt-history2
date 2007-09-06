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

#ifndef Patternist_EmptyIterator_H
#define Patternist_EmptyIterator_H

#include "Iterator.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short An Iterator which always is empty.
     *
     * EmptyIterator is an Iterator over the type @c T, which always is empty. Other
     * iterators can also be empty(or, at least behave as they are empty), but this
     * class is special designed for this purpose and is therefore fast.
     *
     * EmptyIterator's constructor is protected, instances is retrieved from CommonValues.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_iterators
     */
    template<typename T> class EmptyIterator : public Iterator<T>
    {
    public:
        /**
         * @returns always a default constructed value, T().
         */
        virtual T next()
        {
            return T();
        }

        /**
         * @returns always a default constructed value, T().
         */
        virtual T current() const
        {
            return T();
        }

        /**
         * @returns always 0.
         */
        virtual xsInteger position() const
        {
            return 0;
        }

        /**
         * @returns always Cardinality::empty()
         */
        virtual Cardinality cardinality()
        {
            return Cardinality::empty();
        }

        /**
         * @returns always @c this, the reverse of <tt>()</tt> is <tt>()</tt>.
         */
        virtual typename Iterator<T>::Ptr toReversed()
        {
            return typename Iterator<T>::Ptr(const_cast<EmptyIterator<T> *>(this));
        }

        /**
         * @returns always 0
         */
        virtual xsInteger count()
        {
            return 0;
        }

        /**
         * @returns @c this
         */
        virtual typename Iterator<T>::Ptr copy() const
        {
            return typename Iterator<T>::Ptr(const_cast<EmptyIterator *>(this));
        }

    protected:
        friend class CommonValues;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
