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

#ifndef Patternist_Tokenizer_H
#define Patternist_Tokenizer_H

#include <QPair>
#include <QSharedData>
#include <QString>

#include "qatomiccomparator_p.h"
#include "qatomicmathematician_p.h"
#include "qcombinenodes_p.h"
#include "qfunctionargument_p.h"
#include "qitemtype_p.h"
#include "qitem_p.h"
#include "qorderby_p.h"
#include "qquerytransformparser_p.h"
#include "qvalidate_p.h"

/**
 * @file
 * @short Contains functions and classes used by the parser and tokenizer.
 */

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    class ParserContext;

    typedef QPair<QString, Expression::Ptr> AttributeHolder;
    typedef QVector<AttributeHolder> AttributeHolderVector;

    class OrderSpecTransfer
    {
    public:
        typedef QList<OrderSpecTransfer> List;
        inline OrderSpecTransfer()
        {
        }

        inline OrderSpecTransfer(const Expression::Ptr &aExpr,
                                 const OrderBy::OrderSpec aOrderSpec) : expression(aExpr),
                                                                        orderSpec(aOrderSpec)
        {
            Q_ASSERT(expression);
        }

        Expression::Ptr     expression;
        OrderBy::OrderSpec  orderSpec;
    };

    /**
     * @short A union of all the enums the parser uses.
     */
    union EnumUnion
    {
        AtomicComparator::Operator              valueOperator;
        AtomicMathematician::Operator           mathOperator;
        CombineNodes::Operator                  combinedNodeOp;
        Node::Axis                              axis;
        Node::DocumentOrder                     nodeOperator;
        StaticContext::BoundarySpacePolicy      boundarySpacePolicy;
        StaticContext::ConstructionMode         constructionMode;
        StaticContext::InheritMode              inheritMode;
        StaticContext::OrderingEmptySequence    orderingEmptySequence;
        StaticContext::OrderingMode             orderingMode;
        StaticContext::PreserveMode             preserveMode;
        OrderBy::OrderSpec::Direction           sortDirection;
        Validate::Mode                          validationMode;
        VariableSlotID                          slot;
        int                                     tokenizerPosition;
        qint16                                  zeroer;
    };

    /**
     * This is the value the parser and scanner uses for
     * tokens and non-terminals. It is inefficient, but ensures
     * nothing leaks, by invoking C++ destructors even in the cases
     * the code throws exceptions. This might be able to be done in a more
     * efficient way -- suggestions are welcome.
     */
    class TokenValue
    {
    public:
        QString                         sval;

        Expression::Ptr                 expr;
        Expression::List                expressionList;

        Cardinality                     cardinality;
        ItemType::Ptr                   itemType;
        SequenceType::Ptr               sequenceType;
        FunctionArgument::List          functionArguments;
        FunctionArgument::Ptr           functionArgument;
        QName                           qName;
        /**
         * Holds enum values.
         */
        EnumUnion                       enums;

        AttributeHolder                 attributeHolder;
        AttributeHolderVector           attributeHolders;
        OrderSpecTransfer::List         orderSpecs;
        OrderSpecTransfer               orderSpec;
    };
}

QT_END_NAMESPACE

/**
 * Macro for the data type of semantic values; int by default.
 * See section Data Types of Semantic Values.
 */
#define YYSTYPE Patternist::TokenValue

#include "qquerytransformparser_p.h" /* This inclusion must be after TokenValue. */

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short Base class for all tokenizers.
     *
     * The main entry point is nextToken(), which ones calls to retrieve the stream
     * of tokens this Tokenizer delivers.
     *
     * @see <a href="http://www.w3.org/TR/xquery-xpath-parsing/">Building a
     * Tokenizer for XPath or XQuery</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Tokenizer : public QSharedData
    {
    public:
        inline Tokenizer()
        {
        }

        typedef PlainSharedPtr<Tokenizer> Ptr;

        /**
         * typedef for the enum Bison generates that contains
         * the token symbols.
         */
        typedef yytokentype TokenType;

        /**
         * Represents a token by carrying its name and value.
         */
        class Token
        {
        public:
            /**
             * Constructs an invalid Token. This default constructor
             * is need in Qt's container classes.
             */
            inline Token() {}
            inline Token(const TokenType t) : type(t) {enums.zeroer = 0;}
            inline Token(const TokenType t, const QString &val) : type(t), value(val) {enums.zeroer = 0;}
            inline Token(const TokenType t, const EnumUnion val) : type(t), enums(val) {}

            TokenType type;
            QString value;

            /**
             * Is 0 if not set.
             */
            EnumUnion enums;
        };

        /**
         * Destructor.
         */
        virtual ~Tokenizer();

        /**
         * @returns the next token.
         */
        virtual Token nextToken(YYLTYPE *const sourceLocator) = 0;

        /**
         * Switches the Tokenizer to only do scanning, and returns complete
         * strings for attribute value templates as opposed to the tokens for
         * the contained expressions.
         *
         * The current position in the stream is returned. It can be used to
         * later resume regular tokenization.
         */
        virtual int commenceScanOnly() = 0;

        /**
         * Resumes regular parsing from @p position. The tokenizer must be in
         * the scan-only state, which the commenceScanOnly() call transists to.
         *
         * The tokenizer will return the token POSITION_SET once after this
         * function has been called.
         */
        virtual void resumeTokenizationFrom(const int position) = 0;

        /**
         * @returns the URI of the resource being tokenized.
         */
        virtual QUrl uri() const = 0;

    private:
        Q_DISABLE_COPY(Tokenizer)
    };

}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
