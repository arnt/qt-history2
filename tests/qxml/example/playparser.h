#include <qlistview.h>
#include <qxml.h>

class PlayParser : public QXmlDefaultHandler
{
public:
    PlayParser( QListView*, QString* );
    virtual ~PlayParser();

    bool startDocument();
    bool endDocument();
    bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts );
    bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName );
    bool characters( const QString& ch );

    bool fatalError( const QXmlParseException& exception );

private:
    void add( bool sub = FALSE );

    QListView *playTree;
    QListViewItem *playTreeItem;
    QListViewItem *playTreeItemAfter;
    QString *errorProtocol;

    QString treeText;
    QString itemText;
    bool rootElement;
    int level;
    int act;
    int scene;
};
