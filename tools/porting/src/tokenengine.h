/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef TOKENENGINE_H
#define TOKENENGINE_H

#include <iostream>
#include <QByteArray>
#include <QList>
#include <QString>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QMap>

namespace TokenEngine {

class TokenContainer;
/*
    A token is defined as a start-postion and a lenght. Since the actual text
    storage is not reffered to here, Token needs to be used together with
    a TokenContainer in order to be useful.
*/
struct Token
{
    int start;
    int length;
};

/*
    Each TokenContainer has a TypeInfo object with meta-information.
*/
class FileInfo;
class GeneratedInfo;
class TypeInfo
{
public:
    virtual ~TypeInfo() {};
    virtual FileInfo *toFileInfo() const {return 0;}
    virtual GeneratedInfo *toGeneratedInfo() const {return 0;}
};

/*
    MetaInfo for containers that contains tokens from a file
*/
class FileInfo: public TypeInfo
{
public:
    FileInfo *toFileInfo() const
    {return const_cast<FileInfo *>(this);}

    QString filename;
};

/*
    MetaInfo for containers that contains generated tokens.
*/
class GeneratedInfo: public TypeInfo
{
public:
    GeneratedInfo *toGeneratedInfo() const
    {return const_cast<GeneratedInfo *>(this);}

    //preprocessor tree pointer?
};

class TokenAttributes
{
public:
    void addAttribute(const int index, const QByteArray &name,  const QByteArray &value);
    QByteArray attribute(const int index, const QByteArray &name) const;

private:
    inline QByteArray makeKeyText(const int index, const QByteArray &name) const;
    QMap<QByteArray, QByteArray> attributes;
};


/*
    Main interface for the tokenEngine classes. The classes does not actually
    inheret this interface, they implement the same functions. Use the
    TokenSequenceAdapter to get a class that inherits TokenSequence.
*/
class TokenSequence
{
public:
    virtual ~TokenSequence() {}
    virtual int count() const = 0;
    virtual QByteArray text(const int index) const = 0;
    virtual TokenContainer tokenContainer(const int index) const = 0;
    virtual int containerIndex(const int index) const = 0;

//a bi-directional iterator, wich will be faster that the above random-access interface
//  TokenSequenceIterator iterator(int index)
};

/*
    A TokenSequence that stores text and tokens referencing
    that text;
*/
class TokenContainerData : public QSharedData
{
public:
    TokenContainerData()
    {tokenAttributes = new TokenAttributes();}
    ~TokenContainerData()
    {delete tokenAttributes; delete typeInfo; }
    QByteArray text;
    QList<Token> tokens;
    TypeInfo *typeInfo;
    TokenAttributes *tokenAttributes;
};

class TokenContainer
{
public:
    TokenContainer();
    TokenContainer(QByteArray text, QList<Token> tokens, TypeInfo *typeInfo = 0);
    int count() const;
    QByteArray text(const int index) const;
    QByteArray fullText() const;
    TokenContainer tokenContainer(const int index) const;
    inline int containerIndex(const int index) const
    { return index; }
    Token token(const int index) const;
    TypeInfo *typeInfo();
    TokenAttributes *tokenAttributes();
    const TokenAttributes *tokenAttributes() const;
    int line(int index) const;
    int column(int index) const; 
private:
    QExplicitlySharedDataPointer<TokenContainerData> d;
};

/*
    A reference to a single token in a container
*/
class TokenRef
{
public:
    TokenRef(): m_index(-1) {}
    TokenRef(TokenContainer tokenContainer, int containerIndex)
    : m_tokenContainer(tokenContainer), m_index(containerIndex) {}
    inline int count() const
    { return m_index == -1 ? 0 : 1; }
    inline QByteArray text(const int index = 0) const
    { Q_UNUSED(index); return m_tokenContainer.text(m_index);  }
    inline QByteArray fullText() const
    { return text(); }
    inline TokenContainer tokenContainer(const int index = 0) const
    { Q_UNUSED(index); return m_tokenContainer; }
    inline int containerIndex(const int index = 0) const
    { Q_UNUSED(index); return m_index; }
private:
    TokenContainer m_tokenContainer;
    int m_index;
};

/*
    Contains a selected range from a TokenSequence.
*/
class TokenSection
{
public:
    TokenSection() : m_start(0), m_count(0) {}
    TokenSection(TokenContainer tokenContainer,
        const int start, const int count)
    :m_tokenContainer(tokenContainer), m_start(start), m_count(count) {}

    inline int count() const
    { return m_count; }
    inline QByteArray text(const int index) const
    {
        const int cIndex = containerIndex(index);
        Q_ASSERT(cIndex < m_tokenContainer.count());
        return m_tokenContainer.text(cIndex);
    }
    QByteArray fullText() const;
    inline TokenContainer tokenContainer(const int index) const
    { Q_UNUSED(index); return m_tokenContainer; }
    inline int containerIndex(const int index) const
    { return m_start + index; }

private:
    TokenContainer m_tokenContainer;
    int m_start;
    int m_count;
};

/*
    A list of tokens from a tokenContainer
*/
class TokenList
{
public:
    TokenList() {};
    TokenList(TokenContainer tokenContainer, QList<int> tokenList)
    :m_tokenContainer(tokenContainer), m_tokenList(tokenList) {}
    inline int count() const
    { return m_tokenList.count(); }
    inline QByteArray text(const int index) const
    {
        const int cIndex = containerIndex(index);
        Q_ASSERT(cIndex < m_tokenContainer.count());
        return m_tokenContainer.text(cIndex);
    }
    QByteArray fullText() const;
    inline TokenContainer tokenContainer(const int index) const
    { Q_UNUSED(index); return m_tokenContainer; }
    inline int containerIndex(const int index) const
    { return m_tokenList.at(index); }

private:
    TokenContainer m_tokenContainer;
    QList<int> m_tokenList;
};

/*
    Combines a list of TokenSequences into one TokenSequence
*/
class TokenSectionSequence
{
public:
    TokenSectionSequence() :m_count(0) {};
    TokenSectionSequence(QList<TokenSection> tokenSections);
    int count() const;
    QByteArray text(const int index) const;
    QByteArray fullText() const;
    TokenContainer tokenContainer(const int index) const;
    int containerIndex(const int index) const;
protected:
    int findSection(const int index) const;
    int calculateInternalIndex(const int index, const int sectionIndex) const;
private:
    QList<TokenSection> m_tokenSections;
    QList<int> m_startIndexes;
    int m_count;
};

/*
      Adapts classes that provides the TokenSequence interface
      to a TokenSequence derived class.
*/
template <typename TokenSequenceType>
class TokenSequenceAdapter : public TokenSequence
{
public:
    TokenSequenceAdapter(const TokenSequenceType &tokenSequenceObject)
    :m_tokenSequenceObject(tokenSequenceObject) {}
    int count() const
    { return m_tokenSequenceObject.count(); }
    QByteArray text(const int index) const
    { return m_tokenSequenceObject.text(index); }
    TokenContainer tokenContainer(const int index) const
    { return m_tokenSequenceObject.tokenContainer(index); }
    int containerIndex(const int index) const
    { return m_tokenSequenceObject.containerIndex(index); }
private:
    TokenSequenceType m_tokenSequenceObject;
};

template <typename TokenSequence>
QByteArray getText(TokenSequence tokenSequence)
{
    QByteArray text;
    for (int t = 0; t<tokenSequence.count(); ++t) {
        text += tokenSequence.text(t);
    }
    return text;
}


} //namespace TokenEngine

#endif

