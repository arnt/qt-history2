/*
  doc.h
*/

#ifndef DOC_H
#define DOC_H

#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>

#include "location.h"
#include "parsehelpers.h"
#include "stringset.h"
#include "walkthrough.h"

class BinaryWriter;
class HtmlWriter;
class Resolver;

class ExampleLocation
{
public:
    ExampleLocation() : ininc( TRUE ), ln( 0 ), uniq( 0 ) { }
    ExampleLocation( const QString& fileName, bool inInclude, int lineNo,
		     int unique )
	    : fname( fileName ), ininc( inInclude ), ln( lineNo ),
	      uniq( unique ) { }
    ExampleLocation( const ExampleLocation& el )
	    : fname( el.fname ), ininc( el.ininc ), ln( el.ln ),
	      uniq( el.uniq ) { }

    ExampleLocation& operator=( const ExampleLocation& el );

    const QString& fileName() const { return fname; }
    bool inInclude() const { return ininc; }
    int lineNum() const { return ln; }
    int uniqueNum() const { return uniq; }

private:
    QString fname;
    bool ininc;
    int ln;
    int uniq;
};

class DocParser;

/*
  The Doc class represents a doc comment.
*/
class Doc
{
    friend class DocParser;

public:
    enum Kind { Null, Fn, Class, Enum, Property, Page, Base64, Plainpage,
		Defgroup, Example };

    static Doc *create( const Location& loc, const QString& text );

    static void setResolver( const Resolver *resolver ) { res = resolver; }
    static void setHeaderFileList( const StringSet& headerFiles );
    static void setClassLists( const QMap<QString, QString>& allClasses,
			       const QMap<QString, QString>& mainClasses );
    static void setFunctionIndex( const QMap<QString, StringSet>& index );
    static void setGroupMap( const QMap<QString, QString>& groupMap );
    static void setClassHierarchy( const QMap<QString, StringSet>& hierarchy );
    static void printHtmlIncludeHeader( HtmlWriter& out,
					const QString& fileName );
    static const Resolver *resolver() { return res; }
    static QString href( const QString& name,
			 const QString& text = QString::null,
			 bool propertize = FALSE );
    static QString htmlLegaleseList();
    static QString htmlHeaderFileList();
    static QString htmlClassList();
    static QString htmlMainClassList();
    static QString htmlAnnotatedClassList();
    static QString htmlFunctionIndex();
    static QString htmlClassHierarchy();
    static QString htmlExtensionList();

    static QString htmlCompactList( const QMap<QString, QString>& list );
    static QString htmlNormalList( const QMap<QString, QString>& list );

    Doc( Kind kind, const Location& loc, const QString& htmlText,
	 const QString& name = QString::null,
	 const QString& whatsThis = QString::null );
    virtual ~Doc() { delete toc; }

    void setName( const QString& name ) { nam = name; }
    void setWhatsThis( const QString& whatsThis ) { whats = whatsThis; }
    void setFileName( const QString& fileName ) { fnam = fileName; }
    void setInternal( bool internal ) { inter = internal; }
    void setObsolete( bool obsolete ) { obs = obsolete; }
    void setPreliminary( bool preliminary ) { prel = preliminary; }
    void setSeeAlso( const QStringList& seeAlso ) { sa = seeAlso; }
    void setKeywords( const StringSet& keywords ) { kwords = keywords; }
    void setGroups( const StringSet& groups ) { gr = groups; }
    void setContainsExamples( const StringSet& included,
			      const StringSet& thruwalked );
    void setDependsOn( const StringSet& dependsOn ) { deps = dependsOn; }
    void setHtmlLegalese( const QString& legalese ) { q = legalese; }
    void setLink( const QString& link, const QString& title );
    void setTOC( QValueList<Section> *t ) {
	delete toc;
	toc = t;
    }

