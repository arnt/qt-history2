/*
  doc.h
*/

#ifndef DOC_H
#define DOC_H

#include <qmap.h>
#include <qstring.h>

#include "location.h"
#include "stringset.h"

class BinaryWriter;
class HtmlWriter;
class Resolver;

/*
  The Doc class represents a doc comment.
*/
class Doc
{
public:
    enum Kind { Null, Fn, Class, Enum, Page, Base64, Base256, Defgroup,
		Example };

    static Doc *create( const Location& loc, const QString& text );

    static void setResolver( const Resolver *resolver ) { res = resolver; }
    static void setHeaderFileList( const StringSet& headerFiles ); // ### need?
    static void setClassList( const QMap<QString, QString>& classList ); // ###
    static void setFunctionIndex( const QMap<QString, StringSet>& index );
    static void setClassHierarchy( const QMap<QString, StringSet>& hierarchy );
    static void printHtmlIncludeHeader( HtmlWriter& out,
					const QString& fileName );
    static const Resolver *resolver() { return res; }
    static QString href( const QString& name,
			 const QString& text = QString::null );
    static QString htmlQuoteList();
    static QString htmlHeaderFileList();
    static QString htmlClassList();
    static QString htmlAnnotatedClassList();
    static QString htmlFunctionIndex();
    static QString htmlClassHierarchy();
    static QString htmlExtensionList();

    Doc( Kind kind, const Location& loc, const QString& htmlText );
    virtual ~Doc() { }

    void setInternal( bool internal ) { inter = internal; }
    void setObsolete( bool obsolete ) { obs = obsolete; }
    void setSeeAlso( const QStringList& seeAlso ) { sa = seeAlso; }
    void setIndex( const StringSet& index ) { idx = index; }
    void setHtmlMustQuote( const QString& quote ) { q = quote; }
    void setLink( const QString& link, const QString& title );

    Kind kind() const { return ki; }
    const Location& location() const { return lo; }
    bool internal() const { return inter; }
    bool obsolete() const { return obs; }
    bool changedSinceLastRun() const;
    QString htmlSeeAlso() const;

    void printHtml( HtmlWriter& out ) const;

protected:
    const QString& htmlData() const { return html; }

private:
#if defined(Q_DISABLE_COPY)
    Doc( const Doc& com );
    Doc& operator=( const Doc& com );
#endif

    QString finalHtml() const;

    Kind ki;
    Location lo;
    QString html;
    QStringList sa;
    bool inter;
    bool obs;
    QString q;
    StringSet idx;
    QString lnk;

    static const Resolver *res;
    static QRegExp *megaRegExp;
    static QMap<QString, QMap<QString, QString> > quotes;
    static QMap<QString, QString> indices;
    static StringSet hflist;
    static QMap<QString, QString> clist;
    static QMap<QString, StringSet> findex;
    static QMap<QString, StringSet> chierarchy;
protected:
    static StringSet extlist;
    static QMap<QString, QString> classext;
};

class FnDoc : public Doc
{
public:
    FnDoc( const Location& loc, const QString& html, const QString& prototype,
	   const QString& relates, const StringSet& parameters,
	   bool overloads );

    void setOverloads( bool overloads ) { over = overloads; }

    const QString& prototype() const { return proto; }
    const QString& relates() const { return rel; }
    const StringSet& parameterNames() const { return params; }
    bool overloads() const { return over; }

private:
    QString proto;
    QString rel;
    StringSet params;
    bool over;
};

class ClassDoc : public Doc
{
public:
    ClassDoc( const Location& loc, const QString& html,
	      const QString& className, const QString& brief,
	      const QString& module, const QString& extension,
	      const StringSet& groups, const StringSet& headers,
	      const QStringList& important );

    const QString& className() const { return cname; }
    const QString& brief() const { return bf; }
    const QString& whatsThis() const { return whats; }
    const QString& module() const { return mod; }
    const QString& extension() const { return ext; }
    const StringSet& groups() const { return ingroups; }
    const StringSet& headers() const { return h; }
    const QStringList& important() const { return imp; }

private:
    QString cname;
    QString bf;
    QString whats;
    QString mod;
    QString ext;
    StringSet ingroups;
    StringSet h;
    QStringList imp;
};

class EnumDoc : public Doc
{
public:
    EnumDoc( const Location& loc, const QString& html,
	     const QString& enumName );

    const QString& enumName() const { return ename; }

private:
    QString ename;
};

class PageDoc : public Doc
{
public:
    PageDoc( const Location& loc, const QString& html, const QString& fileName,
	     const QString& title );

    const QString& fileName() const { return fname; }
    const QString& title() const { return titl; }

private:
    QString fname;
    QString titl;
};

class Base64Doc : public Doc
{
public:
    Base64Doc( const Location& loc, const QString& html,
	       const QString& fileName );

    const QString& fileName() const { return fname; }

    void print( BinaryWriter& out );

private:
    QString fname;
};

class Base256Doc : public Doc
{
public:
    Base256Doc( const Location& loc, const QString& html,
		const QString& fileName );

    const QString& fileName() const { return fname; }

    void print( BinaryWriter& out );

private:
    QString fname;
};

class DefgroupDoc : public Doc
{
public:
    DefgroupDoc( const Location& loc, const QString& html,
		 const QString& groupName, const QString& title );

    const QString& groupName() const { return gname; }
    const QString& title() const { return titl; }

private:
    QString gname;
    QString titl;
};

class ExampleDoc : public Doc
{
public:
    ExampleDoc( const Location& loc, const QString& html,
		const QString& fileName );

    const QString& fileName() const { return fname; }

private:
    QString fname;
};

#endif
