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

#ifndef Patternist_GMonthDay_H
#define Patternist_GMonthDay_H

#include "AbstractDateTime.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the value instance of the @c xs:gYearMonth type.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class GMonthDay : public AbstractDateTime
    {
    public:
        typedef AtomicValue::Ptr Ptr;

        /**
         * Creates an instance from the lexical representation @p string.
         */
        static GMonthDay::Ptr fromLexical(const QString &string);
        static GMonthDay::Ptr fromDateTime(const QDateTime &dt);

        virtual ItemType::Ptr type() const;

        virtual QString stringValue() const;

    protected:
        friend class CommonValues;

        GMonthDay(const QDateTime &dateTime);
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
