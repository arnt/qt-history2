/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
** Copyright (C) 2001-2004 Roberto Raggi
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef CODEMODEL_H
#define CODEMODEL_H

#include <QByteArray>
#include <QList>
#include <QMap>

#include "smallobject.h"
#include <ast.h>

namespace CodeModel
{

// types
struct Type;
struct EnumType;
struct EnumType;
struct ClassType;
struct BuiltinType;
struct PointerType;
struct ReferenceType;
struct GenericType;
struct AliasType;
struct FunctionType;
struct UnknownType;

// scope
struct Scope;
struct ClassScope;
struct NamespaceScope;
struct BlockScope;

// members
struct Member;
struct FunctionMember;
struct VariableMember;
struct UsingDeclarationMember; //using declaration
struct UsingDirectiveMember;   //using directive;
struct TypeMember; //for types declared in this scope
struct Argument;

// Declaration / use information
struct Declaration;
struct NameUse;

// collections
struct TypeCollection;
struct ScopeCollection;
struct MemberCollection;
struct ArgumentCollection;
struct NameUseCollection;

//builders
struct TypeCollectionBuilder;
struct ScopeCollectionBuilder;
struct MemberCollectionBuilder;
struct ArgumentCollectionBuilder;
struct NameUseCollectionBuilder;

struct SemanticInfo
{
    CodeModel::NamespaceScope *codeModel;

    // tokenindex -> NameUse* map. Use map here bacause we expect name uses to
    // be sparesly distributed among the tokens.
    QMap<int, NameUse*> nameUses;
};


struct Item
{
    Item()  {}
    virtual ~Item() {}

    virtual QByteArray name() const = 0;
    virtual Item *parent() const =0;
protected:
};

struct ItemCollection: public Item
{
    virtual QByteArray name() const
    { return QByteArray(); }

    virtual int count() const = 0;
    virtual Item *item(int index) const = 0;
    virtual int indexOf(Item *item) const = 0;
};

struct Type: public Item
{
    virtual QByteArray name() const =0;

    virtual EnumType *toEnumType() const
    { return 0; }

    virtual ClassType *toClassType() const
    { return 0; }

    virtual UnknownType *toUnknownType() const
    { return 0; }

    virtual BuiltinType *toBuiltinType() const
    { return 0; }

    virtual PointerType *toPointerType() const
    { return 0; }

    virtual ReferenceType *toReferenceType() const
    { return 0; }

    virtual GenericType *toGenericType() const
    { return 0; }

    virtual AliasType *toAliasType() const
    { return 0; }
};

struct Scope: public Item
{
    Scope();

    void setParent(Scope *parent)
    { m_parent = parent; }

    virtual Scope *parent() const
    { return m_parent; }

    virtual NamespaceScope *toNamespaceScope() const
    { return 0; }

    virtual ClassScope *toClassScope() const
    { return 0; }

    virtual BlockScope *toBlockScope() const
    { return 0; }

    virtual ScopeCollection *scopes() const;
    virtual TypeCollection *types() const;
    virtual MemberCollection *members() const;
    virtual NameUseCollection *nameUses() const;

    void addScope(Scope *scope);
    void addType(Type *type);
    void addMember(Member *member);
    void addNameUse(NameUse *nameUse);

    QList<Type*> findType(const QByteArray &name) ; // ### unfinished
    QList<Scope*> findScope(const QByteArray &name) ;
    QList<Member*> findMember(const QByteArray &name);
    QList<FunctionMember*> findFunctionMember(const QByteArray &name);
    QList<VariableMember*> findVariableMember(const QByteArray &name);

    // nestedNameSpecifier:  A,B, ... ,X, where A,B .. are scopes or classes,
    // or ::,A,B, ... ,X, to lookup from the global scope
    // returns a pointer to a the C scope if it can be found from currentScope,
    // 0 othervise
    Scope *lookUpScope( const QList<QByteArray> &nestedNameSpecifier);

private:
    Scope *m_parent;
    ScopeCollectionBuilder *m_scopes;
    TypeCollectionBuilder *m_types;
    MemberCollectionBuilder *m_members;
    NameUseCollectionBuilder *m_nameUses;
};




struct Member: public Item
{
    enum Binding // ### not used yet
    {
        Static,
        Instance
    };

    enum Access // ### not used yet
    {
        Public,
        Protected,
        Private
    };

    Member()
        : m_binding(Static), m_access(Public),
          m_parent(0), m_nameAST(0), m_constant(0), m_static(0)   {}

    QByteArray name() const
    { return m_name; }

    void setName(const QByteArray &name)
    { m_name = name; }

    AST * nameAST() const
    { return m_nameAST; }

    void setNameAST(AST *name)
    { m_nameAST = name; }

    Binding binding() const
    { return m_binding; }

