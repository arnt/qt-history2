#ifndef GENQTSYMBOLS_H
#define GENQTSYMBOLS_H

#include <QStringList>
#include <QString>
#include <QByteArray>

#include <treewalker.h>
#include <lexer.h>
#include <parser.h>


/*
    Base class for QtClass parser and QtNamespaceParser
*/
class QtParser : public TreeWalker
{
public:
    QtParser(QByteArray translationUnit);
    QStringList getQtSymbols();
    QStringList getEnumerators();
    QStringList getEnumSpecifiers();
    QStringList getDeclarators();
protected:
    void parseDeclarator(DeclaratorAST *node);
    void parseEnumerator(EnumeratorAST *node);
    void parseEnumSpecifier(EnumSpecifierAST *node);

    Lexer lexer;
    Parser parser;
    pool p;

    FileSymbol *fileSymbol;
    QStringList symbols;
    QStringList enumerators;
    QStringList enumSpecifiers;
    QStringList declarators;
    bool qtFound;
};



/*
    Parses file and returns all sybols defined
    in the Qt class (which should be defined somewhere
    inside file passed as the fileSymbol argument)
*/
class QtClassParser : public QtParser
{
public:
    QtClassParser(QByteArray translationUnit);
protected:
    void parseClassSpecifier(ClassSpecifierAST *node);
};


/*
    Parses file and returns all sybols defined
    in the Qt namespace (which should be defined somewhere
    inside file passed as the fileSymbol argument)
*/
class QtNamespaceParser : public QtParser
{
public:
    QtNamespaceParser(QByteArray translationUnit);
protected:
    void parseNamespace(NamespaceAST *node);

};




#endif
