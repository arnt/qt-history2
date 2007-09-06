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

#include <QStringList>

#include "Boolean.h"
#include "CommonValues.h"
#include "Debug.h"
#include "ItemMappingIterator.h"
#include "ListIterator.h"
#include "PatternistLocale.h"
#include "AtomicString.h"

#include "PatternMatchingFNs.h"

using namespace Patternist;

MatchesFN::MatchesFN() : PatternPlatform(2)
{
}

Item MatchesFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    const QRegExp regexp(pattern(context));
    QString input;

    const Item arg(m_operands.first()->evaluateSingleton(context));
    if(arg)
        input = arg.stringValue();

    qDebug() << "Input string: " << input;
    return Boolean::fromValue(input.contains(regexp));
}

ReplaceFN::ReplaceFN() : PatternPlatform(3)
{
}

Item ReplaceFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    const QRegExp regexp(pattern(context));
    QString input;

    const Item arg(m_operands.first()->evaluateSingleton(context));
    if(arg)
        input = arg.stringValue();

    qDebug() << "Input string: " << input;
    const QString replacement(m_replacementString.isNull() ? parseReplacement(regexp.numCaptures(), context)
                                                           : m_replacementString);

    qDebug() << "Replacement string: " << replacement;

    return AtomicString::fromValue(input.replace(regexp, replacement));
}

QString ReplaceFN::errorAtEnd(const char ch)
{
    return tr("%1 must be followed by %2 or %3, it cannot "
                "occur at the end of the replacement string.")
                .arg(formatKeyword(QLatin1Char(ch)))
                .arg(formatKeyword(QLatin1Char('\\')))
                .arg(formatKeyword(QLatin1Char('$')));
}

QString ReplaceFN::parseReplacement(const int,
                                    const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    // TODO what if there is no groups, can one rewrite to the replacement then?
    const QString input(m_operands.at(2)->evaluateSingleton(context).stringValue());
    qDebug() << "Input string: " << input;

    QString retval;
    retval.reserve(input.size());
    const int len = input.length();

    for(int i = 0; i < len; ++i)
    {
        const QChar ch(input.at(i));
        switch(ch.toAscii())
        {
            case '$':
            {
                /* QRegExp uses '\' as opposed to '$' for marking sub groups. */
                retval.append(QLatin1Char('\\'));

                ++i;
                if(i == len)
                {
                    context->error(errorAtEnd('$'), ReportContext::FORX0004, this);
                    return QString();
                }

                const QChar nextCh(input.at(i));
                if(nextCh.isDigit())
                    retval.append(nextCh);
                else
                {
                    context->error(tr("In the replacement string, %1 must be "
                                                    "followed by at least one digit when not escaped.")
                                                    .arg(formatKeyword(QLatin1Char('$'))),
                                               ReportContext::FORX0004, this);
                    return QString();
                }

                break;
            }
            case '\\':
            {
                ++i;
                if(i == len)
                {
                    /* error, we've reached the end. */;
                    context->error(errorAtEnd('\\'), ReportContext::FORX0004, this);
                }

                const QChar nextCh(input.at(i));
                if(nextCh == QLatin1Char('\\') || nextCh == QLatin1Char('$'))
                    retval.append(ch);
                else
                {
                    context->error(tr("In the replacement string, %1 can only be used "
                                                    "escape itself or %2, not %3")
                                                    .arg(formatKeyword(QLatin1Char('\\')))
                                                    .arg(formatKeyword(QLatin1Char('$')))
                                                    .arg(formatKeyword(nextCh)),
                                               ReportContext::FORX0004, this);
                    return QString();
                }

                break;
            }
            default:
                retval.append(ch);
        }
    }

    return retval;
}

Expression::Ptr ReplaceFN::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr me(PatternPlatform::compress(context));

    if(me.get() != this)
        return me;

    if(m_operands.at(2)->is(IDStringValue))
    {
        const int capt = captureCount();
        if(capt == -1)
            return me;
        else
            m_replacementString = parseReplacement(captureCount(), context->dynamicContext());
    }

    return me;
}

TokenizeFN::TokenizeFN() : PatternPlatform(2)
{
}

/**
 * Used by Iterator.
 */
inline bool isIteratorEnd(const QString &item)
{
    return item.isNull();
}

Item TokenizeFN::mapToItem(const QString &subject, const DynamicContext::Ptr &) const
{
    return AtomicString::fromValue(subject);
}

Item::Iterator::Ptr TokenizeFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    const Item arg(m_operands.first()->evaluateSingleton(context));
    if(!arg)
        return CommonValues::emptyIterator;

    const QString input(arg.stringValue());
    if(input.isEmpty())
        return CommonValues::emptyIterator;

    const QRegExp regExp(pattern(context));
    const QStringList result(input.split(regExp, QString::KeepEmptyParts));

    return makeItemMappingIterator<Item>(TokenizeFN::Ptr(const_cast<TokenizeFN *>(this)),
                                                              makeListIterator(result),
                                                              DynamicContext::Ptr());
}

// vim: et:ts=4:sw=4:sts=4
