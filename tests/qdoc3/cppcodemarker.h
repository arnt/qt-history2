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

    virtual QString markedUpCode( const QString& code, const Node *relative,
				  const QString& dirPath ) const;
    virtual QString markedUpSynopsys( const Node *node,
				      const Node *relative,
				      SynopsysStyle style ) const;
    virtual QString markedUpName( const Node *node ) const;
    virtual QString markedUpFullName( const Node *node ) const;
    virtual QString markedUpIncludes( const QStringList& includes ) const;
    virtual const Node *resolveTarget( const QString& target,
				       const Node *relative ) const;
    virtual bool recognizeCode( const QString& code ) const;
    virtual bool recognizeExtension( const QString& ext ) const;
    virtual bool recognizeLanguage( const QString& lang ) const;

private:
    QString addMarkUp( const QString& protectedCode, const Node *relative,
		       const QString& dirPath ) const;
};

#endif
