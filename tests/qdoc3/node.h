/*
  node.h
*/

#ifndef NODE_H
#define NODE_H

#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>

#include "codechunk.h"
#include "doc.h"
#include "location.h"

class InnerNode;

class Node
{
public:
    enum Type { Namespace, Class, Enum, Typedef, Function, Property };
    enum Access { Public, Protected, Private };

    // the order is important for inheritedStatus()
    enum Status { Approved, Preliminary, Deprecated, Obsolete };

    virtual ~Node();

    void setAccess( Access access ) { acc = access; }
    void setLocation( const Location& location ) { loc = location; }
    void setDoc( const Doc& doc );
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
    FunctionNode *findFunctionNode( const FunctionNode *clone );
    void addInclude( const QString& include );

    virtual bool isInnerNode() const;
    const NodeList& childNodes() const { return children; }
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
    QMap<QString, QValueList<Node *> > secondaryFunctionMap;
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
    const ClassNode *node;
    QString templateArgs;

    RelatedClass() { }
    RelatedClass( Node::Access access0, const ClassNode *node0,
		  const QString& templateArgs0 = "" )
	: access( access0 ), node( node0 ), templateArgs( templateArgs0 ) { }
};

struct ClassSection
{
    QString name;
    NodeList members;

    ClassSection() { }
    ClassSection( const QString& name0 )
	: name( name0 ) { }
};

class ClassNode : public InnerNode
{
public:
    ClassNode( InnerNode *parent, const QString& name );

    void addBaseClass( Access access, ClassNode *node,
		       const QString& templateArgs );

    QValueList<RelatedClass> baseClasses() const { return bas; }
    QValueList<RelatedClass> derivedClasses() const { return der; }
    QValueList<ClassSection> overviewSections() const;
    QValueList<ClassSection> detailedSections() const;

private:
    static void append( QValueList<ClassSection> *sectionList,
			const ClassSection& section );
    static NodeList nodeList( const QMap<QString, Node *>& map );

    QValueList<RelatedClass> bas;
    QValueList<RelatedClass> der;
};

class EnumItem
{
public:
    EnumItem( const QString& name, const QString& value );

private:
    QString nam;
    QString val;
};

class EnumNode : public LeafNode
{
public:
    EnumNode( InnerNode *parent, const QString& name );
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
    Parameter( const QString& leftType, const QString& rightType,
	       const QString& name = "", const QString& defaultValue = "" );
    Parameter( const Parameter& p );

    Parameter& operator=( const Parameter& p );

    void setName( const QString& name ) { nam = name; }

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
    void setOverloadNumber( int no ) { ove = no; }
    void addParameter( const Parameter& parameter );
    void borrowParameterNames( const FunctionNode *source );

    const QString& returnType() const { return rt; }
    Metaness metaness() const { return met; }
    Virtualness virtualness() const { return vir; }
    bool isConst() const { return con; }
    bool isStatic() const { return sta; }
    int overloadNumber() const { return ove; }
    const QValueList<Parameter>& parameters() const { return params; }

private:
    QString rt;
    Metaness met;
    Virtualness vir;
    bool con;
    bool sta;
    int ove;
    QValueList<Parameter> params;
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
