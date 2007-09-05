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

#include "CommonValues.h"
#include "ListIterator.h"
#include "PatternistLocale.h"
#include "Numeric.h"
#include "AtomicString.h"
#include "ToCodepointsIterator.h"

#include "AssembleStringFNs.h"

using namespace Patternist;

/*
 * Determines whether @p cp is a valid XML 1.0 character.
 *
 * @see <a href="http://www.w3.org/TR/REC-xml/#charsets">Extensible Markup
 * Language (XML) 1.0 (Third Edition)2.2 Characters</a>
 */
static inline bool isValidXML10Char(const qint32 cp)
{
    /* [2]     Char     ::=     #x9 | #xA | #xD | [#x20-#xD7FF] |
     *                          [#xE000-#xFFFD] | [#x10000-#x10FFFF]
     */
    return (cp == 0x9                       ||
            cp == 0xA                       ||
            cp == 0xD                       ||
            (0x20 <= cp && cp <= 0xD7FF)    ||
            (0xE000 <= cp && cp <= 0xFFFD)  ||
            (0x10000 <= cp && cp <= 0x10FFFF));
}

Item CodepointsToStringFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr it(m_operands.first()->evaluateSequence(context));

    if(!it)
        return CommonValues::EmptyString;

    QString retval;
    Item item(it->next());
    while(item)
    {
        const qint32 cp = static_cast<qint32>(item.as<Numeric>()->toInteger());

        if(!isValidXML10Char(cp))
        {
            context->error(tr("%1 is not a valid XML 1.0 character.")
                                            .arg(formatData(QLatin1String("0x") +
                                                          QString::number(cp, 16))),
                                       ReportContext::FOCH0001, this);

            return CommonValues::EmptyString;
        }
        retval.append(QChar(cp));
        item = it->next();
    }

    return AtomicString::fromValue(retval);
}

Item::Iterator::Ptr StringToCodepointsFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const Item item(m_operands.first()->evaluateSingleton(context));
    if(!item)
        return CommonValues::emptyIterator;

    const QString str(item.stringValue());
    if(str.isEmpty())
        return CommonValues::emptyIterator;
    else
        return Item::Iterator::Ptr(new ToCodepointsIterator(str));
}

// vim: et:ts=4:sw=4:sts=4
