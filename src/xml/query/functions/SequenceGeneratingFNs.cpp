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

#include "AnyURI.h"
#include "Boolean.h"
#include "CommonSequenceTypes.h"
#include "CommonValues.h"
#include "EmptySequence.h"
#include "ListIterator.h"
#include "PatternistLocale.h"

#include "SequenceGeneratingFNs.h"

using namespace Patternist;

Item::Iterator::Ptr IdFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr idrefs(m_operands.first()->evaluateSequence(context));

    Item idref(idrefs->next());
    if(!idref)
        return CommonValues::emptyIterator;

    const Item node(m_operands.last()->evaluateSingleton(context));

    checkTargetNode(node.asNode(), context);

    return CommonValues::emptyIterator; /* Haven't implemented further. */
}

void IdFN::checkTargetNode(const Node &node, const DynamicContext::Ptr &context) const
{
    if(node.root().kind() != Node::Document)
    {
        context->error(tr("The root node of the second argument to function %1 "
                          "must be a document node. %1 isn't.").arg(formatFunction(context->namePool(), signature()),
                                                                    formatData(node)),
                        ReportContext::FODC0001, this);
    }
}

Item::Iterator::Ptr IdrefFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr ids(m_operands.first()->evaluateSequence(context));

    Item mId(ids->next());
    if(!mId)
        return CommonValues::emptyIterator;

    const Item node(m_operands.last()->evaluateSingleton(context));
    checkTargetNode(node.asNode(), context);

    return CommonValues::emptyIterator; /* TODO Haven't implemented further. */
}

Item DocFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item itemURI(m_operands.first()->evaluateSingleton(context));

    if(!itemURI)
        return Item();

    const QUrl uri(itemToURI(itemURI, context));

    Q_ASSERT(uri.isValid());
    Q_ASSERT(!uri.isRelative());

    const Item doc(context->resourceLoader()->openDocument(uri));

    if(doc)
        return doc;
    else
    {
        // TODO don't we want the ResourceLoader to pass back some message?
        context->error(tr("Cannot retrieve %1.").arg(formatURI(uri)),
                       ReportContext::FODC0002, this);
        return Item();
    }
}

QUrl DocFN::itemToURI(const Item &itemURI, const ReportContext::Ptr &context) const
{
    Q_ASSERT(itemURI);
    Q_ASSERT_X(!m_staticBaseURI.isEmpty(), Q_FUNC_INFO,
               "m_staticBaseURI should be set when this function is called.");
    qDebug() << Q_FUNC_INFO << "static base uri:" << m_staticBaseURI;

    // TODO TEST this  dyn/stat
    const QUrl mayRela(AnyURI::toQUrl<ReportContext::FODC0005>(itemURI.stringValue(), context, this));

    return context->resolveURI(mayRela, m_staticBaseURI);
}

Expression::Ptr DocFN::typeCheck(const StaticContext::Ptr &context,
                                 const SequenceType::Ptr &reqType)
{
    Q_ASSERT(context);

    m_staticBaseURI = context->baseURI();

    const Expression::Ptr uriOp(m_operands.first());

    if(!uriOp->isEvaluated())
        return Expression::Ptr(FunctionCall::typeCheck(context, reqType));

    const Item uriItem(uriOp->evaluateSingleton(context->dynamicContext()));

    if(!uriItem)
        return EmptySequence::create(this, context)->typeCheck(context, reqType); // TODO test this

    const QUrl uri(itemToURI(uriItem, context));

    /* The URI is supplied statically, so, let's try to be clever. */
    Q_ASSERT_X(context->resourceLoader(), Q_FUNC_INFO,
               "No resource loader is set in the StaticContext.");
    m_type = context->resourceLoader()->announceDocument(uri, ResourceLoader::MayUse);

    if(m_type)
    {
        Q_ASSERT(CommonSequenceTypes::ZeroOrOneDocumentNode->matches(m_type));
        return Expression::Ptr(FunctionCall::typeCheck(context, reqType));
    }
    else
    {
        context->error(tr("It will not be possible to retrieve %1.").arg(formatURI(uri)),
                       ReportContext::FODC0002, this);
        return Expression::Ptr();
    }
}

SequenceType::Ptr DocFN::staticType() const
{
    if(m_type)
        return m_type;
    else
        return CommonSequenceTypes::ZeroOrOneDocumentNode;
}

bool DocAvailableFN::evaluateEBV(const DynamicContext::Ptr &context) const
{
    const Item itemURI(m_operands.first()->evaluateSingleton(context));

    /* 15.5.4 fn:doc reads: "If $uri is the empty sequence, the result is an empty sequence."
     * Hence, we return false for the empty sequence, because this doesn't hold true:
     * "If this function returns true, then calling fn:doc($uri) within
     * the same ·execution scope· must return a document node."(15.5.5 fn:doc-available) */
    if(!itemURI)
        return false;

    const QUrl uri(AnyURI::toQUrl<ReportContext::FODC0005>(itemURI.stringValue(), context, this));

    Q_ASSERT(!uri.isRelative());
    return context->resourceLoader()->isDocumentAvailable(uri);
}

Item::Iterator::Ptr CollectionFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    // TODO resolve with URI resolve
    if(m_operands.isEmpty())
    {
        // TODO check default collection
        context->error(tr("The default collection is undefined"),
                       ReportContext::FODC0002, this);
        return CommonValues::emptyIterator;
    }
    else
    {
        const Item itemURI(m_operands.first()->evaluateSingleton(context));

        if(itemURI)
        {
            const QUrl uri(AnyURI::toQUrl<ReportContext::FODC0004>(itemURI.stringValue(), context, this));

            // TODO 2. Resolve against static context base URI(store base URI at compile time)
            context->error(tr("%1 cannot be retrieved").arg(formatURI(uri)),
                           ReportContext::FODC0004, this);
            return CommonValues::emptyIterator;
        }
        else
        {
            /* This is out default collection currently, */
            return CommonValues::emptyIterator;
        }
    }
}

// vim: et:ts=4:sw=4:sts=4
