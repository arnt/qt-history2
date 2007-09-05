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

#include "BuiltinTypes.h"
#include "CommonNamespaces.h"
#include "CommonSequenceTypes.h"
#include "PatternistLocale.h"
#include "QNameValue.h"
#include "AtomicString.h"

#include "AttributeNameValidator.h"

using namespace Patternist;

AttributeNameValidator::AttributeNameValidator(const Expression::Ptr &source) : SingleContainer(source)
{
}

Item AttributeNameValidator::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item name(m_operand->evaluateSingleton(context));
    const QName qName(name.as<QNameValue>()->qName());

    if(qName.namespaceURI() == StandardNamespaces::xmlns)
    {
        context->error(tr("The namespace URI in the name for a computed attribute cannot "
                          "equal %1").arg(formatURI(CommonNamespaces::XMLNS)),
                       ReportContext::XQDY0044, this);
        return Item(); /* Silence warning. */
    }
    else if(qName.namespaceURI() == StandardNamespaces::empty &&
            qName.localName() == StandardLocalNames::xmlns)
    {
        context->error(tr("The name for a computed attribute cannot have a namespace URI "
                          "equal to %1 while having the local name %2")
                          .arg(formatURI(CommonNamespaces::XMLNS))
                          .arg(formatKeyword("xmlns")),
                       ReportContext::XQDY0044, this);
        return Item(); /* Silence warning. */
    }
    else
        return name;
}

SequenceType::Ptr AttributeNameValidator::staticType() const
{
    return m_operand->staticType();
}

SequenceType::List AttributeNameValidator::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ExactlyOneQName);
    return result;
}

ExpressionVisitorResult::Ptr AttributeNameValidator::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
