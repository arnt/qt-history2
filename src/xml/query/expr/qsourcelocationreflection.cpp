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

#include "qsourcelocation.h"

#include "qsourcelocationreflection_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

QSourceLocation SourceLocationReflection::sourceLocation() const
{
    return QSourceLocation();
}

const SourceLocationReflection *DelegatingSourceLocationReflection::actualReflection() const
{
    return m_r->actualReflection();
}

QString DelegatingSourceLocationReflection::description() const
{
    return m_r->description();
}

QT_END_NAMESPACE
