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
    enum Kind { Null, Fn, Class, Enum, Page, Base64, Plainpage, Defgroup,
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

    Doc( Kind kind, const Location& loc, const QString& htmlText,
	 const QString& name = QString::null,
	 const QString& whatsThis = QString::null );
    virtual ~Doc() { }

    void setName( const QString& name ) { nam = name; }
    void setWhatsThis( const QString& whatsThis ) { whats = whatsThis; }
    void setFileName( const QString& fileName ) { fnam = fileName; }
    void setInternal( bool internal ) { inter = internal; }
    void setObsolete( bool obsolete ) { obs = obsolete; }
    void setSeeAlso( const QStringList& seeAlso ) { sa = seeAlso; }
    void setKeywords( const StringSet& keywords ) { kwords = keywords; }
    void setGroups( const StringSet& groups ) { gr = groups; }
    void setDependsOn( const StringSet& dependsOn ) { deps = dependsOn; }
    void setHtmlMustQuote( const QString& quote ) { q = quote; }
    void setLink( const QString& link, const QString& title );

    Kind kind() const { return ki; }
    const Location& location() const { return lo; }
    const QString& name() const { return nam; }
    const QString& whatsThis() const { return whats; }
    const QString& fileName() const { return fnam; }
    bool internal() const { return inter; }
    bool obsolete() const { return obs; }
    QString htmlSeeAlso() const;
    const StringSet& groups() const { return gr; }
    const StringSet& dependsOn() const { return deps; }

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
    QString nam;
    QString whats;
    QString fnam;
    QStringList sa;
    bool inter;
    bool obs;
    QString q;
    StringSet kwords;
    StringSet gr;
    StringSet deps;
    QString lnk;

    static const Resolver *res;
    static QRegExp *megaRegExp;
    static QMap<QString, QMap<QString, QString> > quotes;
    static QMap<QString, QString> keywordLinks;
    static StringSet hflist;
    static QMap<QString, QString> clist;
    static QMap<QString, StringSet> findex;
    static QMap<QString, StringSet> chierarchy;

protected: // ### evil
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
	      const StringSet& headers, const QStringList& important );

    const QString& brief() const { return bf; }
    const QString& module() const { return mod; }
    const QString& extension() const { return ext; }
    const StringSet& headers() const { return h; }
    const QStringList& important() const { return imp; }

private:
    QString bf;
    QString mod;
    QString ext;
    StringSet h;
    QStringList imp;
};

class EnumDoc : public Doc
{
public:
    EnumDoc( const Location& loc, const QString& html, const QString& name );
};

class PageLikeDoc : public Doc
{
public:
    PageLikeDoc( Kind kind, const Location& loc, const QString& html,
		 const QString& title = QString::null,
		 const QString& heading = QString::null );

    const QString& title() const { return ttl; }
    QString heading() const;

private:
    QString ttl;
    QString hding;
};

class PageDoc : public PageLikeDoc
{
public:
    PageDoc( const Location& loc, const QString& html,
	     const QString& fileName, const QString& title,
	     const QString& heading );
};

class Base64Doc : public PageLikeDoc
{
public:
    Base64Doc( const Location& loc, const QString& html,
	       const QString& fileName );

    void print( BinaryWriter& out );
};

class PlainpageDoc : public PageLikeDoc
{
public:
    PlainpageDoc( const Location& loc, const QString& html,
		  const QString& fileName );

    void print( BinaryWriter& out );
};

class DefgroupDoc : public PageLikeDoc
{
public:
    DefgroupDoc( const Location& loc, const QString& html,
		 const QString& groupName, const QString& title,
		 const QString& heading );
};

class ExampleDoc : public PageLikeDoc
{
public:
    ExampleDoc( const Location& loc, const QString& html,
		const QString& fileName, const QString& title,
		const QString& heading );
};

#endif
