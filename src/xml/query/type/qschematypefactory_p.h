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

#ifndef Patternist_SchemaTypeFactory_H
#define Patternist_SchemaTypeFactory_H

#include <QSharedData>

#include "qreportcontext_p.h"
#include "qitemtype_p.h"
#include "qschematype_p.h"

QT_BEGIN_HEADER 

QT_BEGIN_NAMESPACE

namespace Patternist
{
    /**
     * @short A factory creating schema types.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SchemaTypeFactory : public QSharedData
    {
    public:
        typedef PlainSharedPtr<SchemaTypeFactory> Ptr;

        SchemaTypeFactory();
        virtual ~SchemaTypeFactory();

        /**
         * @returns a schema type for name @p name. If no schema type exists for @p name, @c null
         * is returned
         */
        virtual SchemaType::Ptr createSchemaType(const QName name) const = 0;

        /**
         * @returns a dictionary containing the types this factory serves. The key
         * is the type's QName in Clark name syntax.
         */
        virtual SchemaType::Hash types() const = 0;

    private:
        Q_DISABLE_COPY(SchemaTypeFactory)
    };
}

QT_END_NAMESPACE

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
