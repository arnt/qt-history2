/*
  jscodemarker.h
*/

#ifndef JSCODEMARKER_H
#define JSCODEMARKER_H

#include "codemarker.h"

class JsCodeMarker : public CodeMarker
{
public:
    JsCodeMarker();
    ~JsCodeMarker();

    virtual QString markedUpCode( const QString& code, const Node *relative,
				  const QString& dirPath ) const;
    virtual QString markedUpSynopsis( const Node *node, const Node *relative,
				      SynopsisStyle style ) const;
    virtual QString markedUpName( const Node *node ) const;
    virtual QString markedUpFullName( const Node *node,
				      const Node *relative ) const;
    virtual QString markedUpIncludes( const QStringList& includes ) const;
    virtual const Node *resolveTarget( const QString& target,
				       const Node *relative ) const;
    virtual bool recognizeCode( const QString& code ) const;
    virtual bool recognizeExtension( const QString& ext ) const;
    virtual bool recognizeLanguage( const QString& lang ) const;
};

#endif
