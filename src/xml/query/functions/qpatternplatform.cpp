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

#include <QHash>

#include "qdebug_p.h"
#include "qpatternistlocale_p.h"

#include "qpatternplatform_p.h"

using namespace Patternist;

namespace Patternist
{
    /**
     * @short Used internally by PatternPlatform and describes
     * a flag that affects how a pattern is treated.
     *
     * The member variables aren't declared @c const, in order
     * to make the synthesized assignment operator and copy constructor work.
     *
     * @ingroup Patternist_utils
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class PatternFlag
    {
    public:
        typedef QHash<QChar, PatternFlag> Hash;

        inline PatternFlag() : flag(PatternPlatform::NoFlags)
        {
        }

        inline PatternFlag(const PatternPlatform::Flag opt,
                           const QString &descr) : flag(opt),
                                                   description(descr)
        {
        }

        PatternPlatform::Flag   flag;
        QString                 description;
    };
}

static inline PatternFlag::Hash initDescriptions()
{
    qDebug() << Q_FUNC_INFO;
    PatternFlag::Hash retval;

    retval.insert(QChar(QLatin1Char('s')),
                  PatternFlag(PatternPlatform::DotAllMode,
                              tr("%1 matches newline characters").arg(formatKeyword(QLatin1Char('.')))));

    retval.insert(QChar(QLatin1Char('m')),
                  PatternFlag(PatternPlatform::MultiLineMode,
                              tr("%1 and %2 matches the start and end of any line, respectively")
                                   .arg(formatKeyword(QLatin1Char('^')))
                                   .arg(formatKeyword(QLatin1Char('$')))));

    retval.insert(QChar(QLatin1Char('i')),
                  PatternFlag(PatternPlatform::CaseInsensitive,
                              tr("Matches are case insensitive")));

    retval.insert(QChar(QLatin1Char('x')),
                  PatternFlag(PatternPlatform::SimplifyWhitespace,
                              tr("Whitespace characters are removed, except when appearing "
                                 "in character classes")));

    return retval;
}

Q_GLOBAL_STATIC_WITH_ARGS(PatternFlag::Hash, flagDescriptions, (initDescriptions()))

PatternPlatform::PatternPlatform(const qint8 flagsPosition) : m_compiledParts(NoPart),
                                                              m_flags(NoFlags),
                                                              m_flagsPosition(flagsPosition)
{
}

const QRegExp PatternPlatform::pattern(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "m_compiledParts: " << m_compiledParts;
    if(m_compiledParts == FlagsAndPattern) /* This is the most common case. */
    {
        qDebug() << "Returning fully complete pattern.";
        Q_ASSERT(m_pattern.isValid());
        return m_pattern;
    }

    QRegExp retvalPattern;
    Flags flags;

    /* Compile the flags, if necessary. */
    if((m_compiledParts & OnlyPattern) == 0)
    {
        retvalPattern = m_pattern; /* Pattern is already compiled, so use it. */
        qDebug() << "Compiling flags..";

        const Expression::Ptr flagsOp(m_operands.value(m_flagsPosition));

        if(flagsOp)
            flags = parseFlags(flagsOp->evaluateSingleton(context).stringValue(), context);
        else
            flags = NoFlags;
    }

    /* Compile the pattern, if necessary. */
    if((m_compiledParts & OnlyFlags) == 0)
    {
        qDebug() << "Compiling pattern..";
        retvalPattern = parsePattern(m_operands.at(1)->evaluateSingleton(context).stringValue(),
                                     context);

        if((m_compiledParts & OnlyPattern) != 0)
            flags = m_flags;
    }

    qDebug() << "flags: " << flags;
    applyFlags(flags, retvalPattern);

    Q_ASSERT(m_pattern.isValid());
    return retvalPattern;
}

void PatternPlatform::applyFlags(const Flags flags, QRegExp &patternP)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(patternP.isValid());
    if(flags == NoFlags)
        return;
    qDebug() << "Continuing..";

    if(flags & CaseInsensitive)
    {
        qDebug() << "Pattern is case insensitive.";
        patternP.setCaseSensitivity(Qt::CaseInsensitive);
    }
    // TODO Apply the other flags, like 'x'.
}

QRegExp PatternPlatform::parsePattern(const QString &patternP,
                                      const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;

    if(patternP == QLatin1String("(.)\\3") ||
       patternP == QLatin1String("\\3")    ||
       patternP == QLatin1String("(.)\\2"))
    {
        context->error(QLatin1String("We don't want to hang infinitely on K2-MatchesFunc-9, "
                                     "10 and 11. See Trolltech task 148505."),
                       ReportContext::FOER0000, this);
        return QRegExp();
    }

    QRegExp retval(patternP);

    if(retval.isValid())
        return retval;
    else
    {
        context->error(tr("%1 is an invalid regular expression pattern: %2")
                                        .arg(formatExpression(patternP), retval.errorString()),
                                   ReportContext::FORX0002, this);
        return QRegExp();
    }
}

PatternPlatform::Flags PatternPlatform::parseFlags(const QString &flags,
                                                   const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;

    if(flags.isEmpty())
        return NoFlags;

    const PatternFlag::Hash *const flagDescrs = flagDescriptions();
    const int len = flags.length();
    Flags retval = NoFlags;

    for(int i = 0; i < len; ++i)
    {
        const QChar flag(flags.at(i));
        const Flag specified = flagDescrs->value(flag).flag;

        if(specified != NoFlags)
        {
            retval |= specified;
            continue;
        }

        /* Generate a nice error message. */
        QString message(tr("%1 is an invalid flag for regular expressions. Available flags are:")
                             .arg(formatKeyword(flag)));

        /* This is formatting, so don't bother translators with it. */
        message.append(QLatin1Char('\n'));

        const PatternFlag::Hash::const_iterator end(flagDescrs->constEnd());
        PatternFlag::Hash::const_iterator it(flagDescrs->constBegin());

        for(; it != end;)
        {
            // TODO handle bidi correctly
            // TODO format this with rich text(list/table)
            message.append(formatKeyword(it.key()));
            message.append(QLatin1String(" - "));
            message.append(it.value().description);

            ++it;
            if(it != end)
                message.append(QLatin1Char('\n'));
        }

        context->error(message, ReportContext::FORX0001, this);
        return NoFlags;
    }

    qDebug() << "Returning flags: " << retval;
    return retval;
}

Expression::Ptr PatternPlatform::compress(const StaticContext::Ptr &context)
{
    qDebug() << Q_FUNC_INFO;
    const Expression::Ptr me(FunctionCall::compress(context));
    if(me.get() != this)
        return me;

    if(m_operands.at(1)->is(IDStringValue))
    {
        const DynamicContext::Ptr dynContext(context->dynamicContext());

        m_pattern = parsePattern(m_operands.at(1)->evaluateSingleton(dynContext).stringValue(),
                                 dynContext);
        m_compiledParts |= OnlyPattern;
    }

    const Expression::Ptr flagOperand(m_operands.value(m_flagsPosition));

    if(!flagOperand)
    {
        m_flags = NoFlags;
        m_compiledParts |= OnlyFlags;
    }
    else if(flagOperand->is(IDStringValue))
    {
        const DynamicContext::Ptr dynContext(context->dynamicContext());
        m_flags = parseFlags(flagOperand->evaluateSingleton(dynContext).stringValue(),
                             dynContext);
        m_compiledParts |= OnlyFlags;
    }

    if(m_compiledParts == FlagsAndPattern)
        applyFlags(m_flags, m_pattern);

    return me;
}

// vim: et:ts=4:sw=4:sts=4
