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

#ifndef Patternist_BuiltinTypesFactory_H
#define Patternist_BuiltinTypesFactory_H

#include <QHash>
#include "qschematypefactory_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short Factory for creating schema types for the types defined in XSL-T 2.0.
     *
     * Theses types are essentially the builtin primitive types, plus @c xs:integer,
     * and the types defined in the XPath Data Model.
     *
     * @ingroup Patternist_types
     * @see <a href="http://www.w3.org/TR/xpath-datamodel/#types-predefined">XQuery 1.0 and
     * XPath 2.0 Data Model, 2.6.2 Predefined Types</a>
     * @see <a href="http://www.w3.org/TR/xslt20/#built-in-types">XSL Transformations (XSLT)
     * Version 2.0, 3.13 Built-in Types</a>
     * @see <a href="http://www.w3.org/TR/xmlschema-2/#built-in-primitive-datatypes">XML Schema
     * Part 2: Datatypes Second Edition, 3.2 Primitive datatypes</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class BasicTypesFactory : public SchemaTypeFactory
    {
    public:

        /**
         * Creates a primitive type for @p name. If @p name is not supported,
         * @c null is returned.
         * The intened supported types are the builtin primitive and derived types.
         * That is, the 19 W3C XML Schema types, and the additional 5 in the XPath Data MOdel.
         *
         * @note This does not handle user defined types, only builtin types.
         * @todo Update documentation, proportionally with progress.
         */
        virtual SchemaType::Ptr createSchemaType(const QName ) const;

        virtual SchemaType::Hash types() const;

        /**
         * @returns the singleton instance of BasicTypesFactory.
         */
        static SchemaTypeFactory::Ptr self(const NamePool::Ptr &np);

    protected:
        /**
         * This constructor is protected. Use the static self() function
         * to retrieve a singleton instance.
         */
        BasicTypesFactory(const NamePool::Ptr &np);

    private:
        /**
         * A dictonary of builtin primitive and derived primitives.
         */
        SchemaType::Hash m_types;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
