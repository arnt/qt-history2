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

#ifndef Patternist_ProcessingInstructionConstructor_H
#define Patternist_ProcessingInstructionConstructor_H

#include "PairContainer.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Constructs an element node. This covers both computed and directly constructed
     * element nodes.
     *
     * @see <a href="http://www.w3.org/TR/xquery/#id-constructors">XQuery
     * 1.0: An XML Query Language, 3.7 Constructors</a>
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class ProcessingInstructionConstructor : public PairContainer
    {
    public:
        ProcessingInstructionConstructor(const Expression::Ptr &operand1,
                           const Expression::Ptr &operand2);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
        virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

        virtual SequenceType::Ptr staticType() const;

        /**
         * Both arguments must of type @c xs:string. It is assumes that the first argument's
         * lexical space is @c xs:NCName.
         */
        virtual SequenceType::List expectedOperandTypes() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

        virtual Properties properties() const;
    private:
        inline QName evaluateTarget(const DynamicContext::Ptr &context) const;
        /**
         * Performs left-trimming only.
         *
         * @see QString::trimmed()
         */
        static inline QString leftTrimmed(const QString &input);

        QString data(const DynamicContext::Ptr &context) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
