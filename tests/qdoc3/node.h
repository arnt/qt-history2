/*
  node.h
*/

#ifndef NODE_H
#define NODE_H

#include <qmap.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "codechunk.h"
#include "doc.h"
#include "location.h"

class InnerNode;

class Node
{
public:
    enum Type { Namespace, Class, Fake, Enum, Typedef, Function, Property };
    enum Access { Public, Protected, Private };

    // the order is important for inheritedStatus()
    enum Status { Commendable, Preliminary, Deprecated, Obsolete };

    virtual ~Node();

    void setAccess( Access access ) { acc = access; }
    void setLocation( const Location& location ) { loc = location; }
    void setDoc( const Doc& doc, bool replace = FALSE );
    void setStatus( Status status ) { sta = status; }

    virtual bool isInnerNode() const = 0;
    Type type() const { return typ; }
    InnerNode *parent() const { return par; }
    const QString& name() const { return nam; }

    Access access() const { return acc; }
    const Location& location() const { return loc; }
    const Doc& doc() const { return d; }
    Status status() const { return sta; }
    Status inheritedStatus() const;

protected:
    Node( Type type, InnerNode *parent, const QString& name );

private:
    Type typ;
    InnerNode *par;
    QString nam;
    Access acc;
    Location loc;
    Doc d;
    Status sta;
};

class FunctionNode;

typedef QValueList<Node *> NodeList;

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

    virtual bool isInnerNode() const;
    const Node *findNode( const QString& name ) const;
    const Node *findNode( const QString& name, Type type ) const;
    const FunctionNode *findFunctionNode( const QString& name ) const;
    const FunctionNode *findFunctionNode( const FunctionNode *clone ) const;
    const NodeList& childNodes() const { return children; }
    int overloadNumber( const FunctionNode *func ) const;
    const QStringList& includes() const { return inc; }

protected:
    InnerNode( Type type, InnerNode *parent, const QString& name );

private:
    friend class Node;

    static bool isSameSignature( const FunctionNode *f1,
				 const FunctionNode *f2 );
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

    void addBaseClass( Access access, ClassNode *node,
		       const QString& templateArgs );

    const QValueList<RelatedClass>& baseClasses() const { return bas; }
    const QValueList<RelatedClass>& derivedClasses() const { return der; }

private:
    QValueList<RelatedClass> bas;
    QValueList<RelatedClass> der;
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

    const QValueList<EnumItem>& items() const { return itms; }
    Access itemAccess( const QString& name ) const;

private:
    QValueList<EnumItem> itms;
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

class FunctionNode : public LeafNode
{
public:
    enum Metaness { Plain, Signal, Slot };
    enum Virtualness { NonVirtual, ImpureVirtual, PureVirtual };

    FunctionNode( InnerNode *parent, const QString& name );

    void setReturnType( const QString& returnType ) { rt = returnType; }
    void setMetaness( Metaness metaness ) { met = metaness; }
    void setVirtualness( Virtualness virtualness ) { vir = virtualness; }
    void setConst( bool conste ) { con = conste; }
    void setStatic( bool statique ) { sta = statique; }
    void setOverload( bool overlode );
    void setReimplementation( bool reimp ) { rei = reimp; }
    void addParameter( const Parameter& parameter );
    void borrowParameterNames( const FunctionNode *source );
    void setReimplementedFrom( FunctionNode *from );

    const QString& returnType() const { return rt; }
    Metaness metaness() const { return met; }
    Virtualness virtualness() const { return vir; }
    bool isConst() const { return con; }
    bool isStatic() const { return sta; }
    bool isConstructor() const;
    bool isDestructor() const;
    bool isOverload() const { return ove; }
    bool isReimplementation() const { return rei; }
    int overloadNumber() const;
    const QValueList<Parameter>& parameters() const { return params; }
    const FunctionNode *reimplementedFrom() const { return rf; }
    const QValueList<FunctionNode *>& reimplementedBy() const { return rb; }

private:
    friend class InnerNode;

    QString rt;
    Metaness met;
    Virtualness vir;
    bool con;
    bool sta;
    bool ove;
    bool rei;
    QValueList<Parameter> params;
    const FunctionNode *rf;
    QValueList<FunctionNode *> rb;
};

class PropertyNode : public LeafNode
{
public:
    PropertyNode( InnerNode *parent, const QString& name );

    void setDataType( const QString& dataType ) { dt = dataType; }
    void setGetter( const QString& getter ) { get = getter; }
    void setSetter( const QString& setter ) { set = setter; }
    void setResetter( const QString& resetter ) { reset = resetter; }
    void setStored( bool stored ) { sto = toTrool( stored ); }
    void setDesignable( bool designable ) { des = toTrool( designable ); }

    const QString& dataType() const { return dt; }
    const QString& getter() const { return get; }
    const QString& setter() const { return set; }
    const QString& resetter() const { return reset; }
    bool isStored() const { return fromTrool( sto, storedDefault() ); }
    bool isDesignable() const { return fromTrool( des, designableDefault() ); }

private:
    enum Trool { Trool_True, Trool_False, Trool_Default };

    static Trool toTrool( bool boolean );
    static bool fromTrool( Trool troolean, bool defaultValue );

    bool storedDefault() const { return TRUE; }
    bool designableDefault() const { return !get.isEmpty(); }

    QString dt;
    QString get;
    QString set;
    QString reset;
    Trool sto;
    Trool des;
};

#endif
