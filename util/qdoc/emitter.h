/*
  emitter.h
*/

#ifndef EMITTER_H
#define EMITTER_H

#include <qmap.h>
#include <qstring.h>

#include "decl.h"
#include "htmlchunk.h"
#include "declresolver.h"
#include "stringset.h"

class DefgroupDoc;
class ExampleDoc;
class PageDoc;

class Emitter
{
public:
    Emitter() : resolver( 0 ) { }

    RootDecl *rootDecl() { return &root; }
    // ### needed?
    Decl *resolveMangled( const QString& fullMangledName ) const
    { return root.resolveMangled( fullMangledName ); }
    Decl *resolvePlain( const QString& fullName ) const
    { return root.resolvePlain( fullName ); }
    void addGroup( DefgroupDoc *doc );
    void addGroupie( Doc *groupie );
    void addPage( PageDoc *doc );
    void addExample( ExampleDoc *doc );
    void addHtmlChunk( const QString& link, const HtmlChunk& chk );
    void addLink( const QString& link, const QString& text );
    void nailDownDecls();
    void nailDownDocs();

    void emitHtml() const;

private:
#if defined(Q_DISABLE_COPY)
    Emitter( const Emitter& );
    Emitter& operator=( const Emitter& );
#endif

    void addHtmlFile( const QString& fileName );

    RootDecl root;
    DeclResolver *resolver;
    QMap<QString, DefgroupDoc *> groupdefs;
    QMap<QString, QMap<QString, Doc *> > groupiemap;
    QValueList<PageDoc *> pages;
    QValueList<ExampleDoc *> examples;
    QMap<QString, HtmlChunk> chkmap;
    QMap<QString, StringSet> lmap;

    StringSet eglist;
    StringSet hlist;
    StringSet htmllist;
    QMap<QString, QString> clist;
    QMap<QString, StringSet> findex;
    QMap<QString, StringSet> chierarchy;
    QMap<QString, QString> pmap;
    QMap<QString, StringSet> wmap;
};

#endif
