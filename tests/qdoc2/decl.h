/*
  decl.h
*/

#ifndef DECL_H
#define DECL_H

#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>

#include "codechunk.h"
#include "doc.h"
#include "location.h"
#include "property.h"
#include "stringset.h"

class HtmlWriter;

class Decl
{
public:
    enum Kind { Root, Class, Function, Enum, Typedef };
    enum Access { Public, Protected, Private };

    static QString anchor( const QString& name );

    virtual ~Decl() { }

    void setDoc( Doc *doc );
    void setCurrentChildAccess( Access access ) { cura = access; }
    void setLocation( const Location& loc ) { lo = loc; }

    void buildMangledSymbolTables();
    void destructMangledSymbolTables();
    virtual void buildPlainSymbolTables();

    void fillInDecls();
    void fillInDocs();

    Kind kind() const { return k; }
    Access access() const { return a; }
    Doc *doc() const { return d; }
    bool internal() const { return d != 0 && d->internal(); }
    bool obsolete() const { return d != 0 && d->obsolete(); }
    const QString& name() const { return n; }
    QString fullName() const;
    virtual QString mangledName() const;
    QString fullMangledName() const;
    virtual QString uniqueName() const;
    virtual QString sortName() const;
    QString anchor() const { return anchor( uniqueName() ); }
    const Location& location() const { return lo; }
    Decl *context() const { return c; }
    Decl *relatesContext() const { return relc; }
    Decl *rootContext() const { return rootc; }
    void setReimplements( Decl *superDecl );
    void setRelates( Decl *context );

    Decl *resolveMangled( const QString& relOrFullMangledName ) const;
    Decl *resolvePlain( const QString& relOrFullName ) const;

    const QValueList<Decl *>& children() const { return all; }
    const QValueList<Decl *>& publicChildren() const { return pub; }
    const QValueList<Decl *>& protectedChildren() const { return prot; }
    const QValueList<Decl *>& privateChildren() const { return priv; }
    const QValueList<Decl *>& importantChildren() const { return imp; }
    const QValueList<Decl *>& relatedChildren() const { return rel; }

    Decl *reimplements() const { return reimp; }
    const QValueList<Decl *>& reimplementedBy() const { return reimpby; }

    virtual void printHtmlShort( HtmlWriter& out ) const;
    virtual void printHtmlLong( HtmlWriter& out ) const;

protected:
    Decl( Kind kind, const Location& loc, const QString& name, Decl *context );

    virtual void fillInDeclsThis();
    virtual void fillInDocsThis();

    void setImportantChildren( const QValueList<Decl *>& important );

    enum { MangledSymTable, PlainSymTable, NSymTables };
    QMap<QString, Decl *> symTable[NSymTables];

private:
#if defined(Q_DISABLE_COPY)
    Decl( const Decl& );
    Decl& operator=( const Decl& );
#endif

    Decl *resolveHere( int whichSymTable, const QString& rellName ) const;

    Kind k;
    Access a;
    Doc *d;
    Location lo;
    QString n;
    Access cura;
    Decl *c;
    Decl *relc;
    Decl *rootc;
    bool declsFilledIn;
    bool docsFilledIn;
    QValueList<Decl *> all;
    QValueList<Decl *> pub;
    QValueList<Decl *> prot;
    QValueList<Decl *> priv;
    QValueList<Decl *> imp;
    QValueList<Decl *> rel;
    Decl *reimp;
    QValueList<Decl *> reimpby;
};

class RootDecl : public Decl
{
public:
    RootDecl() : Decl( Root, Location(), QString(""), 0 ) { }

private:
#if defined(Q_DISABLE_COPY)
    RootDecl( const RootDecl& );
    RootDecl& operator=( const RootDecl& );
#endif
};

class ClassDecl : public Decl
{
public:
    ClassDecl( const Location& loc, const QString& name, Decl *context );

    virtual void buildPlainSymbolTables();

    void setHeaderFile( const QString& headerFile ) { hfile = headerFile; }
    void addSuperType( const CodeChunk& superType )
    { supert.append( superType ); }
    void addProperty( const Property& property ) { prop.append( property ); }

    ClassDoc *classDoc() const { return (ClassDoc *) doc(); }
    QString whatsThis() const;
    const QValueList<CodeChunk>& superTypes() const { return supert; }
    const QValueList<CodeChunk>& subTypes() const { return subt; }
    const QValueList<Property>& properties() const { return prop; }

    const QString& headerFile() const { return hfile; }
    virtual void printHtmlShort( HtmlWriter& out ) const;
    virtual void printHtmlLong( HtmlWriter& out ) const;

protected:
    virtual void fillInDeclsThis();
    virtual void fillInDocsThis();

private:
#if defined(Q_DISABLE_COPY)
    ClassDecl( const ClassDecl& );
    ClassDecl& operator=( const ClassDecl& );
#endif

