/*
  plaincodemarker.h
*/

#ifndef PLAINCODEMARKER_H
#define PLAINCODEMARKER_H

#include "codemarker.h"

class PlainCodeMarker : public CodeMarker
{
public:
    PlainCodeMarker();
    ~PlainCodeMarker();

    bool recognizeCode( const QString& code );
    bool recognizeExtension( const QString& ext );
    bool recognizeLanguage( const QString& lang );
    QString plainName( const Node *node );
    QString plainFullName( const Node *node, const Node *relative );
    QString markedUpCode( const QString& code, const Node *relative, const QString& dirPath );
    QString markedUpSynopsis( const Node *node, const Node *relative,
        		      SynopsisStyle style );
    QString markedUpName( const Node *node );
    QString markedUpFullName( const Node *node, const Node *relative );
    QString markedUpIncludes( const QStringList& includes );
    QString functionBeginRegExp( const QString& funcName );
    QString functionEndRegExp( const QString& funcName );
    QList<Section> classSections(const ClassNode *classe, SynopsisStyle style);
    QList<Section> nonclassSections(const InnerNode *innerNode, SynopsisStyle style);
    const Node *resolveTarget(const QString &target, const Node *relative);
};
  
#endif
