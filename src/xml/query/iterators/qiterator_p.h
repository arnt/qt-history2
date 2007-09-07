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
****************************************************************************/

#ifndef Patternist_Iterator_H
#define Patternist_Iterator_H

template<typename T> class QVector;
#include <QList>
#include <QSharedData>

#include "Cardinality.h"
#include "Primitives.h"
#include "PlainSharedPtr.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    template<typename T> class ListIterator;

    /**
     * @short Callback Iterator uses for deteriming whether an item is the end
     * of a sequence.
     *
     * This implementation works for all types that has a boolean operator. In other words,
     * this function should work satisfactory for pointers, for example.
     */
    template<typename T>
    inline bool isIteratorEnd(const T &unit)
    {
        return !unit;
    }

    /**
     * @short Base class for all Iterator classes, the primary way to represent
     * data.
     *
     * In order for an item to be able to be instantiated in Iterator, it needs to:
     * - have a copy constructor, default constructor, and an assignment operator
     * - An appropriate isIteratorEnd() function
     *
     * @ingroup Patternist_iterators
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename T> class Iterator : public QSharedData
    {
    public:
        /**
         * A smart pointer wrapping an Item instance.
         */
        typedef PlainSharedPtr<Iterator<T> > Ptr;

        /**
         * A list of Iterator instances, each wrapped in a smart pointer.
         */
        typedef QList<typename Iterator<T>::Ptr> List;

        /**
         * A vector of Iterator instances, each wrapped in a smart pointer.
         */
        typedef QVector<typename Iterator<T>::Ptr> Vector;

        inline Iterator() {}
        virtual ~Iterator() {}

        /**
         * @returns the next item in the sequence, or
         * @c null if the end has been reached.
         */
        virtual T next() = 0;

        /**
         * @returns the current item in the sequence. If it is called before any call
         * to next(), @c null is returned. If the end of the sequence have been reached @c null.
         */
        virtual T current() const = 0;

        /**
         * @short Returns the current position in the item sequence this
         * Iterator represents.
         *
         * position() may be called several times for the same Iterator, so the
         * implementation of position() should preferrably be constant in
         * execution time.
         *
         * The first position is 1, not 0. If next() hasn't been called, 0 is returned. If
         * the end of this Iterator has been reached, -1 is returned.
         *
         * @returns the current position
         */
        virtual xsInteger position() const = 0;

        /**
         * Determines the amount of items this Iterator represents.
         *
         * Note that this function is not @c const, it modifies the Iterator
         * as opposed to for example Qt's container classes functions by the same name. The
         * reason for this is efficiency. If this Iterator should stay intact, one should
         * use a copy().
         *
         * The default implementation simply calls next() until the end is reached. Hence, it may
         * be of interest to override this function if the sub-class knows a better way of computing
         * its count.
         *
         * @returns the size of the sequence, the number of items in the sequence.
         */
        virtual xsInteger count();

        /**
         * Determines the cardinality of the sequence the Iterator this Iterator iterates over. The
         * returned cardinality must not be a union but an exact cardinality; for example,
         * Cardinality::empty() is but Cardinality::zeroOrMore() isn't. Thus, valid return
         * values are Cardinality::empty(), Cardinality::exactlyOne(), and Cardinality::twoOrMore.
         *
         * The default implementation calls next() for determining the cardinality.
         * @note This function may modify the iterator, it can be considered a function
         * that evaluates this Iterator. It is not a getter, but potentially alters
         * the iterator in the same way the next() function does. If this Iterator should
         * stay intact, such that it can be used for evaluation with next(), one should
         * first copy this Iterator with the copy() function.
         */
        virtual Cardinality cardinality();

        /**
         * Retrieves an iterator working in a reverse direction over the sequence.
         *
         * @note This function may modify the iterator, it can be considered a function
         * that evaluates this Iterator. It is not a getter, but potentially alters
         * the iterator in the same way the next() function does. If this Iterator should
         * stay intact, such that it can be used for evaluation with next(), one should
         * first copy this Iterator with the copy() function.
         *
         * @see <a href="http://www.w3.org/TR/xpath-functions/#func-reverse">XQuery
         * 1.0 and XPath 2.0 Functions and Operators, 15.1.9 fn:reverse</a>
         * @returns an iterator that iterates the sequence this iterator
         * represents, in reverse order.
         */
        virtual typename Iterator<T>::Ptr toReversed();

        /**
         * Performs a copy of this Iterator(with copy()), and returns its items
         * in a QList. Thus, this function acts as a conversion function, by allowing
         * a sequence of items to be converted into a QList, instead of being represented
         * by an Iterator.
         *
         * @note This function may modify the iterator, it can be considered a function
         * that evaluates this Iterator. It is not a getter, but potentially alters
         * the iterator in the same way the next() function does. If this Iterator should
         * stay intact, such that it can be used for evaluation with next(), one should
         * first copy this Iterator with the copy() function.
         */
        virtual QList<T> toList();

        /**
         * @short Copies this Iterator and returns the copy.
         *
         * A copy and the original instance are completely independent of each other, and
         * doesn't affect each other in anyway. Since evaluating an Iterator modifies it
         * one should always use a copy when an Iterator needs to be used several times.
         *
         * @returns a copy of this Iterator.
         */
        virtual typename Iterator<T>::Ptr copy() const = 0;

    private:
        Q_DISABLE_COPY(Iterator<T>)
    };

    template<typename T>
    QList<T> Iterator<T>::toList()
    {
        QList<T> result;
        T item(next());

        while(!isIteratorEnd(item))
        {
            result.append(item);
            item = next();
        }

        return result;
    }

    template<typename T>
    xsInteger Iterator<T>::count()
    {
        xsInteger retval = 0;

        while(!isIteratorEnd(next()))
            ++retval;

        return retval;
    }

    template<typename T>
    Cardinality Iterator<T>::cardinality()
    {
        T unit(next());

        if(isIteratorEnd(unit))
            return Cardinality::empty();

        unit = next();

        if(isIteratorEnd(unit))
            return Cardinality::exactlyOne();
        else
            return Cardinality::twoOrMore();
    }

    template<typename T>
    typename Iterator<T>::Ptr Iterator<T>::toReversed()
    {
        T item(next());
        QList<T> result;

        while(!isIteratorEnd(item))
        {
            result.prepend(item);
            item = next();
        }

        return typename Iterator<T>::Ptr(new ListIterator<T>(result));
    }
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
