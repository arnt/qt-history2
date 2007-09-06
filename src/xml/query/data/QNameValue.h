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

#ifndef Patternist_QNameValue_H
#define Patternist_QNameValue_H

#include "Item.h"
#include "QName.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the value instance of the @c xs:QName type.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     * @todo Documentation is missing/incomplete
     */
    class QNameValue : public AtomicValue
    {
    public:
        friend class CommonValues;
        friend class QNameComparator;

        typedef PlainSharedPtr<QNameValue> Ptr;

        /**
         * Constructs a QNameValue that represents @p name.
         *
         * @param name the QName. May not be @c null.
         * @param np the NamePool.
         */
        static QNameValue::Ptr fromValue(const NamePool::Ptr &np, const QName name);

        virtual QString stringValue() const;

        virtual ItemType::Ptr type() const;

        inline QName qName() const
        {
            return m_qName;
        }

    private:
        QNameValue(const NamePool::Ptr &np, const QName name);

        const QName         m_qName;
        const NamePool::Ptr m_namePool;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
