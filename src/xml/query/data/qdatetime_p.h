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

#ifndef Patternist_DateTime_H
#define Patternist_DateTime_H

#include "AbstractDateTime.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the value instance of the @c xs:dateTime type.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class DateTime : public AbstractDateTime
    {
    public:
        typedef AtomicValue::Ptr Ptr;

        /**
         * Creates an instance from the lexical representation @p string.
         */
        static DateTime::Ptr fromLexical(const QString &string);
        static DateTime::Ptr fromDateTime(const QDateTime &dt);
        virtual ItemType::Ptr type() const;
        virtual QString stringValue() const;
        virtual Item fromValue(const QDateTime &dt) const;

    protected:
        friend class CommonValues;

        DateTime(const QDateTime &dateTime);
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
