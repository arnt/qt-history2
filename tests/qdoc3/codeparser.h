/*
  codeparser.h
*/

#ifndef CODEPARSER_H
#define CODEPARSER_H

class QString;
class Tree;

class CodeParser
{
public:
    virtual ~CodeParser() { }

    virtual void parseHeaderFile( const QString& filePath, Tree *tree ) = 0;
    virtual void parseSourceFile( const QString& filePath, Tree *tree ) = 0;
    virtual void convertTree( const Tree *tree ) = 0;
};

#endif
