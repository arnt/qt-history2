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
#include <QList>
#include <QByteArray>

#include "codemodel2.h"

//using namespace CodeModel;
namespace CodeModel {

static pool global_pool;

BuiltinType *BuiltinType::Bool = new (global_pool.allocate(sizeof(BuiltinType))) BuiltinType("bool", 0, &global_pool);
BuiltinType *BuiltinType::Void = new (global_pool.allocate(sizeof(BuiltinType))) BuiltinType("void", 0, &global_pool);
BuiltinType *BuiltinType::Char = new (global_pool.allocate(sizeof(BuiltinType))) BuiltinType("char", 0, &global_pool);
BuiltinType *BuiltinType::Short = new (global_pool.allocate(sizeof(BuiltinType))) BuiltinType("short", 0, &global_pool);
BuiltinType *BuiltinType::Int = new (global_pool.allocate(sizeof(BuiltinType))) BuiltinType("int", 0, &global_pool);
BuiltinType *BuiltinType::Long = new (global_pool.allocate(sizeof(BuiltinType))) BuiltinType("long", 0, &global_pool);
BuiltinType *BuiltinType::Double = new (global_pool.allocate(sizeof(BuiltinType))) BuiltinType("double", 0, &global_pool);
BuiltinType *BuiltinType::Float = new (global_pool.allocate(sizeof(BuiltinType))) BuiltinType("float", 0, &global_pool);
BuiltinType *BuiltinType::Unsigned = new (global_pool.allocate(sizeof(BuiltinType))) BuiltinType("unsigned", 0, &global_pool);
BuiltinType *BuiltinType::Signed = new (global_pool.allocate(sizeof(BuiltinType))) BuiltinType("signed", 0, &global_pool);

int e;

CodeModel::Scope::Scope(pool *p)
    : m_parent(0)
{
    m_scopes = new (global_pool.allocate(sizeof(ScopeCollectionBuilder))) ScopeCollectionBuilder(this, p);
    m_types = new (global_pool.allocate(sizeof(TypeCollectionBuilder))) TypeCollectionBuilder(this, p);
    m_members = new (global_pool.allocate(sizeof(MemberCollectionBuilder))) MemberCollectionBuilder(this, p);
    m_nameUses = new (global_pool.allocate(sizeof(NameUseCollectionBuilder))) NameUseCollectionBuilder(this, p);
}

ScopeCollection *Scope::scopes() const
{ return m_scopes; }

TypeCollection *Scope::types() const
{ return m_types; }

MemberCollection *Scope::members() const
{ return m_members; }

NameUseCollection *Scope::nameUses() const
{ return m_nameUses; }


void Scope::addScope(Scope *scope)
{
    scope->setParent(this);
    m_scopes->add(scope);
}

void Scope::addType(Type *type)
{
    if (ClassType *klass = type->toClassType())
        klass->setParent(this);
    m_types->add(type);
}

void Scope::addMember(Member *member)
{
    member->setParent(this);
    m_members->add(member);
}

void Scope::addNameUse(NameUse *nameUse)
{
    nameUse->setParent(this);
    m_nameUses->add(nameUse);
}


QList<Scope*> Scope::findScope(const QByteArray &name)
{
    QList<Scope*> l;
    for (int i=0; i<scopes()->count(); ++i) {
        Scope *scope = scopes()->item(i);
        Q_ASSERT(scope);

        if (scope->name() == name)
            l.append(scope);
    }

    if (parent())
        l += parent()->findScope(name);

    return l;
}

QList<Type*> Scope::findType(const QByteArray &name)
{
    QList<Type*> l;
    for (int i=0; i<types()->count(); ++i) {
        Type *type = types()->item(i);
        Q_ASSERT(type);

        if (type->name() == name)
            l.append(type);
    }

    if (ClassScope *klass = toClassScope()) {
        for (int i=0; i<klass->baseClasses()->count(); ++i) {
            Type *type = klass->baseClasses()->item(i);
            if (!(type && type->toClassType()))
                continue;

            ClassType *k = type->toClassType();
            Q_ASSERT(k->scope());

            l += k->scope()->findType(name);
        }
    }

    if (parent())
        l += parent()->findType(name);

    return l;
}

QList<Member*> Scope::findMember(const QByteArray &name)
{
    QList<Member*> l;
    for (int i=0; i<members()->count(); ++i) {
        Member *member = members()->item(i);
        Q_ASSERT(member);

        if (member->name() == name)
            l.append(member);
    }

    if (ClassScope *klass = toClassScope()) {
        for (int i=0; i<klass->baseClasses()->count(); ++i) {
            Type *type = klass->baseClasses()->item(i);
            Q_ASSERT(type && type->toClassType());
            ClassType *klass = type->toClassType();
            Q_ASSERT(klass->scope());

            l += klass->scope()->findMember(name);
        }
    }

    if (parent())
        l += parent()->findMember(name);

    return l;
}

QList<FunctionMember*> Scope::findFunctionMember(const QByteArray &name)
{
    QList<Member*> l = findMember(name);
    QList<FunctionMember*> ll;
    foreach (Member *m, l)
        if (FunctionMember *f = m->toFunctionMember())
            ll.append(f);
    return ll;
}

QList<VariableMember*> Scope::findVariableMember(const QByteArray &name)
{
    QList<Member*> l = findMember(name);
    QList<VariableMember*> ll;
    foreach (Member *m, l)
        if (VariableMember *f = m->toVariableMember())
            ll.append(f);
    return ll;
}

Scope *Scope::lookUpScope(const QList<QByteArray> &nestedNameSpecifier)
{
    if(nestedNameSpecifier.isEmpty()){
        puts("Error in Scope::lookUpScope: empty nestedNameSpecifier");
    }
    Scope *currentScope = this;
    int nestingCounter = 0;
    if(nestedNameSpecifier[nestingCounter]=="::") {
        ++nestingCounter;
        while (currentScope->parent())
            currentScope = currentScope->parent();
    }
    while (currentScope != 0 && nestingCounter < nestedNameSpecifier.count()) {
        ScopeCollection *nestedScopes = currentScope->scopes();
        currentScope=0;
        for (int i=0; i<nestedScopes->count(); ++i) {
            Scope *scope = nestedScopes->item(i);
            Q_ASSERT(scope);

            if (scope->name() == nestedNameSpecifier[nestingCounter]) {
                currentScope = scope;
                break;
            }
        }

        ++nestingCounter;
    }
    return currentScope;
}


// NamespaceScope

NamespaceScope *NamespaceScope::findNamespace(const QByteArray name)
{
   QList<NamespaceScope *> l;
    for (int i=0; i<scopes()->count(); ++i) {
        Scope *scope = scopes()->item(i);
        Q_ASSERT(scope);
        if (scope->name() == name && scope->toNamespaceScope())
            l.append(scope->toNamespaceScope());
    }
    if(l.isEmpty())
        return 0;
    else
        return l[0];

}



// == ClassScope =======================
ClassScope::ClassScope(pool *p)
    : Scope(p)
{
    m_baseClasses = new (global_pool.allocate(sizeof(TypeCollectionBuilder))) TypeCollectionBuilder(this, p);
}

TypeCollection *ClassScope::baseClasses() const
{
    return m_baseClasses;
}

void ClassScope::addBaseClass(Type *baseClass)
{
    m_baseClasses->add(baseClass);
}


// == FunctionMember ===================
FunctionMember::FunctionMember(pool *p)
    : m_returnType(0),
      m_signal(0), m_slot(0),
      m_virtual(0), m_abstract(0)
{
    m_argument = new (global_pool.allocate(sizeof(ArgumentCollectionBuilder))) ArgumentCollectionBuilder(this, p);
}

ArgumentCollection *FunctionMember::arguments() const
{
    return m_argument;
}

void FunctionMember::addArgument(Argument *argument)
{
    m_argument->add(argument);
}

} //namepsace CodeModel

