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
    virtual QString functionBeginRegExp( const QString& funcName );
    virtual QString functionEndRegExp( const QString& funcName );
    virtual QList<ClassSection> classSections( const ClassNode *classe, SynopsisStyle style );
    virtual const Node *resolveTarget( const QString& target,
				       const Node *relative );

private:
    QString addMarkUp( const QString& protectedCode, const Node *relative,
		       const QString& dirPath );
};

#endif
