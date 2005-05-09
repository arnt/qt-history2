/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef CODEMODELWALKER_H
#define CODEMODELWALKER_H
#include "codemodel.h"

class CodeModelWalker
{
public:
    virtual ~CodeModelWalker(){};
    virtual void parseScope(CodeModel::Scope *scope);
    virtual void parseClassScope(CodeModel::ClassScope *){};
    virtual void parseNamespaceScope(CodeModel::NamespaceScope *){};
    virtual void parseBlockScope(CodeModel::BlockScope *){};

    virtual void parseType(CodeModel::Type *type);
    virtual void parseEnumType(CodeModel::EnumType *){};
    virtual void parseClassType(CodeModel::ClassType *){};
    virtual void parseUnknownType(CodeModel::UnknownType *){};
    virtual void parseBuiltinType(CodeModel::BuiltinType *){};
    virtual void parsePointerType(CodeModel::PointerType *){};
    virtual void parseReferenceType(CodeModel::ReferenceType *){};
    virtual void parseGenericType(CodeModel::GenericType *){};
    virtual void parseAliasType(CodeModel::AliasType *){};

    virtual void parseMember(CodeModel::Member *member);
    virtual void parseFunctionMember(CodeModel::FunctionMember *);
    virtual void parseVariableMember(CodeModel::VariableMember *){};
    virtual void parseUsingDeclarationMember(CodeModel::UsingDeclarationMember *){};
    virtual void parseTypeMember(CodeModel::TypeMember *){};

    virtual void parseArgument(CodeModel::Argument *){};
    virtual void parseNameUse(CodeModel::NameUse *){};
};

#endif
