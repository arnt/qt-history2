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

#include "Debug.h"

#include "UserFunction.h"

using namespace Patternist;

UserFunction::UserFunction(const FunctionSignature::Ptr &sign,
                           const Expression::Ptr &b,
                           const VariableSlotID slotOffset,
                           const VariableDeclaration::List &varDecls) : m_signature(sign),
                                                                        m_body(b),
                                                                        m_slotOffset(slotOffset),
                                                                        m_argumentDeclarations(varDecls)
{
    qDebug() << Q_FUNC_INFO << "slot: " << slotOffset;
    Q_ASSERT(m_signature);
    Q_ASSERT(m_body);
    Q_ASSERT(m_slotOffset > -2);
}

// vim: et:ts=4:sw=4:sts=4
