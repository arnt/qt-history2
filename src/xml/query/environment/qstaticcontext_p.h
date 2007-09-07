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

#ifndef Patternist_StaticContext_H
#define Patternist_StaticContext_H

class QUrl;
template<typename Key, typename T> class QHash;

#include "ExternalVariableLoader.h"
#include "ItemType.h"
#include "NamePool.h"
#include "NamespaceResolver.h"
#include "ReportContext.h"
#include "ResourceLoader.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    class DynamicContext;
    class FunctionFactory;
    class SchemaTypeFactory;

    /**
     * @short Carries information and facilities used at compilation time.
     *
     * A representation of the Static Context in XPath 2.0. The Static Context
     * contains information which doesn't change and is the "outer scope" of the
     * expression. It provides for example a base URI the expression can relate to and
     * what functions and variables that are available for the expression.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#static_context">XML Path
     * Language (XPath) 2.0, 2.1.1 Static Context</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class StaticContext : public ReportContext
    {
    public:
        /**
         * A smart pointer wrapping StaticContext instances.
         */
        typedef PlainSharedPtr<StaticContext> Ptr;

        /**
         * @see <a href="http://www.w3.org/TR/xquery/#id-boundary-space-decls">XQuery 1.0:
         * An XML Query Language, 4.3 Boundary-space Declaration</a>
         * @see <a href="http://www.w3.org/TR/xquery/#dt-boundary-space-policy">XQuery 1.0:
         * An XML Query Language, Definition: Boundary-space policy</a>
         */
        enum BoundarySpacePolicy
        {
            BSPPreserve,
            BSPStrip
        };

        /**
         * @see <a href="http://www.w3.org/TR/xquery/#id-construction-declaration">XQuery 1.0:
         * An XML Query Language, 4.6 Construction Declaration</a>
         * @see <a href="http://www.w3.org/TR/xquery/#dt-construction-mode">XQuery 1.0:
         * An XML Query Language, Definition: Construction mode</a>
         */
        enum ConstructionMode
        {
            CMPreserve,
            CMStrip
        };

        /**
         * @see <a href="http://www.w3.org/TR/xquery/#id-default-ordering-decl">XQuery 1.0:
         * An XML Query Language, 4.7 Ordering Mode Declaration</a>
         * @see <a href="http://www.w3.org/TR/xquery/#dt-ordering-mode">XQuery 1.0:
         * An XML Query Language, Definition: Ordering mode</a>
         */
        enum OrderingMode
        {
            Ordered,
            Unordered
        };

        /**
         * @see <a href="http://www.w3.org/TR/xquery/#id-empty-order-decl">XQuery 1.0:
         * An XML Query Language, 4.8 Empty Order Declaration</a>
         * @see <a href="http://www.w3.org/TR/xquery/#dt-default-empty-order">XQuery 1.0:
         * An XML Query Language, Definition: Default order for empty sequences</a>
         */
        enum OrderingEmptySequence
        {
            Greatest,
            Least
        };

        enum InheritMode
        {
            Inherit,
            NoInherit
        };

        enum PreserveMode
        {
            Preserve,
            NoPreserve
        };

        inline StaticContext()
        {
        }

        virtual ~StaticContext();

        virtual NamespaceResolver::Ptr namespaceBindings() const = 0;
        virtual void setNamespaceBindings(const NamespaceResolver::Ptr &) = 0;
        virtual PlainSharedPtr<FunctionFactory> functionSignatures() const = 0;
        virtual PlainSharedPtr<SchemaTypeFactory> schemaDefinitions() const = 0;

        /**
         * The base URI of the context. Typically, this is the base URI
         * if of the element that contained the expression.
         *
         * The base URI is in this implementation is never undefined, but is
         * always valid.
         */
        virtual QUrl baseURI() const = 0;

        virtual void setBaseURI(const QUrl &uri) = 0;

        /**
         * @returns always the standard function namespace defined in
         * <a href="http://www.w3.org/TR/xpath-functions/">XQuery 1.0 and
         * XPath 2.0 Functions and Operators</a>
         */
        virtual QString defaultFunctionNamespace() const = 0;
        virtual void setDefaultFunctionNamespace(const QString &ns) = 0;

        virtual QString defaultElementNamespace() const = 0;
        virtual void setDefaultElementNamespace(const QString &ns) = 0;

        /**
         * @returns the URI identifying the default collation. The function
         * is responsible for ensuring a collation is always returned. If
         * a collation is not provided by the user or the host language in the
         * context, the Unicode codepoint URI should be returned.
         */
        virtual QUrl defaultCollation() const = 0;

        virtual void setDefaultCollation(const QUrl &uri) = 0;

        /**
         * Determine whether Backwards Compatible Mode is used.
         *
         * @see <a href="http://www.w3.org/TR/xpath20/#id-backwards-compatibility">XML Path
         * Language (XPath) 2.0, I Backwards Compatibility with XPath 1.0 (Non-Normative)</a>
         * @see <a href="http://www.w3.org/TR/xpath20/#dt-xpath-compat-mode">XML Path
         * Language (XPath) 2.0, Definition: XPath 1.0 compatibility mode</a>
         */
        virtual bool compatModeEnabled() const = 0;

        /**
         * This is the DynamicContext that is used for pre-evaluation at
         * compilation time, const-folding at the static stage.
         */
        virtual PlainSharedPtr<DynamicContext> dynamicContext() const = 0;

        virtual BoundarySpacePolicy boundarySpacePolicy() const = 0;
        virtual void setBoundarySpacePolicy(const BoundarySpacePolicy policy) = 0;

        virtual ConstructionMode constructionMode() const = 0;
        virtual void setConstructionMode(const ConstructionMode mode) = 0;

        virtual OrderingMode orderingMode() const = 0;
        virtual void setOrderingMode(const OrderingMode mode) = 0;
        virtual OrderingEmptySequence orderingEmptySequence() const = 0;
        virtual void setOrderingEmptySequence(const OrderingEmptySequence ordering) = 0;

        virtual InheritMode inheritMode() const = 0;
        virtual void setInheritMode(const InheritMode mode) = 0;

        virtual PreserveMode preserveMode() const = 0;
        virtual void setPreserveMode(const PreserveMode mode) = 0;

        /**
         * @short The static type of the context item.
         *
         * Different StaticContext instances are used for different nodes in the
         * AST to properly reflect the type of the focus. If the focus is undefined,
         * this function must return @c null.
         *
         * @see <a href="http://www.w3.org/TR/xquery/#dt-context-item-static-type">XQuery
         * 1.0: An XML Query Language, Definition: Context item static type</a>
         */
        virtual ItemType::Ptr contextItemType() const = 0;

        /**
         * Copies this StaticContext and returns the copy.
         *
         * The copy and original must not be independent. Since the StaticContext is modified
         * during the compilation process, the copy must be independent from the original
         * to the degree that is required for the subclass in question.
         */
        virtual StaticContext::Ptr copy() const = 0;

        virtual ExternalVariableLoader::Ptr externalVariableLoader() const = 0;
        virtual ResourceLoader::Ptr resourceLoader() const = 0;
        virtual NamePool::Ptr namePool() const = 0;

        /**
         * @short Adds @p location for @p expression.
         */
        virtual void addLocation(const SourceLocationReflection *const reflection,
                                 const QSourceLocation &location) = 0;

        /**
         * @short Changes the source location for @p from, to be to associated
         * to @p to.
         *
         * This is useful when rewriting one Expression into another one.
         */
        /*
        virtual void moveLocation(const PlainSharedPtr<Expression> &from,
                                  const PlainSharedPtr<Expression> &to) = 0;
                                  */

        /**
         * @short Makes the location applying for @p existing, also apply for
         * @p newExpr.
         * @short Makes the location applying for @p existing, also apply for
         * @p newExpr.
         */
        /*
        virtual void duplicateLocation(const PlainSharedPtr<Expression> &existing,
                                       const PlainSharedPtr<Expression> &newExpr) = 0;
                                       */

        /**
         * @short Removes the register location for @p target.
         */
        //virtual void deleteLocation(const PlainSharedPtr<Expression> &target) = 0;

        /**
         * @short Returns a hash of the contained locations.
         *
         * The key is the address for the expression, and the value is its location. Note
         * that the key cannot be dereferenced, there's no guarantee the
         * Expression is in scope. The key is merely an identifier.
         */
        virtual LocationHash sourceLocations() const = 0;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
