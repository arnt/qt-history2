#include "genqtsymbols.h"

#include <QByteArray>

QtParser::QtParser(QByteArray translationUnit)
:qtFound(false)
{
    fileSymbol = new FileSymbol();
    fileSymbol->contents=translationUnit;
    fileSymbol->tokenStream = lexer.tokenize(fileSymbol);
    fileSymbol->ast = parser.parse(fileSymbol, &p);
}

QStringList QtParser::getQtSymbols()
{
    return symbols;
}

QStringList QtParser::getEnumerators()
{
    return enumerators;
}

QStringList QtParser::getEnumSpecifiers()
{
    return enumSpecifiers;
}

QStringList QtParser::getDeclarators()
{
    return declarators;
}


void QtParser::parseDeclarator(DeclaratorAST *node)
{
    if(!qtFound)
        return;
    AST *n = node->declaratorId();
    if(n) {
        int num = node->declaratorId()->startToken();
        QByteArray name = fileSymbol->tokenStream->tokenText(num);
        //printf("Walking a declarator, named: %s\n", name.constData());
        symbols.append(name);
        declarators.append(name);
        TreeWalker::parseDeclarator(node);
    }
}

void QtParser::parseEnumerator(EnumeratorAST *node)
{
    if(!qtFound)
        return;
    QByteArray name = fileSymbol->tokenStream->tokenText(node->id()->startToken());
//    printf("Walking a enumerator, named: %s\n", name.constData());
    symbols.append(name);
    enumerators.append(name);
    TreeWalker::parseEnumerator(node);
}

void QtParser::parseEnumSpecifier(EnumSpecifierAST *node)
{
     if(!qtFound)
        return;
    NameAST *nameAst = node->name();
    if (nameAst) {
        int nameToken = node->name()->startToken();
        QByteArray name = fileSymbol->tokenStream->tokenText(nameToken);
       // printf("Walking a enumSpecifier, named: %s\n", name.constData());
        symbols.append(name);
        enumSpecifiers.append(name);
    }

    TreeWalker::parseEnumSpecifier(node);
}

//////////////////////////////////////////////////////////////////

QtClassParser::QtClassParser(QByteArray translationUnit)
:QtParser(translationUnit)
{
    parseNode(fileSymbol->ast);
}

void QtClassParser::parseClassSpecifier(ClassSpecifierAST *node)
{
    QByteArray className = fileSymbol->tokenStream->tokenText(node->classKey()->endToken());
    if(className == "Qt")
    {
        //printf("Walking a class, named: %s\n", className.constData());
        //puts("found Qt class");
        qtFound=true;
        TreeWalker::parseClassSpecifier(node);
        qtFound=false;
    }
}
////////////////////////////////////////////////////


QtNamespaceParser::QtNamespaceParser(QByteArray translationUnit)
:QtParser(translationUnit)
{
    parseNode(fileSymbol->ast);
}

void QtNamespaceParser::parseNamespace(NamespaceAST *node)
{
    QByteArray name = fileSymbol->tokenStream->tokenText(node->namespaceName()->startToken());
    if(name == "Qt")
    {
     //   puts("found Qt namespace");
        qtFound=true;
        TreeWalker::parseNamespace(node);
        qtFound=false;
    }
}

