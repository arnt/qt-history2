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

#include "CommonSequenceTypes.h"
#include "CommonValues.h"
#include "ItemMappingIterator.h"
#include "ListIterator.h"
#include "PatternistLocale.h"

#include "TraceFN.h"

using namespace Patternist;

/**
 * @short TraceCallback is a MappingCallback and takes care of
 * the tracing of each individual item.
 *
 * Because Patternist must be thread safe, TraceFN creates a TraceCallback
 * each time the function is evaluated. In other words, TraceFN, which is
 * an Expression sub class, can't modify its members, but MappingCallback
 * does not have this limitation since it's created on a per evaluation basis.
 *
 * @author Frans Englich <fenglich@trolltech.com>
 */
class TraceCallback : public QSharedData
{
public:
    typedef PlainSharedPtr<TraceCallback> Ptr;

    inline TraceCallback(const QString &msg) : m_position(0),
                                               m_msg(msg)
    {
    }

    /**
     * Performs the actual tracing.
     */
    Item mapToItem(const Item &item,
                        const DynamicContext::Ptr &context)
    {
        QTextStream out(stderr);
        ++m_position;
        if(m_position == 1)
        {
            if(item)
            {
                out << qPrintable(m_msg)
                    << " : "
                    << qPrintable(item.stringValue());
            }
            else
            {
                out << qPrintable(m_msg)
                    << " : ("
                    << qPrintable(formatType(context->namePool(), CommonSequenceTypes::Empty))
                    << ")\n";
                return Item();
            }
        }
        else
        {
            out << qPrintable(item.stringValue())
                << '['
                << m_position
                << "]\n";
        }

        return item;
    }

private:
    xsInteger m_position;
    const QString m_msg;
};

Item::Iterator::Ptr TraceFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const QString msg(m_operands.last()->evaluateSingleton(context).stringValue());

    return makeItemMappingIterator<Item>(TraceCallback::Ptr(new TraceCallback(msg)),
                                              m_operands.first()->evaluateSequence(context),
                                              context);
}

Item TraceFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const QString msg(m_operands.last()->evaluateSingleton(context).stringValue());
    const Item item(m_operands.first()->evaluateSingleton(context));

    return TraceCallback::Ptr(new TraceCallback(msg))->mapToItem(item, context);
}

SequenceType::Ptr TraceFN::staticType() const
{
    return m_operands.first()->staticType();
}

// vim: et:ts=4:sw=4:sts=4
