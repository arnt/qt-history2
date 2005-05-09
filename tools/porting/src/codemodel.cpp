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
#include "codemodel.h"

#include <QList>
#include <QByteArray>
#include <qdebug.h>

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

void Scope::addScope(Scope *scope)
{
    scope->setParent(this);
    m_scopes.add(scope);
}

void Scope::addType(Type *type)
{
    if (ClassType *klass = type->toClassType())
        klass->setParent(this);
    m_types.add(type);
}

void Scope::addMember(Member *member)
{
    member->setParent(this);
    m_members.add(member);
}

void Scope::addNameUse(NameUse *nameUse)
{
    nameUse->setParent(this);
    m_nameUses.add(nameUse);
}

} //namepsace CodeModel

