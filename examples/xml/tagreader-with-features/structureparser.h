/*
$Id$
*/  

#include <qxml.h>

class QTable;
class QString;

class StructureParser: public QXmlDefaultHandler
{
public:
    StructureParser( QTable * );
    bool startDocument();
    bool startElement( const QString&, const QString&, const QString& , 
                       const QXmlAttributes& );
    bool endElement( const QString&, const QString&, const QString& );
    bool endDocument();

private:
    int generation;
    int row;
    int column;
    QTable * table;

};                   