    void setBinding(Binding binding)
    { m_binding = binding; }

    Access access() const
    { return m_access; }

    void setAccess(Access access)
    { m_access = access; }

    bool isConstant() const
    { return m_constant; }

    void setConstant(bool b)
    { m_constant = b; }

    bool isStatic() const
    { return m_static; }

    void setStatic(bool b)
    { m_static = b; }

    Scope *parent() const
    { return m_parent; }

    void setParent(Scope *parent)
    { m_parent = parent; }


    virtual FunctionMember *toFunctionMember() const
    { return 0; }

    virtual VariableMember *toVariableMember() const
    { return 0; }

    virtual UsingDeclarationMember *toUsingDeclarationMember() const
    { return 0; }

    virtual UsingDirectiveMember *toUsingDirectiveMember() const
    { return 0; }

    virtual TypeMember *toTypeMember() const
    { return 0; }

private:
    Binding m_binding;
    Access m_access;
    Scope *m_parent;
    QByteArray m_name;
    AST *m_nameAST;
    uint m_constant : 1;
    uint m_static : 1;
};

struct NamespaceScope: public Scope
{
    NamespaceScope() {}

    QByteArray name() const
    { return m_name; }

    void setName(const QByteArray &name)
    {
        m_name=name;
    }

    virtual NamespaceScope *toNamespaceScope() const
    { return const_cast<NamespaceScope*>(this); }

    /*
        Finds namespaces in the current scope
    */
    NamespaceScope * findNamespace(const QByteArray name);

private:
    QByteArray m_name;
};

struct ClassScope: public Scope
{
    ClassScope();

    TypeCollection *baseClasses() const;

    QByteArray name() const
    { return m_name; }

    void setName(const QByteArray &name)
    { m_name = name; }

    void addBaseClass(Type *baseClass);

    virtual ClassScope *toClassScope() const
    { return const_cast<ClassScope*>(this); }

private:
    QByteArray m_name;
    TypeCollectionBuilder *m_baseClasses;
};

struct BlockScope: public Scope
{
    BlockScope() {}

    QByteArray name() const
    { return m_name; }

    void setName(const QByteArray &name)
    { m_name = name; }

    virtual BlockScope *toBlockScope() const
    { return const_cast<BlockScope*>(this); }

private:
    QByteArray m_name;
};

struct EnumType: public Type
{
    EnumType() {}

    QByteArray name() const
    { return m_name; }

    void setName(const QByteArray &name)
    { m_name = name; }

    virtual Scope *parent() const
    { return m_parent; }

    void setParent(Scope *parent)
    { m_parent = parent; }

    virtual EnumType *toEnumType() const
    { return const_cast<EnumType*>(this); }

private:
    Scope *m_parent;
    QByteArray m_name;
};

struct UnknownType: public Type
{
    UnknownType()
        : m_parent(0) {}

    QByteArray name() const
    { return m_name; }

    void setName(const QByteArray &name)
    { m_name = name; }

    virtual Scope *parent() const
    { return m_parent; }

    void setParent(Scope *parent)
    { m_parent = parent; }

    virtual UnknownType *toUnknownType() const
    { return const_cast<UnknownType*>(this); }

private:
    Scope *m_parent;
    QByteArray m_name;
};

struct ClassType: public Type
{
    ClassType()
        : m_scope(0) {}

    ClassScope *scope() const
    { return m_scope; }

    void setScope(ClassScope *scope)
    { m_scope = scope; }

    virtual QByteArray name() const
    { return m_scope ? m_scope->name() : /*anonymous*/ QByteArray(); }

    virtual Scope *parent() const
    { return m_parent; }

    void setParent(Scope *parent)
    { m_parent = parent; }

    virtual ClassType *toClassType() const
    { return const_cast<ClassType*>(this); }

private:
    Scope *m_parent;
    ClassScope *m_scope;

};

struct BuiltinType: public Type
{
protected:
    BuiltinType(const QByteArray &name, Scope *parent)
        : m_name(name), m_parent(parent) {}

public:
    virtual QByteArray name() const
    { return m_name; }

    virtual Scope *parent() const
    { return m_parent; }

    virtual BuiltinType *toBuiltinType() const
    { return const_cast<BuiltinType*>(this); }

    static BuiltinType Bool;
    static BuiltinType Void;
    static BuiltinType Char;
    static BuiltinType Short;
    static BuiltinType Int;
    static BuiltinType Long;
    static BuiltinType Double;
    static BuiltinType Float;
    static BuiltinType Unsigned;
    static BuiltinType Signed;
    // ### more

private:
    QByteArray m_name;
    Scope *m_parent;
};

struct PointerType: public Type
{
    PointerType()
        : m_baseType(0) {}

    Type *baseType() const
    { return m_baseType; }

