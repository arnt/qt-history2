#include <qxml.h>
#include <qstringlist.h>

class QuoteHandler : public QXmlDefaultHandler
{
public:
    QuoteHandler();
    virtual ~QuoteHandler();

    // return the list of quotes
    QStringList quotes();

    // return the error protocol if parsing failed
    QString errorProtocol();

    // overloaded handler functions
    bool startDocument();
    bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts );
    bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName );
    bool characters( const QString& ch );

    QString errorString();

    bool fatalError( const QXmlParseException& exception );

private:
    QStringList quoteList;
    QString errorProt;

    QString author;
    QString reference;

    enum State {
	StateInit,
	StateDocument,
	StateQuote,
	StateLine,
	StateHeading,
	StateP
    };
    State state;
};
