#ifndef DOCUPARSER_H
#define DOCUPARSER_H

#include <qxml.h>
#include <qptrlist.h>

class QString;

class ContentItem{
public:
    ContentItem( const QString &n, const QString &r, 
		 const QString &c, int d ) 
	: name( n ), ref( r ), cat( c ), depth( d ) {}
    QString getContentName() const { return name; }
    QString getContentRef() const  { return ref; }
    QString getDocuCategory() const { return cat; }
    int getDepth() const { return depth; }
private:
    QString name, ref, cat;
    int depth;
};

enum StateCon{
    StateInit,
    StateContent,
    StateDocTitle,
    StateSect,
    StateSectTitle 
};

enum StateInd{
    StateInitial,
    StateIndex,
    StateItem,
    StateLink
};

class DocuContentParser : public QXmlDefaultHandler
{
public:
    DocuContentParser();    
    bool startDocument();
    bool startElement( const QString&, const QString&, const QString& ,
                       const QXmlAttributes& );
    bool endElement( const QString&, const QString&, const QString& );
    bool characters( const QString & );
    QPtrList<ContentItem> getContentItems() { return contentList; } 
    bool fatalError( const QXmlParseException& exception );
    QString getCategory() { return category; }
    QString errorProtocol();
private:
    QString refBuf, catBuf, nameBuf, errorProt;
    QString category;
    int depth;
    StateCon state;
    QPtrList<ContentItem> contentList;    
};

class DocuIndexParser : public QXmlDefaultHandler
{
public:
    DocuIndexParser();
    bool startDocument();
    bool startElement( const QString&, const QString&, const QString& ,
                       const QXmlAttributes& );
    bool endElement( const QString&, const QString&, const QString& );
    bool characters( const QString & );
    bool fatalError( const QXmlParseException& exeption );
    QString errorProtocol();
    QStringList getIndices() { return indexlist; }
    QStringList getTitles()  { return titlelist; }
    QString getCategory()    { return category;  }
private:     
    QString wordBuf, descrBuf, refBuf, errorProt;
    QString category;
    QStringList indexlist, titlelist;
    StateInd state;
};


#endif //DOCUPARSER_H



