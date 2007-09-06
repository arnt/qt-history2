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

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @todo docs
     *
     * XPath's approach of compilation is traditional. An Abstract Syntax Tree(AST)
     * is built, where the Expression class is the abstract base class for all kinds
     * of implementations of expressions.
     *
     * What perhaps can be said to be characteristic for Patternist is that the base class,
     * Expression, performs a lot of work, and that sub-classes declares what specific
     * behaviors they need, which the Expression's functions then bring into action.
     *
     * XPath expression often have different amount of operand. For example, the 'and' expression
     * takes two, the context item(".") none, and the if-expression three. To help expression
     * implementations with that, there exist the abstract EmptyContainer, SingleContainer,
     * PairContainer, TripleContainer, and UnlimitedContainer classes for avoiding
     * duplicating code.
     *
     * @defgroup Patternist_expressions Expressions
     * @author Frans Englich <fenglich@trolltech.com>
     */
}

// vim: et:ts=4:sw=4:sts=4
