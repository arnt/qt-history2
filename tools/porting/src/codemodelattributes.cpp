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

#include "codemodelattributes.h"
#include "tokenengine.h"

using namespace CodeModel;
using namespace TokenEngine;

/*
     Walk the codemodel.
*/
void CodeModelAttributes::createAttributes(TranslationUnit translationUnit)
{
    m_translationUnit = translationUnit;
    parseScope(const_cast<CodeModel::NamespaceScope *>(translationUnit.codeModel()));
}

/*
    Create attributes for each name use and assign to the token.
*/
void CodeModelAttributes::parseNameUse(CodeModel::NameUse *nameUse)
{
    //get the container for this token
    const int nameTokenIndex = nameUse->nameAST()->startToken();
    TokenSectionSequence tokens = m_translationUnit.tokens();
    TokenContainer container = tokens.tokenContainer(nameTokenIndex);
    const int containerIndex = tokens.containerIndex(nameTokenIndex);
    TokenAttributes *attributes = container.tokenAttributes();

    //add attributes this namnUse
    attributes->addAttribute(containerIndex, "nameUse", nameUse->name());
    attributes->addAttribute(containerIndex, "parentScope",
                             nameUse->declaration()->parent()->name() );
    if(CodeModel::Scope * skop = nameUse->declaration()->parent()->parent()) {
        attributes->addAttribute(containerIndex, "grandParentScope", skop->name() );
    }
}
/*
    Create attributes for members and assign to token.
*/
void CodeModelAttributes::parseMember(CodeModel::Member *member)
{
    if(!member || !member->nameAST() || member->name() == QByteArray())
        return;

    //get the container for this token
    const int nameTokenIndex = member->nameAST()->startToken();
    TokenSectionSequence tokens = m_translationUnit.tokens();
    TokenContainer container = tokens.tokenContainer(nameTokenIndex);
    const int containerIndex = tokens.containerIndex(nameTokenIndex);
    TokenAttributes *attributes = container.tokenAttributes();

     //add attributes for this declaration
    attributes->addAttribute(containerIndex, "declaration", member->name());

    CodeModelWalker::parseMember(member);
}