    void setBaseType(Type *baseType)
    { m_baseType = baseType; }

    virtual QByteArray name() const
    { return m_baseType->name(); }

    virtual Scope *parent() const
    { return m_parent; }

    void setParent(Scope *parent)
    { m_parent = parent; }

    virtual PointerType *toPointerType() const
    { return const_cast<PointerType*>(this); }

private:
    Scope *m_parent;
    Type *m_baseType;
};

struct ReferenceType: public Type
{
    ReferenceType()
        : m_parent(0), m_baseType(0) {}

    Type *baseType() const
    { return m_baseType; }

    void setBaseType(Type *baseType)
    { m_baseType = baseType; }

    virtual QByteArray name() const
    { return m_baseType->name(); }

    virtual Scope *parent() const
    { return m_parent; }

    void setParent(Scope *parent)
    { m_parent = parent; }

    virtual ReferenceType *toReferenceType() const
    { return const_cast<ReferenceType*>(this); }

private:
    Scope *m_parent;
    Type *m_baseType;
};

struct GenericType: public Type // ### implement me
{
    virtual GenericType *toGenericType() const
    { return const_cast<GenericType*>(this); }
};

struct AliasType: public Type // ### implement me
{
    AliasType ()
        :m_parent(0) {}
    virtual QByteArray name() const
    {  return m_name;  }
    virtual Item *parent() const
    {  return m_parent;  }

    virtual AliasType *toAliasType() const
    { return const_cast<AliasType*>(this); }
private:
    QByteArray m_name;
    Scope *m_parent;
};

struct FunctionMember: public Member
{
    FunctionMember();

    Type *returnType() const
    { return m_returnType; }

    void setReturnType(Type *type)
    { m_returnType = type; }

    ArgumentCollection *arguments() const;
    void addArgument(Argument *argument);

    void setFunctionBodyScope(BlockScope *functionBodyScope)
    { m_functionBodyScope = functionBodyScope; }

    BlockScope *functionBodyScope() const
    {return m_functionBodyScope;}

    bool isSignal() const
    { return m_signal; }

    void setSignal(bool b)
    { m_signal = b; }

    bool isSlot() const
    { return m_slot; }

    void setSlot(bool b)
    { m_slot = b; }

    bool isVirtual() const
    { return m_virtual; }

    void setVirtual(bool b)
    { m_virtual = b; }

    bool isAbstract() const
    { return m_abstract; }

    void setAbstract(bool b)
    { m_abstract = b; }

    virtual FunctionMember *toFunctionMember() const
    { return const_cast<FunctionMember*>(this); }

private:
    Type *m_returnType;
    ArgumentCollectionBuilder *m_argument;
    BlockScope *m_functionBodyScope;
    uint m_signal: 1;
    uint m_slot: 1;
    uint m_virtual: 1;
    uint m_abstract: 1;
};

struct VariableMember: public Member
{
    VariableMember() {}

    Type *type() const
    { return m_type; }

    void setType(Type *type)
    { m_type = type; }

    virtual VariableMember *toVariableMember() const
    { return const_cast<VariableMember*>(this); }

private:
    Type *m_type;
};

struct UsingDirectiveMember: public Member
{
    UsingDirectiveMember() {}

    virtual UsingDirectiveMember *toUsingDirectiveMember() const
    { return const_cast<UsingDirectiveMember*>(this); }

    virtual Scope *targetScope() const
    { return m_targetScope; }

    void setTargetScope(Scope *targetScope)
    { m_targetScope = targetScope; }

private:
    Scope *m_targetScope;
};

struct UsingDeclarationMember: public Member
{
    UsingDeclarationMember() {}

    virtual UsingDeclarationMember *toUsingDeclarationMember() const
    { return const_cast<UsingDeclarationMember*>(this); }

    Member *member() const
    { return m_member; }

    void setMember(Member *member)
    { m_member = member; }

private:
    Member *m_member;
};

struct TypeMember: public Member
{
    TypeMember() {}

     virtual TypeMember *toTypeMember() const
    { return const_cast<TypeMember*>(this); }

    Type *type() const
    { return m_type; }

    void setType(Type *type)
    { m_type = type; }
private:
    Type *m_type;

};

struct Argument: public Item
{
    Argument()
        :  m_type(0), m_nameAST(0) {}

    Type *type() const
    { return m_type; }

    void setType(Type *type)
    { m_type = type; }

    QByteArray name() const
    { return m_name; }

    void setName(const QByteArray &name)
    { m_name = name; }

    AST * nameAST() const
    { return m_nameAST; }

    void setNameAST(AST *name)
    { m_nameAST = name; }

    virtual FunctionMember *parent() const
    { return m_parent; }

