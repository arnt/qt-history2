#ifndef GENCLASSNAMES_H
#define GENCLASSNAMES_H

#include <QStringList>
#include <QString>

#include <treewalker.h>
#include <lexer.h>

/*
    Parses file and returns the names of all
    classes defined.
*/
class ClassNamesParser : public TreeWalker
{
public:
    ClassNamesParser(FileSymbol *fileSymbol);
    QStringList getClassNames();

protected:
    void parseClassSpecifier(ClassSpecifierAST *node);

    bool isDuplicate(QString name);
private:
    FileSymbol *fileSymbol;
    QStringList symbols;
    bool qtClassFound;
};


/*
    Parses a translation unit and returns a list of classes
    that inherits the Qt class
*/
class QtClassAncestors : public TreeWalker
{
public:
    QtClassAncestors(FileSymbol *fileSymbol);
    QStringList getClassNames();

protected:
    void parseClassSpecifier(ClassSpecifierAST *node);

    bool isDuplicate(QString name);
private:
    FileSymbol *fileSymbol;
    QStringList classNames;
    //bool qtClassFound;
};





#endif
