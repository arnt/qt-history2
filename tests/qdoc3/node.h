/*
  node.h
*/

#ifndef NODE_H
#define NODE_H

#include <qmap.h>
#include <qstringlist.h>

#include "codechunk.h"
#include "doc.h"
#include "location.h"

class InnerNode;

class Node
{
public:
    enum Type { Namespace, Class, Fake, Enum, Typedef, Function, Property };
    enum Access { Public, Protected, Private };
    enum Status { Obsolete, Deprecated, Preliminary, Commendable }; // don't reorder
    enum ThreadSafeness { UnspecifiedSafeness, NonReentrant, Reentrant, ThreadSafe };

    virtual ~Node();

    void setAccess( Access access ) { acc = access; }
    void setLocation( const Location& location ) { loc = location; }
    void setDoc( const Doc& doc, bool replace = false );
    void setStatus( Status status ) { sta = status; }
    void setThreadSafeness(ThreadSafeness safeness) { saf = safeness; }

    virtual bool isInnerNode() const = 0;
    Type type() const { return typ; }
    InnerNode *parent() const { return par; }
    const QString& name() const { return nam; }

    Access access() const { return acc; }
    const Location& location() const { return loc; }
    const Doc& doc() const { return d; }
    Status status() const { return sta; }
    Status inheritedStatus() const;
    ThreadSafeness threadSafeness() const;
    ThreadSafeness inheritedThreadSafeness() const;

protected:
    Node( Type type, InnerNode *parent, const QString& name );

private:
    Type typ : 3;
    Access acc : 2;
    Status sta : 2;
    ThreadSafeness saf : 2;
    InnerNode *par;
    QString nam;
    Location loc;
    Doc d;
};

class FunctionNode;

typedef QList<Node *> NodeList;

class InnerNode : public Node
{
public:
    ~InnerNode();

    Node *findNode( const QString& name );
    Node *findNode( const QString& name, Type type );
    FunctionNode *findFunctionNode( const QString& name );
    FunctionNode *findFunctionNode( const FunctionNode *clone );
    void addInclude( const QString& include );
    void setOverload( const FunctionNode *func, bool overlode );
    void normalizeOverloads();
    void deleteChildren();

    virtual bool isInnerNode() const;
    const Node *findNode( const QString& name ) const;
    const Node *findNode( const QString& name, Type type ) const;
    const FunctionNode *findFunctionNode( const QString& name ) const;
    const FunctionNode *findFunctionNode( const FunctionNode *clone ) const;
    const NodeList& childNodes() const { return children; }
    int overloadNumber( const FunctionNode *func ) const;
    int numOverloads( const QString& funcName ) const;
    const QStringList& includes() const { return inc; }

protected:
    InnerNode( Type type, InnerNode *parent, const QString& name );

private:
    friend class Node;

    static bool isSameSignature( const FunctionNode *f1, const FunctionNode *f2 );
    void addChild( Node *child );
    void removeChild( Node *child );

    QStringList inc;
    NodeList children;
    QMap<QString, Node *> childMap;
    QMap<QString, Node *> primaryFunctionMap;
    QMap<QString, NodeList> secondaryFunctionMap;
};

class LeafNode : public Node
{
public:
    LeafNode();

    virtual bool isInnerNode() const;

protected:
    LeafNode( Type type, InnerNode *parent, const QString& name );
};

class NamespaceNode : public InnerNode
{
public:
    NamespaceNode( InnerNode *parent, const QString& name );
};

class ClassNode;

struct RelatedClass
{
    Node::Access access;
    ClassNode *node;
    QString templateArgs;

    RelatedClass() { }
    RelatedClass( Node::Access access0, ClassNode *node0,
		  const QString& templateArgs0 = "" )
	: access( access0 ), node( node0 ), templateArgs( templateArgs0 ) { }
};

class ClassNode : public InnerNode
{
public:
    ClassNode( InnerNode *parent, const QString& name );

    void addBaseClass(Access access, ClassNode *node, const QString &templateArgs = "");

    const QList<RelatedClass> &baseClasses() const { return bas; }
    const QList<RelatedClass> &derivedClasses() const { return der; }

private:
    QList<RelatedClass> bas;
    QList<RelatedClass> der;
};

class FakeNode : public InnerNode
{
public:
    enum SubType { File, Group, Module, Page };

    FakeNode( InnerNode *parent, const QString& name, SubType subType );

    SubType subType() const { return sub; }

private:
    SubType sub;
};

class EnumItem
{
public:
    EnumItem() { }
    EnumItem( const QString& name, const QString& value )
	: nam( name ), val( value ) { }

