/*
    javacodemarker.h
*/

#ifndef JAVACODEMARKER_H
#define JAVACODEMARKER_H

#include "codemarker.h"

class JavaCodeMarker : public CodeMarker
{
public:
    JavaCodeMarker();
    ~JavaCodeMarker();

    bool recognizeCode( const QString& code );
    bool recognizeExtension( const QString& ext );
    bool recognizeLanguage( const QString& lang );
    QString plainName(const Node *node);
    QString plainFullName(const Node *node, const Node *relative);
    QString markedUpCode( const QString& code, const Node *relative,
        		  const QString& dirPath );
    QString markedUpSynopsis( const Node *node, const Node *relative,
        		      SynopsisStyle style );
    QString markedUpName( const Node *node );
    QString markedUpFullName( const Node *node, const Node *relative );
    QString markedUpEnumValue(const QString &enumValue, const Node *relative);
    QString markedUpIncludes( const QStringList& includes );
    QList<Section> sections(const InnerNode *innerNode, SynopsisStyle style, Status status);
    QString functionBeginRegExp( const QString& funcName );
    QString functionEndRegExp( const QString& funcName );
    const Node *resolveTarget( const QString& target, const Tree *tree, const Node *relative );
};

#endif
