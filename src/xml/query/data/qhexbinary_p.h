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

#ifndef Patternist_HexBinary_H
#define Patternist_HexBinary_H

#include "Base64Binary.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Implements the value instance of the @c xs:hexBinary type.
     *
     * HexBinary inherits from Base64Binary for implementation reasons. The two
     * classes are similar, and inheritance therefore save code.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     * @todo Documentation is missing
     */
    class HexBinary : public Base64Binary
    {
    public:
        friend class CommonValues;

        typedef AtomicValue::Ptr Ptr;

        virtual QString stringValue() const;
        virtual ItemType::Ptr type() const;

        /**
         * Creates a @c xs:hexBinary from the lexical representation @p value.
         */
        static AtomicValue::Ptr fromLexical(const NamePool::Ptr &np, const QString &value);

        /**
         * Creates an instance representing @p value.
         */
        static HexBinary::Ptr fromValue(const QByteArray &data);

    protected:
        HexBinary(const QByteArray &val);

    private:
        static inline qint8 fromHex(const QChar &c);
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
