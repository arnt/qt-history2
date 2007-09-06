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

#ifndef Patternist_ItemMappingIterator_H
#define Patternist_ItemMappingIterator_H

#include "Iterator.h"
#include "DynamicContext.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Proxies another Iterator, and for each item, returns the
     * Item returned from a mapping function.
     *
     * ItemMappingIterator is practical when the items in an Iterator needs to
     * be translated to another sequence, while still doing it in a pipe-lined
     * fashion.
     *
     * This is achieved by that ItemMappingIterator's constructor takes
     * an instance of a class, that must have the following member:
     *
     * @code
     * TResult::Ptr mapToItem(const TSource &item,
     *                        const DynamicContext::Ptr &context) const
     * @endcode
     *
     * For each item in the Iterator ItemMappingIterator proxies, this function is
     * called and its return value becomes the return value of the ItemMappingIterator. If the
     * mapping function returns null, ItemMappingIterator maps the next item in the source sequence
     * such that a contiguous sequence of items is returned.
     *
     * Declaring the mapToItem() function as inline, can be a good way to improve performance.
     *
     * @see SequenceMappingIterator
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_iterators
     */
    template<typename TResult, typename TSource, typename TMapper>
    class ItemMappingIterator : public Iterator<TResult>
    {
    public:
        /**
         * Constructs an ItemMappingIterator.
         *
         * @param mapper the object that has the mapToItem() sequence.
         * @param iterator the Iterator whose items should be mapped.
         * @param context the DynamicContext that will be passed to the map function.
         * May be null.
         */
        ItemMappingIterator(const TMapper &mapper,
                            const typename Iterator<TSource>::Ptr &iterator,
                            const DynamicContext::Ptr &context) : m_mapper(mapper),
                                                                  m_it(iterator),
                                                                  m_context(context),
                                                                  m_position(0)
        {
            Q_ASSERT(mapper);
            Q_ASSERT(iterator);
        }

        /**
         * @returns the next item in the sequence, or
         * @c null if the end have been reached.
         */
        virtual TResult next()
        {
            const TSource sourceItem(m_it->next());

            if(isIteratorEnd(sourceItem))
            {
                m_current = TResult();
                m_position = -1;
                return TResult();
            }
            else
            {
                m_current = m_mapper->mapToItem(sourceItem, m_context);
                if(isIteratorEnd(m_current))
                    return next(); /* The mapper returned null, so continue with the next in the source. */
                else
                {
                    ++m_position;
                    return m_current;
                }
            }
        }

        virtual TResult current() const
        {
            return m_current;
        }

        virtual xsInteger position() const
        {
            return m_position;
        }

        virtual typename Iterator<TResult>::Ptr copy() const
        {
            return typename Iterator<TResult>::Ptr
                (new ItemMappingIterator<TResult, TSource, TMapper>(m_mapper, m_it->copy(), m_context));
        }

    private:
        const TMapper                           m_mapper;
        const typename Iterator<TSource>::Ptr   m_it;
        const DynamicContext::Ptr               m_context;
        TResult                                 m_current;
        xsInteger                               m_position;
    };

    /**
     * @short An object generator for ItemMappingIterator.
     *
     * makeItemMappingIterator() is a convenience function for avoiding specifying
     * the full template instantiation for ItemMappingIterator. Conceptually, it
     * is identical to Qt's qMakePair().
     *
     * @relates ItemMappingIterator
     */
    template<typename TResult, typename TSource, typename TMapper>
    static inline
    typename Iterator<TResult>::Ptr
    makeItemMappingIterator(const TMapper &mapper,
                            const PlainSharedPtr<Iterator<TSource> > &source,
                            const DynamicContext::Ptr &context)
    {
        return typename Iterator<TResult>::Ptr
            (new ItemMappingIterator<TResult, TSource, TMapper>(mapper, source, context));
    }
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