    void setParent(FunctionMember *parent)
    { m_parent = parent; }

private:
    FunctionMember *m_parent;
    Type *m_type;
    QByteArray m_name;
    AST *m_nameAST;
};


struct NameUse: public Item
{
    NameUse()
    : m_declaration(0) {}

    QByteArray name() const
    { return m_name; }

    void setName(const QByteArray &name)
    {  m_name = name; }

    AST * nameAST() const
    { return m_nameAST; }

    void setNameAST(AST *name)
    { m_nameAST = name; }

     virtual Scope *parent() const
    { return m_parent; }

    void setParent(Scope *parent)
    { m_parent = parent; }

    virtual Member *declaration() const
    { return m_declaration; }

    void setDeclaration(Member *parent)
    { m_declaration = parent; }

private:
    QByteArray m_name;
    AST *m_nameAST;
    Member *m_declaration;
    Scope *m_parent;
};

struct ScopeCollection: public ItemCollection
{
    ScopeCollection(Scope *parent)
        : m_parent(parent) {}

    virtual Scope *parent() const
    { return m_parent; }

    virtual Scope *item(int index) const
    { return m_scope.at(index); }

    virtual int count() const
    { return m_scope.count(); }

    virtual int indexOf(Item *item) const
    { return m_scope.indexOf(static_cast<Scope*>(item)); }

protected:
    Scope *m_parent;
    QList<Scope*> m_scope;
};

struct ScopeCollectionBuilder: public ScopeCollection
{
    ScopeCollectionBuilder(Scope *parent)
        : ScopeCollection(parent) {}

    void add(Scope *scope)
    { m_scope.append(scope); }
};

struct MemberCollection: public ItemCollection
{
    MemberCollection(Scope *parent)
        : m_parent(parent) {}

    virtual Scope *parent() const
    { return m_parent; }

    virtual Member *item(int index) const
    { return m_member.at(index); }

    virtual int count() const
    { return m_member.count(); }

    virtual int indexOf(Item *item) const
    { return m_member.indexOf(static_cast<Member*>(item)); }

protected:
    Scope *m_parent;
    QList<Member*> m_member;
};

struct MemberCollectionBuilder: public MemberCollection
{
    MemberCollectionBuilder(Scope *parent)
        : MemberCollection(parent) {}

    void add(Member *member)
    { m_member.append(member); }
};

struct ArgumentCollection: public ItemCollection
{
    ArgumentCollection(FunctionMember *parent)
        : m_parent(parent) {}

    virtual FunctionMember *parent() const
    { return m_parent; }

    virtual Argument *item(int index) const
    { return m_argument.at(index); }

    virtual int count() const
    { return m_argument.count(); }

    virtual int indexOf(Item *item) const
    { return m_argument.indexOf(static_cast<Argument*>(item)); }

protected:
    FunctionMember *m_parent;
    QList<Argument*> m_argument;
};

struct ArgumentCollectionBuilder: public ArgumentCollection
{
    ArgumentCollectionBuilder(FunctionMember *parent)
        : ArgumentCollection(parent) {}

    void add(Argument *argument)
    { m_argument.append(argument); }
};

struct TypeCollection: public ItemCollection
{
    TypeCollection(Scope *parent)
        : m_parent(parent) {}

    virtual Scope *parent() const
    { return m_parent; }

    virtual Type *item(int index) const
    { return m_type.at(index); }

    virtual int count() const
    { return m_type.count(); }

    virtual int indexOf(Item *item) const
    { return m_type.indexOf(static_cast<Type*>(item)); }

protected:
    Scope *m_parent;
    QList<Type*> m_type;
};

struct TypeCollectionBuilder: public TypeCollection
{
    TypeCollectionBuilder(Scope *parent)
        : TypeCollection(parent) {}

    void add(Type *type)
    { m_type.append(type); }
};


struct NameUseCollection: public ItemCollection
{
    NameUseCollection(Scope *parent)
        : m_parent(parent) {}

    virtual Scope *parent() const
    { return m_parent; }

    virtual NameUse *item(int index) const
    { return m_nameUse.at(index); }

    virtual int count() const
    { return m_nameUse.count(); }

    virtual int indexOf(Item *item) const
    { return m_nameUse.indexOf(static_cast<NameUse*>(item)); }

protected:
    Scope *m_parent;
    QList<NameUse*> m_nameUse;
};

struct NameUseCollectionBuilder: public NameUseCollection
{
    NameUseCollectionBuilder(Scope *parent)
        : NameUseCollection(parent) {}

    void add(NameUse *nameUse)
    { m_nameUse.append(nameUse); }
};

template <class T>
T *Create(TypedPool<CodeModel::Item> *p)
{
    return new (p->allocate(sizeof(T))) T();
}

} // namespace CodeModel


#endif // CODEMODEL_H
