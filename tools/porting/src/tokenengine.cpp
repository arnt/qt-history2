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
#include "tokenengine.h"

namespace TokenEngine {

/*
     Construnct an empty TokenContainer.
*/
TokenContainer::TokenContainer()
{
    d = new TokenContainerData();
    d->typeInfo = 0;
}

/*
    Construnct a TokenContainer from data
*/
TokenContainer::TokenContainer(QByteArray text, QList<Token> tokens, TypeInfo *typeInfo)
{
    d = new TokenContainerData();
    d->text = text;
    d->tokens = tokens;
    if(d->typeInfo == 0)
		d->typeInfo = new TypeInfo();
	else 
		d->typeInfo = typeInfo;
}

int TokenContainer::count() const
{
    return d->tokens.count();
}

QByteArray TokenContainer::text(const int index) const
{
    Token token = d->tokens.at(index);
    return d->text.mid(token.start, token.length);
}

QByteArray TokenContainer::fullText() const
{
    return d->text;
}

TokenContainer TokenContainer::tokenContainer(const int index) const
{
    Q_UNUSED(index);
    return *this;
}

Token TokenContainer::token(const int index) const
{
    return d->tokens.at(index);
}

TypeInfo *TokenContainer::typeInfo()
{
    return d->typeInfo;
}

TokenAttributes *TokenContainer::tokenAttributes()
{
    return d->tokenAttributes;
}

QByteArray TokenSection::fullText() const
{
    QByteArray text;
    for (int t = m_start; t < m_start + m_count; ++t) {
        text += m_tokenContainer.text(t);
    }
    return text;
}

QByteArray TokenList::fullText() const
{
    QByteArray text;
    for (int t = 0; t < m_tokenList.count(); ++t) {
        text += m_tokenContainer.text(m_tokenList.at(t));
    }
    return text;
}

TokenSectionSequence::TokenSectionSequence(QList<TokenSection> tokenSections)
:m_tokenSections(tokenSections), m_count(0)
{
    for(int s = 0; s < m_tokenSections.count(); ++s) {
        m_startIndexes.append(m_count);
        m_count += m_tokenSections.at(s).count();
    }
}

int TokenSectionSequence::count() const
{
    return m_count;
}

QByteArray TokenSectionSequence::text(const int index) const
{
    const int sectionIndex = findSection(index);
    const int sectionInternalIndex = calculateInternalIndex(index, sectionIndex);
    return m_tokenSections.at(sectionIndex).text(sectionInternalIndex);
}

QByteArray TokenSectionSequence::fullText() const
{
    QByteArray text;
    foreach(TokenSection section, m_tokenSections) {
        text += section.fullText();
    }
    return text;
}

TokenContainer TokenSectionSequence::tokenContainer(const int index) const
{
    const int sectionIndex = findSection(index);
    const int sectionInternalIndex = calculateInternalIndex(index, sectionIndex);
    return m_tokenSections.at(sectionIndex).tokenContainer(sectionInternalIndex);
}

int TokenSectionSequence::containerIndex(const int index) const
{
    const int sectionIndex = findSection(index);
    const int sectionInternalIndex = calculateInternalIndex(index, sectionIndex);
    return m_tokenSections.at(sectionIndex).containerIndex(sectionInternalIndex);
}

int TokenSectionSequence::findSection(const int index) const
{
    int c = 0;
    bool found = false;
    //Here we do a linear search through all collections in the list,
    //which could turn out to be to slow.
    while(!found && c < m_tokenSections.count()) {
        const int sectionEnd = m_startIndexes.at(c)
                                + m_tokenSections.at(c).count();
        if (index < sectionEnd)
            found = true;
        else
            ++c;
    }
    if(!found) {
        //error
        Q_ASSERT(0);
        return -1;
    }
    Q_ASSERT(c < m_tokenSections.count());
    return c;
}

int TokenSectionSequence::calculateInternalIndex(const int index, const int sectionIndex) const
{
    const int sectionInternalIndex =
        index - m_startIndexes.at(sectionIndex);
    Q_ASSERT(sectionInternalIndex < m_tokenSections.at(sectionIndex).count());
    return sectionInternalIndex;
}

} //namespace TokenEngine

