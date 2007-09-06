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

#ifndef Patternist_Base64Binary_H
#define Patternist_Base64Binary_H

#include "Item.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements the value instance of the @c xs:base64Binary type.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class Base64Binary : public AtomicValue
    {
    public:
        friend class CommonValues;

        typedef AtomicValue::Ptr Ptr;

        /**
         * Creates an instance representing @p value.
         */
        static AtomicValue::Ptr fromLexical(const QString &value);

        static Base64Binary::Ptr fromValue(const QByteArray &data);

        virtual QString stringValue() const;
        virtual ItemType::Ptr type() const;
        virtual QByteArray asByteArray() const;

    protected:
        Base64Binary(const QByteArray &val);

        const QByteArray m_value;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
