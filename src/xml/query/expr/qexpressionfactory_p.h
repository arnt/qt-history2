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

#ifndef Patternist_ExpressionFactory_H
#define Patternist_ExpressionFactory_H

class QIODevice;

#include <QSharedData>
#include <QUrl>

#include "Expression.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short The central entry point for compiling expressions.
     *
     * @ingroup Patternist_expressions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ExpressionFactory : public QSharedData
    {
    public:
        typedef PlainSharedPtr<ExpressionFactory> Ptr;

        /**
         * @short This constructor cannot be synthesized since we
         * use the Q_DISABLE_COPY macro.
         */
        inline ExpressionFactory()
        {
        }

        virtual ~ExpressionFactory()
        {
        }

        /**
         * The XPath implementation, emphazing the parser, support
         * different types of languages which all are sub-sets or very
         * close to the XPath 2.0 syntax. This enum is used for communicating
         * what particular language "accent" an expression is of, and should be compiled
         * for.
         *
         * @author Frans Englich <fenglich@trolltech.com>
         */
        enum LanguageAccent
        {
            /**
             * Signifies the XPath 2.0 language.
             *
             * @see <a href="http://www.w3.org/TR/xpath20">XML Path Language (XPath) 2.0</a>
             */
            XPath10     = 1,

            /**
             * Signifies the XPath 1.0 language.
             *
             * @see <a href="http://www.w3.org/TR/xpath">XML Path
             * Language (XPath) Version 1.0</a>
             */
            XPath20     = 2,

            /*
             * Signifies the XSL-T 2.0 Attribute Value Template. That is,
             * a plain attribute value template that contains
             * embedded XPath 2.0 expressions.
             *
             * @see <a href="http://www.w3.org/TR/xslt20/#attribute-value-templates">XSL
             * Transformations (XSLT) Version 2.0, 5.6 Attribute Value Templates</a>
             *
            AVT20
             */

            /**
             * Signifies the XQuery 1.0 language.
             * @see <a href="http://www.w3.org/TR/xquery/">XQuery 1.0: An XML Query Language</a>
             */
            XQuery10    = 4
        };

        enum CompilationStage
        {
            QueryBodyInitial        = 1,
            QueryBodyTypeCheck      = 2,
            QueryBodyCompression    = 4,
            UserFunctionTypeCheck   = 8,
            UserFunctionCompression = 16,
            GlobalVariableTypeCheck = 32
        };

        /**
         * Creates a compiled representation of the XPath expression @p expr, with Static
         * Context information supplied via @p context. This is for example whether the expression
         * is an XPath 1.0 or XPath 2.0 expression, or what functions that are available.
         *
         * @p requiredType specifies what type results of the evaluating the expression
         * must match. Passing CommonValues::ZeroOrMoreItems allows anything as result, while
         * passing CommonSequenceTypes::EBV means anything but an Effective %Boolean Value extractable
         * result is a type error, for example.
         *
         * @note An empty @p expr is an invalid XPath expression. It will be reported as such,
         * but it is neverthless the caller's resonsibility to ensure that it's not that(since
         * it is likely invalid already in the medium it was stored).
         */
        virtual Expression::Ptr createExpression(const QString &expr,
                                                 const StaticContext::Ptr &context,
                                                 const LanguageAccent lang,
                                                 const SequenceType::Ptr &requiredType,
                                                 const QUrl &queryURI);

        Expression::Ptr createExpression(QIODevice *const device,
                                         const StaticContext::Ptr &context,
                                         const LanguageAccent lang,
                                         const SequenceType::Ptr &requiredType,
                                         const QUrl &queryURI);

    protected:
        /**
         * This function is called by createExpression() each time
         * after a pass on the AST has been completed. Under a typical
         * compilation this function is thus called three times: after the initial
         * build, after the Expression::typeCheck() stage, and after
         * Expression::compress(). @p tree is the AST after each pass.
         *
         * This mechanism is currently used for debugging, since it provides a
         * way of introspecting what the compilation process do to the tree. The
         * current implementation do nothing.
         */
        virtual void processTreePass(const Expression::Ptr &tree,
                                     const CompilationStage stage);

    private:
        Q_DISABLE_COPY(ExpressionFactory)
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
