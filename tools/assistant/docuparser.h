#ifndef DOCUPARSER_H
#define DOCUPARSER_H

#include <qxml.h>
#include <qptrlist.h>

class QString;

struct ContentItem {
    ContentItem( const QString &t, const QString &r, int d )
	: title( t ), reference( r ), depth( d ) {}
    QString title;
    QString reference;
    int depth;
};

struct IndexItem {
    IndexItem( const QString &k, const QString &r )
	: keyword( k ), reference( r ) {}
    QString keyword;
    QString reference;
};

enum States{
    StateInit,
    StateContent,
    StateSect,
    StateKeyword
};

class DocuParser : public QXmlDefaultHandler
{
public:
    DocuParser();    
    bool startDocument();
    bool startElement( const QString&, const QString&, const QString& ,
                       const QXmlAttributes& );
    bool endElement( const QString&, const QString&, const QString& );
    bool characters( const QString & );
    bool fatalError( const QXmlParseException& exception );
    QString errorProtocol();
    
    QPtrList<ContentItem> getContentItems();
    QPtrList<IndexItem> getIndexItems();
    QString getCategory();
    
private:
    QString category, contentRef, indexRef, errorProt;
    QString title;
    int depth;
    States state;
    QPtrList<ContentItem> contentList;
    QPtrList<IndexItem> indexList;
};

#endif //DOCUPARSER_H
