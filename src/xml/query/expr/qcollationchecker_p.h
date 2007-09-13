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

#ifndef Patternist_CollationChecker_H
#define Patternist_CollationChecker_H

#include "qsinglecontainer_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short Checks that its operand evaluates to a supported string collation.
     *
     * CollationChecker is inserted in the AST when an Expression has LastOperandIsCollation
     * set. If the argument that CollationChecker is a string literal the CollationChecker
     * will const-fold as usual, but otherwise will simply pipe through the value of its argument,
     * if it's a supported collation. Otherwise it raise an error, with code ReportContext::FOCH0002.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_expressions
     */
    class CollationChecker : public SingleContainer
    {
    public:
        CollationChecker(const Expression::Ptr &source);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        /**
         * Expects exactly one string.
         */
        virtual SequenceType::List expectedOperandTypes() const;

        /**
         * @returns its operand's static type.
         */
        virtual SequenceType::Ptr staticType() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
