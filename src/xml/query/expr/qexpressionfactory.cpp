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

#include <QByteArray>
#include <QtDebug>

#include "CommonSequenceTypes.h"
#include "Debug.h"
#include "Expression.h"
#include "OptimizationPasses.h"
#include "ParserContext.h"
#include "QueryTransformParser.h"
#include "Tokenizer.h"
#include "XQueryTokenizer.h"

#include "ExpressionFactory.h"

using namespace Patternist;

/**
 * @short The entry point to the parser.
 *
 * @param info supplies the information the parser & scanner
 * needs to create expressions. The created expression, if everything
 * succeeds, can be retrieved via the object @p info points to.
 * @returns non-negative if the parser fails.
 * @see ExpressionFactory::createExpression()
 */
extern int XPathparse(Patternist::ParserContext *const info);

Expression::Ptr ExpressionFactory::createExpression(const QString &expr,
                                                    const StaticContext::Ptr &context,
                                                    const LanguageAccent lang,
                                                    const SequenceType::Ptr &requiredType,
                                                    const QUrl &queryURI)
{
    qDebug() << Q_FUNC_INFO << queryURI;
    Q_ASSERT(context);
    Q_ASSERT(requiredType);
    Q_ASSERT(queryURI.isValid());

    OptimizationPasses::Coordinator::init();

    const ParserContext::Ptr info(new ParserContext(context, lang,
                                                    Tokenizer::Ptr(new XQueryTokenizer(expr, queryURI))));

    const int bisonRetval = XPathparse(info.get());

    Q_ASSERT_X(bisonRetval == 0, Q_FUNC_INFO,
               "We shouldn't be able to get an error, because we throw exceptions.");
    Q_UNUSED(bisonRetval); /* Needed when not compiled in debug mode, since bisonRetval won't
                            * be used in the Q_ASSERT_X above. */

    Expression::Ptr result(info->queryBody);

    if(!result)
    {
        context->error(tr("A library module cannot be evaluated directly, it must be imported from a main module."),
                       ReportContext::XPST0003, QSourceLocation(queryURI, 1, 1));
    }

    qDebug() << "----- Initial AST build. -----";
    processTreePass(result, QueryBodyInitial);
    qDebug() << "------------------------------";

    qDebug() << "-----     Type Check     -----";
    result->rewrite(result, result->typeCheck(context, requiredType), context);
    processTreePass(result, QueryBodyTypeCheck);
    qDebug() << "------------------------------";

    qDebug() << "-----      Compress      -----";
    result->rewrite(result, result->compress(context), context);
    processTreePass(result, QueryBodyCompression);
    qDebug() << "------------------------------";

    /* Here, we type check user declared functions and global variables. This means
     * that variables and functions that are not used are type checked(which they otherwise
     * wouldn't have been), and those which are used, are type-checked twice, unfortunately. */

    const UserFunction::List::const_iterator end(info->userFunctions.constEnd());
    UserFunction::List::const_iterator it(info->userFunctions.constBegin());
    for(; it != end; ++it)
    {
        const Expression::Ptr typeCheck((*it)->body()->typeCheck(context, (*it)->signature()->returnType()));
        processTreePass(typeCheck, UserFunctionTypeCheck);

        const Expression::Ptr comp(typeCheck->compress(context));
        processTreePass(comp, UserFunctionCompression);
    }

    const VariableDeclaration::Stack::const_iterator vend(info->variables.constEnd());
    VariableDeclaration::Stack::const_iterator vit(info->variables.constBegin());
    for(; vit != vend; ++vit)
    {
        Q_ASSERT(*vit);
        /* If it's already used, it will be typeChecked later on. */
        if((*vit)->isUsed())
            continue;

        Q_ASSERT((*vit)->expression());
        /* We supply ZeroOrMoreItems, meaning the variable can evaluate to anything. */
        // FIXME which is a source to bugs
        const Expression::Ptr
        nev((*vit)->expression()->typeCheck(context, CommonSequenceTypes::ZeroOrMoreItems));
        processTreePass(nev, GlobalVariableTypeCheck);
    }

    return result;
}

Expression::Ptr ExpressionFactory::createExpression(QIODevice *const device,
                                                    const StaticContext::Ptr &context,
                                                    const LanguageAccent lang,
                                                    const SequenceType::Ptr &requiredType,
                                                    const QUrl &queryURI)
{
    Q_ASSERT(device);
    Q_ASSERT(device->isReadable());

    // TODO We need to do encoding detection.
    return createExpression(QString::fromUtf8(device->readAll()), context, lang, requiredType, queryURI);
}

void ExpressionFactory::processTreePass(const Expression::Ptr &, const CompilationStage)
{
}

// vim: et:ts=4:sw=4:sts=4