    void emitHtmlListOfAllMemberFunctions() const;

    QValueList<CodeChunk> supert;
    QValueList<CodeChunk> subt;
    QValueList<Property> prop;
    QString hfile;
};

class Parameter
{
public:
    Parameter() { }
    Parameter( const CodeChunk& type, const QString& name = QString::null,
	       const CodeChunk& defaultValue = CodeChunk() );
    Parameter( const Parameter& p );

    Parameter& operator=( const Parameter& p );

    void setName( const QString& name ) { n = name; }

    const CodeChunk& dataType() const { return t; }
    const QString& name() const { return n; }
    const CodeChunk& defaultValue() const { return d; }

    void printHtmlShort( HtmlWriter& out ) const;
    void printHtmlLong( HtmlWriter& out, const Decl *context = 0 ) const;

private:
    CodeChunk t;
    QString n;
    CodeChunk d;
};

class FunctionDecl : public Decl
{
public:
    typedef QValueList<Parameter>::ConstIterator ParameterIterator; // ###

    FunctionDecl( const Location& loc, const QString& name, Decl *context,
		  const CodeChunk& returnType );

    virtual QString mangledName() const;
    virtual QString uniqueName() const;
    virtual QString sortName() const;

    void setConst( bool cons ) { c = cons; }
    void setStatic( bool stat ) { st = stat; }
    void setVirtual( bool vir ) { v = vir; }
    void setPure( bool pure ) { p = pure; }
    void setSignal( bool signal ) { si = signal; }
    void setSlot( bool slot ) { sl = slot; }
    void setOverloadNumber( int no );
    void addParameter( const Parameter& param );
    void borrowParameterNames( ParameterIterator p );
    const StringSet& parameterNames() const { return ps; }

    FnDoc *fnDoc() const { return (FnDoc *) doc(); }
    const CodeChunk& returnType() const { return r; }
    bool isConst() const { return c; }
    bool isStatic() const { return st; }
    bool isVirtual() const { return v; }
    bool isPure() const { return p; }
    bool isSignal() const { return si; }
    bool isSlot() const { return sl; }
    int overloadNumber() const { return ovo; }
    bool isConstructor() const;
    bool isDestructor() const;
    ParameterIterator parameterBegin() const { return pl.begin(); }
    ParameterIterator parameterEnd() const { return pl.end(); }

    virtual void printHtmlShort( HtmlWriter& out ) const;
    virtual void printHtmlLong( HtmlWriter& out ) const;

private:
#if defined(Q_DISABLE_COPY)
    FunctionDecl( const Decl& );
    FunctionDecl& operator=( const Decl& );
#endif

    CodeChunk r;
    bool c;
    bool st;
    bool v;
    bool p;
    bool si;
    bool sl;
    int ovo;
    QValueList<Parameter> pl;
    StringSet ps;
};

class EnumItem
{
public:
    EnumItem() { }
    EnumItem( const QString& ident, const CodeChunk& value );
    EnumItem( const EnumItem& item );

    EnumItem& operator=( const EnumItem& item );

    const QString& ident() const { return id; }
    const CodeChunk& value() const { return v; }

    void printHtml( HtmlWriter& out ) const;

private:
    QString id;
    CodeChunk v;
};

class EnumDecl : public Decl
{
public:
    typedef QValueList<EnumItem>::ConstIterator ItemIterator;

    EnumDecl( const Location& loc, const QString& name, Decl *context );

    void addItem( const EnumItem& item ) { il.append( item ); }
    ItemIterator itemBegin() const { return il.begin(); }
    ItemIterator itemEnd() const { return il.end(); }

    EnumDoc *enumDoc() const { return (EnumDoc *) doc(); }
    virtual void printHtmlShort( HtmlWriter& out ) const;
    virtual void printHtmlLong( HtmlWriter& out ) const;

private:
#if defined(Q_DISABLE_COPY)
    EnumDecl( const Decl& );
    EnumDecl& operator=( const Decl& );
#endif

    QValueList<EnumItem> il;
};

class TypedefDecl : public Decl
{
public:
    TypedefDecl( const Location& loc, const QString& name, Decl *context,
		 const CodeChunk& type );

    const CodeChunk& dataType() const { return t; }

    EnumDoc *enumDoc() const { return (EnumDoc *) doc(); }
    virtual void printHtmlShort( HtmlWriter& out ) const;
    virtual void printHtmlLong( HtmlWriter& out ) const;

private:
#if defined(Q_DISABLE_COPY)
    TypedefDecl( const Decl& );
    TypedefDecl& operator=( const Decl& );
#endif

    CodeChunk t;
};

#endif
