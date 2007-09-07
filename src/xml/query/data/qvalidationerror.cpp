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

#include "qdebug_p.h"

#include "qvalidationerror_p.h"

using namespace Patternist;

ValidationError::ValidationError(const QString &msg,
                                 const ReportContext::ErrorCode code) : m_message(msg),
                                                                        m_code(code)
{
}

AtomicValue::Ptr ValidationError::createError(const QString &description,
                                              const ReportContext::ErrorCode code)
{
    return ValidationError::Ptr(new ValidationError(description, code));
}

bool ValidationError::hasError() const
{
    return true;
}

QString ValidationError::stringValue() const
{
    Q_ASSERT_X(false, Q_FUNC_INFO, "stringValue() asked for ValidationError -- it makes no sense.");
    return QString();
}

QString ValidationError::message() const
{
    return m_message;
}

ItemType::Ptr ValidationError::type() const
{
    Q_ASSERT_X(false, Q_FUNC_INFO,
               "This function should never be called, the caller "
               "didn't check whether the AtomicValue was an ValidationError.");
    return ItemType::Ptr();
}

ReportContext::ErrorCode ValidationError::errorCode() const
{
    return m_code;
}

// vim: et:ts=4:sw=4:sts=4
