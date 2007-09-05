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

#ifndef Patternist_ListIterator_H
#define Patternist_ListIterator_H

#include <QtGlobal>
#include <QList>

#include "Iterator.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Helper class for ListIterator, and should only
     * be instantiated through sub-classing.
     *
     * ListIteratorPlatform iterates an InputList with instances
     * of InputType. For every item in it, it returns an item from it,
     * that is converted to OutputType by calling a function on Derived
     * that has the following signature:
     *
     * @todo Document why this class doesn't duplicate ItemMappingIterator,
     * more or less.
     *
     * @code
     * OutputType inputToOutputItem(const InputType &inputType) const;
     * @endcode
     *
     * @ingroup Patternist_iterators
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename InputType,
             typename OutputType,
             typename Derived>
    class ListIteratorPlatform : public Iterator<OutputType>
    {
        typedef QList<InputType> InputList;
    public:
        /**
         * @returns the next item in the sequence, or
         * @c null if the end have been reached.
         */
        virtual OutputType next()
        {
            if(m_position == -1)
                return OutputType();

            if(m_position == m_list.count())
            {
                m_position = -1;
                m_current = OutputType();
                return OutputType();
            }

            m_current = static_cast<const Derived *const>(this)->inputToOutputItem(m_list.at(m_position));
            ++m_position;
            return m_current;
        }

        virtual OutputType current() const
        {
            return m_current;
        }

        virtual xsInteger position() const
        {
            return m_position;
        }

        virtual xsInteger count()
        {
            return m_list.count();
        }

        virtual Cardinality cardinality()
        {
            return Cardinality::fromCount(m_list.count());
        }

        virtual typename Iterator<OutputType>::Ptr copy() const
        {
            return typename Iterator<OutputType>::Ptr(new ListIteratorPlatform<InputType, OutputType, Derived>(m_list));
        }

    protected:
        /**
         * Creates a ListIteratorPlatform that walks @p list.
         */
        inline ListIteratorPlatform(const InputList &list) : m_list(list)
                                                           , m_position(0)
        {
        }

        const InputList m_list;

    private:
        xsInteger   m_position;
        OutputType  m_current;
    };

    /**
     * @short Bridges values in Qt's QList container class into an Iterator.
     *
     * ListIterator takes a reference to a QList<T> instance, and allows access
     * to that list via its Iterator interface. ListIterator is parameterized on
     * one argument, the type to iterate upon, such as Item or Expression::Ptr.
     *
     * ListIterator is for example used by the ExpressionSequence, to create an iterator
     * over its operandsListIteratorHelperExpression::Ptr instances) that is subsequently passed to
     * an MappingIterator.
     *
     * @ingroup Patternist_iterators
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename T>
    class ListIterator : public ListIteratorPlatform<T, T, ListIterator<T> >
    {
        using ListIteratorPlatform<T, T, ListIterator<T> >::m_list;

    public:
        inline ListIterator(const QList<T> &list) : ListIteratorPlatform<T, T, ListIterator<T> >(list)
        {
        }

        /**
         * Overriden for efficiency. Since ListIteratorPlatform internally already stores
         * a QList it can return that instance instead of letting Iterator's generic
         * implementation create one.
         */
        virtual QList<T> toList()
        {
            return m_list;
        }

    private:
        inline T inputToOutputItem(const T &inputType) const
        {
            return inputType;
        }

        friend class ListIteratorPlatform<T, T, ListIterator<T> >;
    };

    /**
     * @short An object generator for ListIterator.
     *
     * makeListIterator() is a convenience function for avoiding specifying
     * the full template instantiation for ListIterator. Conceptually, it
     * is identical to Qt's qMakePair().
     *
     * @relates ListIterator
     */
    template<typename T>
    inline
    typename ListIterator<T>::Ptr
    makeListIterator(const QList<T> &qList)
    {
        return typename ListIterator<T>::Ptr(new ListIterator<T>(qList));
    }
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
