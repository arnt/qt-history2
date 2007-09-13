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

#include "qfunctionargument_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

FunctionArgument::FunctionArgument(const QName nameP,
                                   const SequenceType::Ptr &typeP) : m_name(nameP),
                                                                     m_type(typeP)
{
    Q_ASSERT(!nameP.isNull());
    Q_ASSERT(typeP);
}

QName FunctionArgument::name() const
{
    return m_name;
}

SequenceType::Ptr FunctionArgument::type() const
{
    return m_type;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
