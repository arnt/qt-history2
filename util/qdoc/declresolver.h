/*
  declresolver.h
*/

#ifndef DECLRESOLVER_H
#define DECLRESOLVER_H

#include <qmap.h>

#include "htmlchunk.h"
#include "resolver.h"
#include "stringset.h"

class Decl;
class ClassDecl;

/*
  The DeclResolver class is a Resolver that resolves identifiers using a Decl
  tree.
*/
class DeclResolver : public Resolver
{
public:
    DeclResolver( const Decl *root );

    virtual QString resolve( const QString& name ) const;
    virtual bool changedSinceLastRun( const QString& link,
				      const QString& html ) const;
    virtual bool warnChangedSinceLastRun( const Location& loc,
					  const QString& link,
					  const QString& html ) const;

    virtual QString relatedProperty( const QString& name ) const;

    void setCurrentClass( const ClassDecl *classDecl ) { c = classDecl; }
    void setExampleFileList( const StringSet& exampleFiles )
    { eg = exampleFiles; }
    void setHeaderFileList( const StringSet& headerFiles ) { h = headerFiles; }
    void setHtmlFileList( const StringSet& htmlFiles ) { html = htmlFiles; }
    void setHtmlChunkMap( const QMap<QString, HtmlChunk>& chunkMap )
    { chkmap = chunkMap; }

private:
    Q_DISABLE_COPY(DeclResolver)

    QString resolved( const Decl *decl ) const;

    const Decl *r;
    const ClassDecl *c;
    StringSet eg;
    StringSet h;
    StringSet html;
    QMap<QString, HtmlChunk> chkmap;
};

#endif
