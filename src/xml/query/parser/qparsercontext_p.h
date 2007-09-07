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

#ifndef Patternist_ParserContext_H
#define Patternist_ParserContext_H

#include <QFlags>
#include <QSharedData>
#include <QStack>
#include <QStringList>
#include <QtGlobal>

#include "qexpressionfactory_p.h"
#include "quserfunction_p.h"
#include "quserfunctioncallsite_p.h"
#include "qvariabledeclaration_p.h"
#include "qfunctionsignature_p.h"
#include "qtokenizer_p.h"
#include "qbuiltintypes_p.h"
#include "qdebug_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Contains data used when parsing and tokenizing.
     *
     * When ExpressionFactory::create() is called, an instance of this class
     * is passed to the scanner and parser. It holds all information that is
     * needed to create the expression.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class ParserContext : public QSharedData
    {
    public:
        typedef PlainSharedPtr<ParserContext> Ptr;

        enum PrologDeclaration
        {
            BoundarySpaceDecl               = 1,
            DefaultCollationDecl            = 2,
            BaseURIDecl                     = 4,
            ConstructionDecl                = 8,
            OrderingModeDecl                = 16,
            EmptyOrderDecl                  = 32,
            CopyNamespacesDecl              = 64,
            DeclareDefaultElementNamespace  = 128,
            DeclareDefaultFunctionNamespace = 256
        };

        typedef QFlags<PrologDeclaration> PrologDeclarations;

        /**
         * Constructs a ParserContext instance.
         *
         * @param context the static context as defined in XPath. This contain
         * namespace bindings, error handler, and other information necessary
         * for creating an XPath expression.
         * @param lang the particular XPath language sub-set that should be parsed
         * @param tokenizer the Tokenizer to use.
         * @see ExpressionFactory::LanguageAccent
         */
        ParserContext(const StaticContext::Ptr &context,
                      const ExpressionFactory::LanguageAccent lang,
                      const Tokenizer::Ptr &tokenizer);

        inline VariableSlotID allocateRangeSlot()
        {
            ++m_rangeSlot;
            qDebug() << Q_FUNC_INFO << m_rangeSlot << endl;
            return m_rangeSlot;
        }

        inline VariableSlotID allocatePositionalSlot()
        {
            ++m_positionSlot;
            qDebug() << Q_FUNC_INFO << m_positionSlot << endl;
            return m_positionSlot;
        }

        inline VariableSlotID allocateExpressionSlot()
        {
            qDebug() << Q_FUNC_INFO << m_expressionSlot << endl;
            const VariableSlotID retval = m_expressionSlot;
            ++m_expressionSlot;
            return retval;
        }

        inline bool hasDeclaration(const PrologDeclaration decl) const
        {
            return (m_prologDeclarations & decl) == decl;
        }

        inline void registerDeclaration(const PrologDeclaration decl)
        {
            m_prologDeclarations |= decl;
        }

        /**
         * The namespaces declared with <tt>declare namespace</tt>.
         */
        QStringList declaredPrefixes;

        VariableDeclaration::Stack variables;

        const StaticContext::Ptr staticContext;
        const Tokenizer::Ptr tokenizer;
        const ExpressionFactory::LanguageAccent languageAccent;

        /**
         * Used when parsing direct element constructors. It is used
         * for ensuring tags are well-balanced.
         */
        QStack<QName> tagStack;

        /**
         * The actual expression, the Query. This member may be @c null,
         * such as in the case of an XQuery library module.
         */
        Expression::Ptr queryBody;

        /**
         * The user functions declared in the prolog.
         */
        UserFunction::List userFunctions;

        /**
         * Contains all calls to user defined functions.
         */
        UserFunctionCallsite::List userFunctionCallsites;

        /**
         * All variables declared with <tt>declare variable</tt>.
         */
        VariableDeclaration::List declaredVariables;

        inline VariableSlotID currentRangeSlot() const
        {
            qDebug() << Q_FUNC_INFO << m_rangeSlot << endl;
            return m_rangeSlot;
        }

        inline VariableSlotID currentPositionSlot() const
        {
            return m_positionSlot;
        }

        inline VariableSlotID currentExpressionSlot() const
        {
            return m_expressionSlot;
        }

        inline void restoreNodeTestSource()
        {
            nodeTestSource = BuiltinTypes::element;
        }

        inline VariableSlotID allocateCacheSlot()
        {
            return ++m_evaluationCacheSlot;
        }

        ItemType::Ptr nodeTestSource;

        QStack<Expression::Ptr> typeswitchSource;

        /**
         * The library module namespace set with <tt>declare module</tt>.
         */
        QName::NamespaceCode moduleNamespace;

        /**
         * When a direct element constructor is processed, resolvers are
         * created in order to carry the namespace declarations. In such case,
         * the old resolver is pushed here.
         */
        QStack<NamespaceResolver::Ptr> resolvers;

        /**
         * This is used for handling the following obscene case:
         *
         * - <tt>\<e\>{1}{1}\<\/e\></tt> produce <tt>\<e\>11\</e\></tt>
         * - <tt>\<e\>{1, 1}\<\/e\></tt> produce <tt>\<e\>1 1\</e\></tt>
         *
         * This boolean tracks whether the previous reduction inside element
         * content was done with an enclosed expression.
         */
        bool isPreviousEnclosedExpr;

        int elementConstructorDepth;

        QStack<bool> scanOnlyStack;

        QStack<OrderBy::Stability> orderStability;
    private:
        VariableSlotID      m_evaluationCacheSlot;
        VariableSlotID      m_rangeSlot;
        VariableSlotID      m_expressionSlot;
        VariableSlotID      m_positionSlot;
        PrologDeclarations  m_prologDeclarations;
        Q_DISABLE_COPY(ParserContext)
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
