#include <qlistview.h>
#include <qlabel.h>
#include <qxml.h>

class XMLParser : public QXmlDefaultHandler
{
public:
    XMLParser( QListView*, QLabel*, QListView*, QTextStream* );
    virtual ~XMLParser();

    virtual void setDocumentLocator( QXmlLocator* locator );
    virtual bool startDocument();
    virtual bool endDocument();
    virtual bool startPrefixMapping( const QString& prefix, const QString& uri );
    virtual bool endPrefixMapping( const QString& prefix );
    virtual bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts );
    virtual bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName );
    virtual bool characters( const QString& ch );
    virtual bool ignorableWhitespace( const QString& ch );
    virtual bool processingInstruction( const QString& target, const QString& data );
    virtual bool skippedEntity( const QString& name );

    virtual bool warning( const QXmlParseException& exception );
    virtual bool error( const QXmlParseException& exception );
    virtual bool fatalError( const QXmlParseException& exception );

    virtual bool notationDecl( const QString& name, const QString& publicId, const QString& systemId );
    virtual bool unparsedEntityDecl( const QString& name, const QString& publicId, const QString& systemId, const QString& notationName );

    virtual bool resolveEntity( const QString& publicId, const QString& systemId, QXmlInputSource* ret );

    virtual bool startDTD( const QString& name, const QString& publicId, const QString& systemId );
    virtual bool endDTD();
    virtual bool startCDATA();
    virtual bool startEntity( const QString& name );
    virtual bool endEntity( const QString& name );
    virtual bool endCDATA();
    virtual bool comment( const QString& ch );

    virtual bool attributeDecl( const QString& eName, const QString& aName, const QString& type, const QString& valueDefault, const QString& value );
    virtual bool internalEntityDecl( const QString& name, const QString& value );
    virtual bool externalEntityDecl( const QString& name, const QString& publicId, const QString& systemId );

private:
    void addToProtocol( const QString& name, const QString &args );

    QXmlLocator *loc;
    QListView *parseProtocol;
    QTextStream *parseProtocolTS;
    QListViewItem *parseProtocolItem;
    QLabel *errorProtocol;
    QListView *tree;
    QListViewItem *treeItem;
    QListViewItem *treeItemAfter;
};
