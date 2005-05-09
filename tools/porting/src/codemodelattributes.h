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

#ifndef CODEMODELATTRIBUTES_H
#define CODEMODELATTRIBUTES_H

#include "codemodelwalker.h"
#include "translationunit.h"

class CodeModelAttributes : public CodeModelWalker
{
public:
    void createAttributes(TranslationUnit translationUnit);
protected:
    void parseNameUse(CodeModel::NameUse *);
    void parseMember(CodeModel::Member *member);

    void createNameTypeAttribute(CodeModel::Member *member);
    void createNameTypeAttribute(CodeModel::NameUse *nameUse);

private:
    void createNameTypeAttributeAtIndex(TokenEngine::TokenAttributes *attributes,
                                        int index, CodeModel::Member *member);
    bool areAttributesEnabled(const TokenEngine::TokenAttributes *attributes) const;

    TranslationUnit  m_translationUnit;
};

#endif