    Kind kind() const { return ki; }
    const Location& location() const { return lo; }
    const QString& name() const { return nam; }
    const QString& whatsThis() const { return whats; }
    const QString& fileName() const { return fnam; }
    bool isInternal() const { return inter; }
    bool isObsolete() const { return obs; }
    bool isPreliminary() const { return prel; }
    QString htmlSeeAlso() const;
    const StringSet& groups() const { return gr; }
    const StringSet& dependsOn() const { return deps; }

    void printHtml( HtmlWriter& out ) const;

    QString finalHtml() const;

protected:
    QString html;

    static StringSet extlist;
    static QMap<QString, QString> classext;

private:
#if defined(Q_DISABLE_COPY)
    Doc( const Doc& com );
    Doc& operator=( const Doc& com );
#endif

    QString htmlTableOfContents() const;

    Kind ki;
    Location lo;
    QString nam;
    QString whats;
    QString fnam;
    QStringList sa;
    bool inter;
    bool obs;
    bool prel;
    QString q;
    StringSet kwords;
    StringSet gr;
    StringSet deps;
    StringSet incl;
    StringSet thru;
    QString lnk;
    QValueList<Section> *toc;

    static const Resolver *res;
    static QRegExp *megaRegExp;
    static QMap<QString, QMap<QString, QString> > legaleses;
    static QMap<QString, QString> keywordLinks;
    static StringSet hflist;
    static QMap<QString, QString> clist;
    static QMap<QString, QString> mainclist;
    static QMap<QString, StringSet> findex;
    static QMap<QString, QString> grmap;
    static QMap<QString, StringSet> chierarchy;

    // QMap<example file, LinkMap>
    static QMap<QString, LinkMap> includeLinkMaps;
    static QMap<QString, LinkMap> walkthroughLinkMaps;

    // QMap<function link, QMap<score, ExampleLocation> >
    static QMap<QString, QMap<int, ExampleLocation> > megaExampleMap;

    static StringSet includedExamples;
    static StringSet thruwalkedExamples;

    // QMap<example file, example link>
    static QMap<QString, QString> includedExampleLinks;
    static QMap<QString, QString> thruwalkedExampleLinks;
};

class FnDoc : public Doc
{
public:
    FnDoc( const Location& loc, const QString& html, const QString& prototype,
	   const QString& relates, const StringSet& documentedParams,
	   bool overloads );

    void setOverloads( bool overloads ) { over = overloads; }

    const QString& prototype() const { return proto; }
    const QString& relates() const { return rel; }
    const StringSet& documentedParameters() const { return params; }
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
	      const StringSet& headers, const QStringList& important,
	      bool mainClass );

    const QString& brief() const { return bf; }
    const QString& module() const { return mod; }
    const QString& extension() const { return ext; }
    const StringSet& headers() const { return h; }
    const QStringList& important() const { return imp; }
    bool mainClass() const { return main; }

private:
    QString bf;
    QString mod;
    QString ext;
    StringSet h;
    QStringList imp;
    bool main;
};

class EnumDoc : public Doc
{
public:
    EnumDoc( const Location& loc, const QString& html, const QString& name,
	     const StringSet& documentedValues );

    const StringSet& documentedValues() const { return values; }

private:
    StringSet values;
};

class PropertyDoc : public Doc
{
public:
    PropertyDoc( const Location& loc, const QString& html, const QString& name,
		 const QString& brief );

    const QString& brief() const { return bf; }
    void setFunctions( const QString& read, const QString& readRef,
		       const QString& write, const QString& writeRef,
		       const QString& reset, const QString& resetRef );

private:
    QString bf;
};

class PageLikeDoc : public Doc
{
public:
    PageLikeDoc( Kind kind, const Location& loc, const QString& html,
		 const QString& title = QString::null );

    const QString& title() const { return ttl; }

private:
    QString ttl;
};

class PageDoc : public PageLikeDoc
{
public:
    PageDoc( const Location& loc, const QString& html,
	     const QString& fileName, const QString& title );
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
		 const QString& groupName, const QString& title );
};

class ExampleDoc : public PageLikeDoc
{
public:
    ExampleDoc( const Location& loc, const QString& html,
		const QString& fileName, const QString& title );
};

#endif