    const QString& name() const { return nam; }
    const QString& value() const { return val; }

private:
    QString nam;
    QString val;
};

class EnumNode : public LeafNode
{
public:
    EnumNode( InnerNode *parent, const QString& name );

    void addItem( const EnumItem& item );

    const QList<EnumItem>& items() const { return itms; }
    Access itemAccess( const QString& name ) const;

private:
    QList<EnumItem> itms;
};

class TypedefNode : public LeafNode
{
public:
    TypedefNode( InnerNode *parent, const QString& name );
};

class Parameter
{
public:
    Parameter() { }
    Parameter( const QString& leftType, const QString& rightType = "",
	       const QString& name = "", const QString& defaultValue = "" );
    Parameter( const Parameter& p );

    Parameter& operator=( const Parameter& p );

    void setName( const QString& name ) { nam = name; }

    bool hasType() const { return lef.length() + rig.length() > 0; }
    const QString& leftType() const { return lef; }
    const QString& rightType() const { return rig; }
    const QString& name() const { return nam; }
    const QString& defaultValue() const { return def; }

private:
    QString lef;
    QString rig;
    QString nam;
    QString def;
};

class PropertyNode;

class FunctionNode : public LeafNode
{
public:
    enum Metaness { Plain, Signal, Slot, Ctor, Dtor };
    enum Virtualness { NonVirtual, ImpureVirtual, PureVirtual };

    FunctionNode( InnerNode *parent, const QString& name );

    void setReturnType( const QString& returnType ) { rt = returnType; }
    void setMetaness( Metaness metaness ) { met = metaness; }
    void setVirtualness( Virtualness virtualness ) { vir = virtualness; }
    void setConst( bool conste ) { con = conste; }
    void setStatic( bool statique ) { sta = statique; }
    void setOverload( bool overlode );
    void addParameter( const Parameter& parameter );
    inline void setParameters( const QList<Parameter>& parameters );
    void borrowParameterNames( const FunctionNode *source );
    void setReimplementedFrom( FunctionNode *from );
    void setAssociatedProperty( PropertyNode *property );

    const QString& returnType() const { return rt; }
    Metaness metaness() const { return met; }
    Virtualness virtualness() const { return vir; }
    bool isConst() const { return con; }
    bool isStatic() const { return sta; }
    bool isOverload() const { return ove; }
    int overloadNumber() const;
    int numOverloads() const;
    const QList<Parameter>& parameters() const { return params; }
    QStringList parameterNames() const;
    const FunctionNode *reimplementedFrom() const { return rf; }
    const QList<FunctionNode *> &reimplementedBy() const { return rb; }
    const PropertyNode *associatedProperty() const { return ap; }

private:
    friend class InnerNode;

    QString rt;
    Metaness met : 3;
    Virtualness vir : 2;
    bool con : 1;
    bool sta : 1;
    bool ove : 1;
    QList<Parameter> params;
    const FunctionNode *rf;
    const PropertyNode *ap;
    QList<FunctionNode *> rb;
};

class PropertyNode : public LeafNode
{
public:
    enum FunctionRole { Getter, Setter, Resetter, NumFunctionRoles };

    PropertyNode( InnerNode *parent, const QString& name );

    void setDataType( const QString& dataType ) { dt = dataType; }
    void setFunction( const FunctionNode *function, FunctionRole role );
    void setStored( bool stored ) { sto = toTrool( stored ); }
    void setDesignable( bool designable ) { des = toTrool( designable ); }

    const QString& dataType() const { return dt; }
    const FunctionNode *function(FunctionRole role) const { return funcs[(int)role]; }
    const FunctionNode *getter() const { return function(Getter); }
    const FunctionNode *setter() const { return function(Setter); }
    const FunctionNode *resetter() const { return function(Resetter); }
    bool isStored() const { return fromTrool( sto, storedDefault() ); }
    bool isDesignable() const { return fromTrool( des, designableDefault() ); }

private:
    enum Trool { Trool_True, Trool_False, Trool_Default };

    static Trool toTrool( bool boolean );
    static bool fromTrool( Trool troolean, bool defaultValue );

    bool storedDefault() const { return true; }
    bool designableDefault() const { return setter() != 0; }

    QString dt;
    const FunctionNode *funcs[(int)NumFunctionRoles];
    Trool sto : 2;
    Trool des : 2;
};

inline void FunctionNode::setParameters(const QList<Parameter> &parameters)
{
    params = parameters;
}

inline void PropertyNode::setFunction(const FunctionNode *function, FunctionRole role)
{
    funcs[(int)role] = function;
}

#endif
