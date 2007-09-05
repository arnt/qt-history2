/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#ifndef Patternist_ValidationError_H
#define Patternist_ValidationError_H

#include "Item.h"
#include "ReportContext.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Used for signalling casting errors.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class ValidationError : public AtomicValue
    {
    public:
        typedef PlainSharedPtr<ValidationError> Ptr;

        /**
         * Creates a ValidationError instance that represents a type error.
         *
         * @param description A detailed description of what that made the cast fail,
         * if any. If @c null, which QString() creates, a generic message
         * will be used.
         */
        static AtomicValue::Ptr createError(const QString &description = QString(),
                                            const ReportContext::ErrorCode = ReportContext::FORG0001);

        /**
         * A human readable, translated message describing the error.
         */
        QString message() const;

        /**
         * @returns always @c true
         */
        virtual bool hasError() const;

        /**
         * Always results in an assert crash.
         */
        virtual ItemType::Ptr type() const;

        /**
         * Always results in an assert crash.
         */
        virtual QString stringValue() const;

        /**
         * @returns the error code this ValidationError represents. Typically, this
         * is ReportContext::FORG0001.
         */
        ReportContext::ErrorCode errorCode() const;

    protected:
        ValidationError(const QString &msg, const ReportContext::ErrorCode code);

        const QString                   m_message;
        const ReportContext::ErrorCode  m_code;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
