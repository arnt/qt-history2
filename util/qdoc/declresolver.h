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
    virtual QString resolvefn( const QString& name ) const;
    virtual bool changedSinceLastRun( const Location& location,
				      const QString& link,
				      const QString& html ) const;

    void setCurrentClass( const ClassDecl *classDecl ) { c = classDecl; }
    void setHeaderFileList( const StringSet& headerFiles ) { h = headerFiles; }
    void setHtmlFileList( const StringSet& htmlFiles ) { html = htmlFiles; }
    void setHtmlChunkMap( const QMap<QString, HtmlChunk>& chunkMap )
    { chkmap = chunkMap; }

private:
#if defined(Q_DISABLE_COPY)
    DeclResolver( const DeclResolver& );
    DeclResolver& operator=( const DeclResolver& );
#endif

    const Decl *r;
    const ClassDecl *c;
    StringSet h;
    StringSet html;
    QMap<QString, HtmlChunk> chkmap;
};

#endif
