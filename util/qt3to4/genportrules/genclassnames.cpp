#include "genclassnames.h"
#include <iostream>
#include <QByteArray>

using namespace std;

ClassNamesParser::ClassNamesParser (FileSymbol *fileSymbol)
:fileSymbol(fileSymbol)
,qtClassFound(false)
{

}

QStringList ClassNamesParser::getClassNames()
{
    parseNode(fileSymbol->ast);
    symbols.sort();
    return symbols;
}

bool ClassNamesParser::isDuplicate(QString newName)
{
    foreach(QString name, symbols)
        if (name==newName)
            return true;
    return false;
}


void ClassNamesParser::parseClassSpecifier(ClassSpecifierAST *node)
{
    QByteArray className = fileSymbol->tokenStream->tokenText(node->classKey()->endToken());
    /*
    printf("Walking a class\n");
    int i;
    for(i=node->classKey()->startToken(); i<=node->classKey()->endToken(); ++i) {
        printf("     token: %s\n", fileSymbol->tokenStream->tokenText(i).constData());
    }
*/
    if (className[0]=='Q' &&!isDuplicate(className))
        symbols.append(className);


   //TreeWalker::parseClassSpecifier(node);
}

//////////////////////////////////////////

QtClassAncestors::QtClassAncestors (FileSymbol *fileSymbol)
:fileSymbol(fileSymbol)
{

}

QStringList QtClassAncestors::getClassNames()
{
    classNames.push_back("Qt");

    int numClassNames = -1;
    while (classNames.size() !=  numClassNames) {
    //   cout << "parsing" << endl;
        numClassNames = classNames.size();
        parseNode(fileSymbol->ast);
    }
    return classNames;
}

bool QtClassAncestors::isDuplicate(QString newName)
{

    foreach(QString name, classNames)
        if (name==newName)
            return true;
    return false;
}


void QtClassAncestors::parseClassSpecifier(ClassSpecifierAST *node)
{
    QByteArray className = fileSymbol->tokenStream->tokenText(node->classKey()->endToken());
    if (className[0]=='Q' ) {
        BaseClauseAST * baseClause =  node->baseClause();
        if(!baseClause)
            return;
        List<BaseSpecifierAST *> *baseSpecifierList = baseClause->baseSpecifierList();
        if(!baseSpecifierList)
            return;
        bool inheritsQt = false;
        foreach(BaseSpecifierAST *baseSpecifier, *baseSpecifierList) {
            QByteArray baseClassName = fileSymbol->tokenStream->tokenText(baseSpecifier->name()->startToken());
            //cout << "found base class" <<baseClassName.constData()<<endl;
            foreach(QString name, classNames)
                if(baseClassName == name)
                    inheritsQt = true;
        }
        if(!inheritsQt)
            return;
        if(isDuplicate(className))
            return;
    //    cout<<"Found class that inherits Qt "<< className.constData()<<endl;
        classNames.append(className);
    }
}

