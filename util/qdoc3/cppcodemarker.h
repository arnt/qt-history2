/*
  cppcodemarker.h
*/

#ifndef CPPCODEMARKER_H
#define CPPCODEMARKER_H

#include "codemarker.h"

class CppCodeMarker : public CodeMarker
{
public:
    CppCodeMarker();
    ~CppCodeMarker();

    bool recognizeCode( const QString& code );
    bool recognizeExtension( const QString& ext );
    bool recognizeLanguage( const QString& lang );
    QString plainName(const Node *node);
    QString plainFullName(const Node *node, const Node *relative);
    QString markedUpCode(const QString& code, const Node *relative, const QString& dirPath );
    QString markedUpSynopsis( const Node *node, const Node *relative,
				      SynopsisStyle style );
    QString markedUpName( const Node *node );
    QString markedUpFullName( const Node *node, const Node *relative );
    QString markedUpEnumValue(const QString &enumValue, const Node *relative);
    QString markedUpIncludes( const QStringList& includes );
    QString functionBeginRegExp( const QString& funcName );
    QString functionEndRegExp( const QString& funcName );
    QList<Section> classSections(const ClassNode *classe, SynopsisStyle style);
    QList<Section> nonclassSections(const InnerNode *innerNode, SynopsisStyle style);
    const Node *resolveTarget( const QString& target, const Tree *tree, const Node *relative );

private:
    QString addMarkUp( const QString& protectedCode, const Node *relative, const QString& dirPath );
};

#endif
