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

#include "codemodel.h"

//using namespace CodeModel;
namespace CodeModel {

BuiltinType BuiltinType::Bool("bool", 0 );
BuiltinType BuiltinType::Void("void", 0 );
BuiltinType BuiltinType::Char("char", 0 );
BuiltinType BuiltinType::Short("short", 0 );
BuiltinType BuiltinType::Int("int", 0 );
BuiltinType BuiltinType::Long("long", 0 );
BuiltinType BuiltinType::Double("double", 0 );
BuiltinType BuiltinType::Float("float", 0 );
BuiltinType BuiltinType::Unsigned("unsigned", 0 );
BuiltinType BuiltinType::Signed("signed", 0 );

int e;

CodeModel::Scope::Scope()
    : m_parent(0)
{
    m_scopes = new ScopeCollectionBuilder(this);
    m_types = new TypeCollectionBuilder(this);
    m_members = new MemberCollectionBuilder(this);
    m_nameUses = new NameUseCollectionBuilder(this);
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
ClassScope::ClassScope()
{
    m_baseClasses = new TypeCollectionBuilder(this);
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
FunctionMember::FunctionMember()
    : m_returnType(0),
      m_functionBodyScope(0),
      m_signal(0),
      m_virtual(0), m_abstract(0)
{
    // need to assign this outside the construction list, to work around
    // a bug in aCC.
    m_slot = 0;
    m_argument = new  ArgumentCollectionBuilder(this);
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

