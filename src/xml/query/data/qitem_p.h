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

#ifndef Patternist_Item_H
#define Patternist_Item_H

class QSourceLocation;
template<typename T> class QList;
template<typename T> class QVector;

#include <QUrl>
#include <QVariant>

#include "qitemtype_p.h"
#include "qlistiterator_p.h"
#include "qsingletoniterator_p.h"
#include "qplainsharedptr_p.h"

/**
 * @file
 * @short Due to strong interdependencies, this file contains the definitions for
 * the classes Item, Node, NodeModel and AtomicValue. The implementations are
 * in their respective source files.
 */

/**
 * @class QSharedData
 * @short Qt's base class for reference counting.
 * @author Trolltech
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    class DynamicContext;
    class Item;
    class ItemType;
    class NodeModel;
    class QObjectNodeModel;
    class SequenceReceiver;
    template<typename T> class EmptyIterator;
    template<typename T> class Iterator;
    template<typename T> class Iterator;
    template<typename T> class ListIterator;

    /**
     * @short Represents an XML node in the XPath Data Model.
     *
     * Node represent a node. It cannot be used directly, but must
     * be sub-classed.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Q_AUTOTEST_EXPORT Node
    {
    public:
        typedef qint32 Data;

        /**
         * An integer that uniquely identifies a Node.
         */
        typedef Data Identity;
        typedef quint32 AdditionalData;

        /**
         *
         * @see kind()
         */
        enum NodeKind
        {
            Attribute               = 1,
            Comment                 = 2,
            Document                = 4,
            Element                 = 8,
            Namespace               = 16,
            ProcessingInstruction   = 32,
            Text                    = 64
        };


        /**
         * @short Identifies what specific node comparison operator that should be used.
         */
        enum DocumentOrder
        {
            /**
             * Signifies the <tt>\<\<</tt> operator. Whether the first operand precedes
             * the second operand in document order.
             */
            Precedes,

            /**
             * Signifies the <tt>\>\></tt> operator. Whether the first operand follows
             * the second operand in document order.
             */
            Follows,

            /**
             * Signifies the <tt>is</tt> operator. Whether two nodes have the same node
             * identity.
             */
            Is
        };

        /**
         * Identify axes. An axis navigates from a node in a way specific to that axis.
         *
         * The axes Child, Descendant, Attribute, Self, DescendantOrSelf, FollowingSibling,
         * and Following are forward axes.
         *
         * The axes Parent, Ancestor, PrecedingSibling, Preceding and AncestorOrSelf are reverse axes.
         *
         * @see iterate()
         */
        enum Axis
        {
            /**
             * All axes that are forward axes have this value OR'd in.
             */
            ForwardAxis         = 8192,

            /**
             * All axes that are reverse axes have this value OR'd in.
             */
            ReverseAxis         = 16384,

            /**
             * The <tt>child</tt> axis.
             */
            Child               = 1 | ForwardAxis,

            /**
             * The <tt>descendant</tt> axis.
             */
            Descendant          = 2 | ForwardAxis,

            /**
             * The <tt>attribute</tt> axis. This enum value isn't named
             * "Attribute", in order to not clash with node kind by that name.
             */
            AttributeAxis       = 4 | ForwardAxis,

            /**
             * The <tt>self</tt> axis.
             */
            Self                = 8 | ForwardAxis,

            /**
             * The <tt>descendant-or-self</tt> axis.
             */
            DescendantOrSelf    = 16 | ForwardAxis,

            /**
             * The <tt>following-sibling</tt> axis.
             */
            FollowingSibling    = 32 | ForwardAxis,

            /**
             * The <tt>namespace</tt> axis. It does not exist in XQuery,
             * is deprecated in XPath 2.0(optionally supported), and mandatory in XPath 1.0.
             */
            NamespaceAxis       = 64 | ForwardAxis,

            /**
             * The <tt>following</tt> axis.
             */
            Following           = 128 | ReverseAxis,

            /**
             * The <tt>parent</tt> axis.
             */
            Parent              = 256 | ReverseAxis,

            /**
             * The <tt>ancestor</tt> axis.
             */
            Ancestor            = 512 | ReverseAxis,

            /**
             * The <tt>preceding-sibling</tt> axis.
             */
            PrecedingSibling    = 1024 | ReverseAxis,

            /**
             * The <tt>preceding</tt> axis.
             */
            Preceding           = 2048 | ReverseAxis,

            /**
             * The <tt>ancestor-or-self</tt> axis.
             */
            AncestorOrSelf      = 4096 | ReverseAxis
        };

        typedef QFlags<Axis> Axes;
        inline Data data() const
        {
            return m_data;
        }

        inline const void *internalPointer() const
        {
            return m_ptr;
        }

        static QString axisName(const Axis axis);

        /* Delegators onto NodeModel's members. */
        inline QName name() const;
        inline Node parent() const;
        inline Node root() const;
        inline PlainSharedPtr<Iterator<Item> > iterate(const Axis axis) const;
        inline PlainSharedPtr<Iterator<Item> > typedValue() const;
        inline QUrl documentURI() const;
        inline QUrl baseURI() const;
        inline Identity identity() const;
        inline NodeKind kind() const;
        inline bool isDeepEqual(const Node other) const;
        inline DocumentOrder compareOrderTo(const Node other) const;
        inline void sendNamespaces(const PlainSharedPtr<SequenceReceiver> &receiver) const;
        inline NamespaceBinding::Vector namespaceBindings() const;
        inline QName::NamespaceCode namespaceForPrefix(const QName::PrefixCode prefix) const;
        inline QString stringValue() const;
        inline ItemType::Ptr type() const;
        inline bool is(const Node other) const;

        inline operator bool() const
        {
            return m_model;
        }

        inline bool operator!() const
        {
            return !m_model;
        }

        inline AdditionalData additionalData() const
        {
            return m_additionalData;
        }

    private:
        static inline Node create(const Data d,
                                  const NodeModel *const nm)
        {
            Node n;
            n.m_data = d;
            n.m_model = nm;
            n.m_additionalData = 0;
            return n;
        }

        static inline Node create(const void *const pointer,
                                  const NodeModel *const nm,
                                  const AdditionalData addData)
        {
            Node n;
            n.m_ptr = pointer;
            n.m_model = nm;
            n.m_additionalData = addData;
            return n;
        }

        friend class NodeModel;
        friend class Item;
        static bool isDeepEqual(const Node n1, const Node n2);
        inline operator int() const; // Disable

        union
        {
            const void *m_ptr;
            Data m_data;
        };

        AdditionalData m_additionalData;
        const NodeModel *m_model;
    };

    /**
     * @short Base class for all classes representing atomic values.
     *
     * Instantiating AtomicValues sub classes from a value of somekind,
     * for a certain type is done in three different ways:
     *
     * - The static factory fromLexical which available in most classes. This
     * function attempts to create a value from a QString that is considered
     * a lexical representation of the value. Thus, this function performs validation, takes
     * care of whitespace facets, and everything else related to instantiating a value from
     * a lexical representation.
     * - The static factory function fromValue. This function exists for
     * values where a C++ type exists which corresponds to the type's value space.
     * - By using instances available in CommonValues. This is the preferred method
     * since it uses existing singleton instances and thus saves memory. CommonValues
     * should be used whenever possible, it can be thought as a collection of constant values.
     *
     * For types that does not distinguish the value space and lexical space, such as <tt>xs:string</tt>,
     * only the fromValue() function exist, and fromLexical() is omitted.
     *
     * @include Example-AtomicValue-qboolean.cpp
     *
     * This example demonstrates different ways of creating AtomicValue instances for <tt>xs:boolean</tt>.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AtomicValue : public QSharedData
    {
    public:
        virtual ~AtomicValue();

        /**
         * A smart pointer wrapping AtomicValue instances.
         */
        typedef PlainSharedPtr<AtomicValue> Ptr;

        /**
         * Determines whether this atomic value has an error. This is used
         * for implementing casting.
         *
         * @returns always @c false
         */
        virtual bool hasError() const;

        /**
         * Always fails by issuing the type error ReportContext::FORG0006. Sub-classes
         * whose represented type do allow EBV to be extracted from, must thus
         * re-implement this function.
         */
        virtual bool evaluateEBV(const PlainSharedPtr<DynamicContext> &context) const;

        virtual QString stringValue() const = 0;
        virtual ItemType::Ptr type() const = 0;

        /**
         * Converts @p value to a QVariant.
         */
        static QVariant toQt(const AtomicValue *const value);

        static inline QVariant toQt(const AtomicValue::Ptr &value)
        {
            return toQt(value.get());
        }

        static Item toXDM(const QVariant &value,
                          const QObjectNodeModel *const nm);

    protected:
        inline AtomicValue()
        {
        }
    };

    /**
     * @short Represents an item in the XPath 2.0 Data Model.
     *
     * There exists two types of items: nodes and atomic values.
     *
     * The XQuery 1.0 and XPath 2.0 Data Model and XML Path Language (XPath) 2.0 specification
     * makes a very strong distinction between a sequence of items and an atomized sequence.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class Item
    {
    public:
        /**
         * A smart pointer wrapping an Item instance.
         */
        typedef Iterator<Item> Iterator;
        typedef ListIterator<Item> ListIterator;

        /**
         * A list of Item instances, each wrapped in a smart pointer.
         */
        typedef QList<Item> List;

        /**
         * A vector of Item instances, each wrapped in a smart pointer.
         */
        typedef QVector<Item> Vector;

        typedef SingletonIterator<Item> SingletonIterator;
        typedef EmptyIterator<Item> EmptyIterator;

        /**
         * Default constructor.
         */
        inline Item()
        {
            /* This is the area which atomicValue uses. Becauase we want as()
             * to return null on null-constructed objects, we initialize it. */
            node.m_data = 0;

            /* This signals that we're not an atomic value. */
            node.m_model = 0;
        }

        inline Item(const Node n) : node(n)
        {
        }

        inline Item(const Item &other) : node(other.node)

        {
            Q_ASSERT_X(sizeof(Node) >= sizeof(AtomicValue), Q_FUNC_INFO,
                       "Since we're only copying the node member, it must be the largest.");
            if(isAtomicValue())
                atomicValue->ref.ref();
        }

        inline Item(const AtomicValue::Ptr &a)
        {
            if(a)
            {
                atomicValue = a.get();
                atomicValue->ref.ref();

                /* Signal that we're housing an atomic value. */
                node.m_model = reinterpret_cast<const NodeModel *>(~0);
            }
            else
                node.m_model = 0; /* Like the default constructor. */
        }

        inline Item(AtomicValue *const a)
        {
            /* Note, the implementation is a copy of the constructor above. */

            if(a)
            {
                atomicValue = a;
                atomicValue->ref.ref();

                /* Signal that we're housing an atomic value. */
                node.m_model = reinterpret_cast<const NodeModel *>(~0);
            }
            else
                node.m_model = 0; /* Like the default constructor. */
        }

        inline ~Item()
        {
            if(isAtomicValue())
                atomicValue->ref.deref();
        }

        inline Item &operator=(const Item &other)
        {
            Q_ASSERT_X(sizeof(Node) >= sizeof(AtomicValue *), Q_FUNC_INFO,
                       "If this doesn't hold, we won't copy all data.");

            if(other.isAtomicValue())
                other.atomicValue->ref.ref();

            if(isAtomicValue())
            {
                if(!atomicValue->ref.deref())
                    delete atomicValue;
            }

            node = other.node;

            return *this;
        }

        template<typename TCastTarget>
        inline TCastTarget *as() const
        {
            Q_ASSERT_X(atomicValue == 0 || dynamic_cast<TCastTarget *>(atomicValue),
                       Q_FUNC_INFO,
                       "The cast is invalid. This class does not inherit the cast target.");
            return static_cast<TCastTarget *>(atomicValue);
        }

        /**
         * @short Returns the string value of this Item.
         *
         * In the case of a node, it is the node value corresponding to
         * the particular node type. For atomic values, it is equivalent
         * to the value cast as <tt>xs:string</tt>.
         *
         * Conceptually, this functions corresponds to the <tt>dm:string-value</tt> accessor.
         *
         * @see <a href="http://www.w3.org/TR/xpath-datamodel/#dm-string-value">XQuery 1.0 and
         * XPath 2.0 Data Model, 5.13 string-value Accessor</a>
         * @returns the string value.
         */
        inline QString stringValue() const
        {
            if(isAtomicValue())
                return atomicValue->stringValue();
            else
                return node.stringValue();
        }

        /**
         * @short Returns the typed value of this item.
         *
         * Conceptually, this functions corresponds to the <tt>dm:typed-value</tt> accessor. Here are
         * examples of what the typed value of an Item is:
         *
         * - The typed value of an atomic value is always the atomic value itself.
         * - A comment node has always a typed value of type @c xs:string
         * - For attribute and element nodes, the typed value can be arbitrary. For example, an
         *   element can have a sequence of @c xs:dateTime instances.
         *
         * @returns the typed value of this item
         * @see <a href="http://www.w3.org/TR/xpath-datamodel/#dm-typed-value">XQuery 1.0 and
         * XPath 2.0 Data Model, 5.15 typed-value Accessor</a>
         */
        Item::Iterator::Ptr typedValue() const;

        /**
         * @short Determines whether this item is an atomic value, or a node.
         *
         * @see isNode()
         * @returns @c true if it is an atomic value, otherwise @c false.
         */
        inline bool isAtomicValue() const
        {
            // TODO Document all this shit.
            return node.m_model == reinterpret_cast<NodeModel *>(~0);
        }

        /**
         * @short Determines whether this item is an atomic value, or a node.
         *
         * This returns the opposite of isAtomicValue, and is provided for readability.
         *
         * @see isAtomicValue()
         * @returns @c true if this item is a node, otherwise @c false.
         */
        inline bool isNode() const
        {
            return !isAtomicValue();
        }

        /**
         * @short Returns the ItemType this Item is of.
         *
         * For example, if this Item is an XML node, more specifically a text node,
         * <tt>text()</tt> is returned. That is, BuiltinTypes::text. However, if this
         * Item is an atomic value of type <tt>xs:long</tt> that is what's returned,
         * BuiltinTypes::xsLong.
         *
         * @returns the type of this Item.
         */
        inline PlainSharedPtr<ItemType> type() const
        {
            if(isAtomicValue())
                return atomicValue->type();
            else
                return node.type();
        }

        inline AtomicValue *asAtomicValue() const
        {
            Q_ASSERT(isAtomicValue());
            return atomicValue;
        }

        inline const Node &asNode() const
        {
            Q_ASSERT_X(node.m_model, Q_FUNC_INFO,
                       "This item isn't a valid Node.");
            return node;
        }

        inline operator bool() const
        {
            return node.m_model;
        }

        inline void reset()
        {
            node.m_model = 0;
        }

    private:
        union
        {
            Node node;
            AtomicValue *atomicValue;
        };
    };

    /**
     * @short Similar to QAbstractModelIndex, this is the central interface
     * for implementing a custom node tree.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class NodeModel : public QSharedData
    {
    public:
        typedef PlainSharedPtr<NodeModel> Ptr;
        typedef QList<Ptr> List;

        /**
         * @short Does nothing.
         *
         * This default constructor cannot be synthesized, due to that the
         * Q_DISABLE_COPY() macro is used.
         */
        inline NodeModel()
        {
        }

        virtual ~NodeModel();

        /**
         * @short Returns the base URI of @p ni.
         *
         * This function maps to the <tt>dm:base-uri</tt> accessor.
         *
         * @see <a href="http://www.w3.org/TR/xpath-datamodel/#dm-base-uri">XQuery 1.0
         * and XPath 2.0 Data Model (XDM), 5.2 base-uri Accessor</a>
         */
        virtual QUrl baseURI(const Node ni) const = 0;

        /**
         * @short Returns the document URI of @p ni.
         *
         * This function maps to the <tt>dm:document-uri</tt> accessor.
         *
         * The implementor is guaranteed, as consistent with the specification
         * for the accessor, to:
         * - If @p ni is a document node, return an absolute, valid QUrl
         *   containing the document URI, or a default constructed QUrl. The
         *   latter signals that no document URI is available for the document
         *   node.
         * - For all other kinds of nodes, return a default constructed QUrl.
         *
         * @see <a href="http://www.w3.org/TR/xpath-datamodel/#dm-document-uri">XQuery 1.0
         * and XPath 2.0 Data Model (XDM), 5.4 document-uri Accessor</a>
         */
        virtual QUrl documentURI(const Node ni) const = 0;

        /**
         * @short Determines what node kind @p ni is.
         *
         * This function maps to the <tt>dm:node-kind()</tt> accessor.
         *
         * @see <a href="http://www.w3.org/TR/xpath-datamodel/#dm-node-kind">XQuery 1.0
         * and XPath 2.0 Data Model (XDM), 5.10 node-kind Accessor</a>
         */
        virtual Node::NodeKind kind(const Node ni) const = 0;

        /**
         * @short Returns the relative document order between @p ni1 and @p
         * ni2.
         *
         * If @p ni1 is identical to @p ni2, Is is returned.
         * If @p ni1 precedes @p ni2 in document order, Precedes is
         * returned. If @p ni1 follows @p ni2 in document order, Follows
         * is returned.
         *
         * The caller guarantees that @p ni1 and @p ni2 are always non-null, valid nodes.
         */
        virtual Node::DocumentOrder compareOrderTo(const Node ni1, const Node ni2) const = 0;

        /**
         * @short Returns the root node of the tree @p n is part of. This is typically a document
         * node.
         *
         * If @p n is a direct child of the Node returned from this function, parent() would
         * return the same Node.
         */
        virtual Node root(const Node n) const = 0;

        /**
         * @short Returns the parent Node of @p ni.
         *
         * This function maps to the <tt>dm:parent()</tt> accessor.
         *
         * @see <a href="http://www.w3.org/TR/xpath-datamodel/#dm-parent">XQuery 1.0
         * and XPath 2.0 Data Model (XDM), 5.12 parent Accessor</a>
         * @returns the parent or @c null if this Node is parentless.
         */
        virtual Node parent(const Node ni) const = 0;

        /**
         * @short Performs navigation, starting from @p ni by returning an Iterator that
         * returns nodes that the Axis @p axis reaches from @p ni.
         *
         * @p axis is guaranteed by the caller to never be @p Parent, because
         * the @c parent axis is implemented with parent(). TODO this is not
         * true.
         *
         * For all axes, the implementor is guaranteed to return nodes that are
         * in document order and without duplicates.
         */
        virtual Item::Iterator::Ptr iterate(const Node ni, const Node::Axis axis) const = 0;

        /**
         * @short Returns the name of @p ni.
         *
         * If a node does not have a name, such as a comment code, a null QName is returned.
         *
         * This function maps to the <tt>dm:node-name()</tt> accessor.
         *
         * As specified, if  @p ni is a processing instruction, a QName is returned
         * where the local name is the target name and the namespace URI and prefix is empty.
         *
         * @see <a href="http://www.w3.org/TR/xpath-datamodel/#dm-node-name">XQuery 1.0 and
         * XPath 2.0 Data Model (XDM), 5.11 node-name Accessor</a>
         * @see QName
         */
        virtual QName name(const Node ni) const = 0;

        /**
         * @short Returns the in-scope namespaces of @p n. This
         * corresponds to the <tt>dm:namespace-nodes</tt> accessor.
         *
         * @note This is not only the namespace declarations that appear on
         * this element, but takes also into account namespace bindings of the
         * ancestors.
         *
         * The caller guarantees that @p n is an Element.
         */
        virtual NamespaceBinding::Vector namespaceBindings(const Node n) const = 0;

        /**
         * @short Sends the namespaces declared on @p n to @p receiver.
         *
         * As a consequence, no namespaces are sent except if this node is an
         * element and has namespaces declared.
         *
         * @param receiver the receiver that this node is supposed to send its
         * namespaces to. This is guaranteed by the caller to be a valid
         * pointer.
         */
        virtual void sendNamespaces(const Node n,
                                    const PlainSharedPtr<SequenceReceiver> &receiver) const = 0;

        /**
         * @short Same as Item::stringValue(), but applies on @p n.
         */
        virtual QString stringValue(const Node n) const = 0;

        /**
         * @short Same as Item::typedValue(), but applies on @p ni.
         */
        virtual Item::Iterator::Ptr typedValue(const Node ni) const = 0;

        /**
         * @short Same as Item::type(), but applies on @p ni.
         */
        virtual ItemType::Ptr type(const Node ni) const = 0;

        /**
         * @short Returns the namespace URI on @p ni that corresponds to @p prefix.
         *
         * If @p prefix is StandardPrefixes::empty, the namespace URI for the default
         * namespace is returned.
         *
         * The default implementation use namespaceBindings(), in a straight
         * forward manner.
         *
         * The caller guarantees to only call this function for element nodes.
         */
        virtual QName::NamespaceCode namespaceForPrefix(const Node ni,
                                                        const QName::PrefixCode prefix) const;


        /**
         * @shoort Determines whether @p ni1 is deep equal to @p ni2.
         *
         * isDeepEqual() is defined as evaluating the expression <tt>fn:deep-equal($n1, $n2)</tt> where
         * <tt>$n1</tt> is @p ni1 and <tt>$n1</tt> is @p ni2. This function is associative, meaning
         * the same value is returned regardless of if isDeepEqual() is invoked with @p ni1 as first
         * argument or second. It is guaranteed that @p ni1 and @p ni2 are nodes, as opposed to the
         * definition of <tt>fn:deep-equal()</tt>.
         *
         * @see <a href="http://www.w3.org/TR/xpath-functions/#func-deep-equal">XQuery 1.0
         * and XPath 2.0 Functions and Operators, 15.3.1 fn:deep-equal</a>
         * @param other the Node to compare with. It is guaranteed by the caller to never be @c null.
         * @returns @c true if @p ni1 is deep-equal to @p ni2, otherwise @c false
         */
        virtual bool isDeepEqual(const Node ni1,
                                 const Node ni2) const;

    protected:
        static ItemType::Ptr typeFromKind(const Node::NodeKind nodeKind);

        inline Node createNode(const Node::Data d) const
        {
            return Node::create(d, this);
        }

        inline Node createNode(const void *const pointer,
                               const Node::AdditionalData data = 0) const
        {
            return Node::create(pointer, this, data);
        }

        inline Node createNullNode() const
        {
            return Node::create(0, 0);
        }

    private:
        static inline bool isIgnorable(const Node n);
        Q_DISABLE_COPY(NodeModel)
    };

    template<typename T>
    inline Item toItem(const PlainSharedPtr<T> atomicValue)
    {
        return Item(atomicValue.get());
    }

    inline QName Node::name() const
    {
        return m_model->name(*this);
    }

    inline Node Node::parent() const
    {
        return m_model->parent(*this);
    }

    inline Node Node::root() const
    {
        return m_model->root(*this);
    }

    inline Item::Iterator::Ptr Node::iterate(const Node::Axis axis) const
    {
        return m_model->iterate(*this, axis);
    }

    inline QUrl Node::documentURI() const
    {
        return m_model->documentURI(*this);
    }

    inline QUrl Node::baseURI() const
    {
        return m_model->baseURI(*this);
    }

    inline Node::Identity Node::identity() const
    {
        return m_data;
    }

    inline Node::NodeKind Node::kind() const
    {
        return m_model->kind(*this);
    }

    inline bool Node::isDeepEqual(const Node other) const
    {
        return m_model->isDeepEqual(*this, other);
    }

    inline Node::DocumentOrder Node::compareOrderTo(const Node other) const
    {
        return m_model->compareOrderTo(*this, other);
    }

    inline bool Node::is(const Node other) const
    {
        return m_model == other.m_model &&
               m_data == other.m_data &&
               m_additionalData == other.m_additionalData;
    }

    inline void Node::sendNamespaces(const PlainSharedPtr<SequenceReceiver> &receiver) const
    {
        m_model->sendNamespaces(*this, receiver);
    }

    inline NamespaceBinding::Vector Node::namespaceBindings() const
    {
        return m_model->namespaceBindings(*this);
    }

    inline QName::NamespaceCode Node::namespaceForPrefix(const QName::PrefixCode prefix) const
    {
        return m_model->namespaceForPrefix(*this, prefix);
    }

    inline QString Node::stringValue() const
    {
        return m_model->stringValue(*this);
    }

    inline ItemType::Ptr Node::type() const
    {
        return m_model->type(*this);
    }

    inline PlainSharedPtr<Iterator<Item> > Node::typedValue() const
    {
        return m_model->typedValue(*this);
    }

    /**
     * This is an overload, provided for convenience.
     * @relates Node
     */
    static inline QString formatData(const Node node)
    {
        return node.stringValue(); // This can be improved a lot.
    }
}

Q_DECLARE_TYPEINFO(Patternist::Item::Iterator::Ptr, Q_MOVABLE_TYPE);

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
