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
#include "stringset.h"

class HtmlWriter;

class Decl
{
public:
    enum Kind { Root, Class, Function, Enum, EnumItem, Typedef, Property };
    enum Access { Public, Protected, Private };

    static QString ref( const QString& name );

    virtual ~Decl() { }

    void setDoc( Doc *doc );
    void setCurrentChildAccess( Access access ) { cura = access; }
    void setLocation( const Location& loc ) { lo = loc; }

    void buildMangledSymbolTables();
    virtual void buildPlainSymbolTables( bool omitUndocumented );
    void destructSymbolTables();

    void fillInDecls();
    void fillInDocs();

    Kind kind() const { return k; }
    Access access() const { return a; }
    Doc *doc() const { return d; }
    bool isInternal() const;
    bool isObsolete() const { return d != 0 && d->isObsolete(); }
    const QString& name() const { return n; }
    QString fullName() const;
    virtual QString mangledName() const;
    QString fullMangledName() const;
    virtual QString uniqueName() const;
    virtual QString sortName() const;
    QString ref() const { return ref( uniqueName() ); }
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

    virtual void fillInDeclsForThis() { }
    virtual void fillInDocsForThis() { }

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

class PropertyDecl;

class ClassDecl : public Decl
{
public:
    ClassDecl( const Location& loc, const QString& name, Decl *context );

    virtual void buildPlainSymbolTables( bool omitUndocumented );

    void setHeaderFile( const QString& headerFile ) { hfile = headerFile; }
    void addSuperType( const CodeChunk& superType )
    { supert.append( superType ); }
    void addProperty( PropertyDecl *prop ) { props.append( prop ); }

    ClassDoc *classDoc() const { return (ClassDoc *) doc(); }
    QString whatsThis() const;
    const QValueList<CodeChunk>& superTypes() const { return supert; }
    const QValueList<CodeChunk>& subTypes() const { return subt; }

    const QString& headerFile() const { return hfile; }
    virtual void printHtmlShort( HtmlWriter& out ) const;
    virtual void printHtmlLong( HtmlWriter& out ) const;

    const QValueList<PropertyDecl *>& properties() const { return props; }

protected:
    virtual void fillInDeclsForThis();
    virtual void fillInDocsForThis();

private:
#if defined(Q_DISABLE_COPY)
    ClassDecl( const ClassDecl& );
    ClassDecl& operator=( const ClassDecl& );
#endif

    void emitHtmlListOfAllMemberFunctions() const;

    QValueList<CodeChunk> supert;
    QValueList<CodeChunk> subt;
    QString hfile;
    QValueList<PropertyDecl *> props;
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
    typedef QValueList<Parameter> ParameterList;

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
    void borrowParameterNames( ParameterList::ConstIterator p );
    void setRelatedProperty( PropertyDecl *property ) { prop = property; }

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
    const ParameterList& parameters() const { return pl; }
    const StringSet& parameterNames() const { return ps; }
    PropertyDecl *relatedProperty() const { return prop; }

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
    PropertyDecl *prop;
};

class EnumDecl;

class EnumItemDecl : public Decl
{
public:
    EnumItemDecl( const Location& loc, const QString& name,
		  EnumDecl *parentEnum, const CodeChunk& value );

    virtual QString uniqueName() const;
    const CodeChunk& value() const { return v; }

    virtual void printHtmlShort( HtmlWriter& out ) const;

    EnumDecl *parentEnum() const { return enumDecl; }

private:
#if defined(Q_DISABLE_COPY)
    EnumItemDecl( const Decl& );
    EnumItemDecl& operator=( const Decl& );
#endif

    EnumDecl *enumDecl;
    CodeChunk v;
};

class EnumDecl : public Decl
{
public:
    typedef QValueList<EnumItemDecl *>::ConstIterator ItemIterator;

    EnumDecl( const Location& loc, const QString& name, Decl *context );

    virtual QString uniqueName() const;

    void addItem( EnumItemDecl *item ) { il.append( item ); }
    // ### remove these two functions
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

    QValueList<EnumItemDecl *> il;
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

/*
  The PropertyDecl class represents a Qt property. See
  http://doc.trolltech.com/properties.html.
*/
class PropertyDecl : public Decl
{
public:
    PropertyDecl( const Location& loc, const QString& name, Decl *context,
		  const CodeChunk& type );

    virtual QString uniqueName() const;

    void setReadFunction( const QString& getter ) { read = getter; }
    void setWriteFunction( const QString& setter ) { write = setter; }
    void setStored( bool stored ) { store = toTrool( stored ); }
    void setDesignable( bool designable ) { design = toTrool( designable ); }
    void setResetFunction( const QString& resetter ) { reset = resetter; }

    PropertyDoc *propertyDoc() const { return (PropertyDoc *) doc(); }
    const CodeChunk& dataType() const { return t; }
    const QString& readFunction() const { return read; }
    const QString& writeFunction() const { return write; }
    bool stored() const { return fromTrool( store, storedDefault() ); }
    bool storedDefault() const { return TRUE; }
    bool designable() const { return fromTrool( design, designableDefault() ); }
    bool designableDefault() const { return !write.isEmpty(); }
    const QString& resetFunction() const { return reset; }

    virtual void printHtmlShort( HtmlWriter& out ) const;
    virtual void printHtmlLong( HtmlWriter& out ) const;

private:
    /*
      A Trool is a bit like a bool, except that it admits three truth
      values (true, false and default).
    */
    enum Trool { Ttrue, Tfalse, Tdef };

    static Trool toTrool( bool b );
    static bool fromTrool( Trool tr, bool def );

    CodeChunk t;
    QString n;
    QString read;
    QString write;
    Trool store;
    Trool design;
    QString reset;
};

#endif
