#ifndef CODEMODELWALKER_H
#define CODEMODELWALKER_H
#include <codemodel2.h>

class CodeModelWalker
{
public:
/*
    CodeModelWalker();
*/
    virtual ~CodeModelWalker(){};
    virtual void parseScope(CodeModel::Scope *scope);
    virtual void parseClassScope(CodeModel::ClassScope */*scope*/){};
    virtual void parseNamespaceScope(CodeModel::NamespaceScope */*scope*/){};
    virtual void parseBlockScope(CodeModel::BlockScope */*scope*/){};

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
    virtual void parseFunctionMember(CodeModel::FunctionMember *){};
    virtual void parseVariableMember(CodeModel::VariableMember *){};
    virtual void parseUsingDeclarationMember(CodeModel::UsingDeclarationMember *){};
    virtual void parseUsingDirectiveMember(CodeModel::UsingDirectiveMember *){};
    virtual void parseTypeMember(CodeModel::TypeMember *){};

    virtual void parseArgument(CodeModel::Argument *){};
    virtual void parseNameUse(CodeModel::NameUse *){};
};

#endif
