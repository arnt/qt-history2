/*
  qscodemarker.h
*/

#ifndef QSCODEMARKER_H
#define QSCODEMARKER_H

#include "codemarker.h"

class QsCodeMarker : public CodeMarker
{
public:
    QsCodeMarker();
    ~QsCodeMarker();

    virtual bool recognizeCode( const QString& code );
    virtual bool recognizeExtension( const QString& ext );
    virtual bool recognizeLanguage( const QString& lang );
    virtual QString markedUpCode( const QString& code, const Node *relative,
				  const QString& dirPath );
    virtual QString markedUpSynopsis( const Node *node, const Node *relative,
				      SynopsisStyle style );
    virtual QString markedUpName( const Node *node );
    virtual QString markedUpFullName( const Node *node, const Node *relative );
    virtual QString markedUpIncludes( const QStringList& includes );
    virtual QList<ClassSection> classSections( const ClassNode *classe, SynopsisStyle style );
    virtual QString functionBeginRegExp( const QString& funcName );
    virtual QString functionEndRegExp( const QString& funcName );
    virtual const Node *resolveTarget( const QString& target,
				       const Node *relative );
};

#endif
