/*
$Id$
*/  

#ifndef STRUCTUREPARSER_H
#define STRUCTUREPARSER_H   

#include <qxml.h>
#include <qstack.h>

class QListView;
class QListViewItem;
class QString;

class StructureParser: public QXmlDefaultHandler
{
public:
    StructureParser( QListView * );
    bool startElement( const QString&, const QString&, const QString& , 
                       const QXmlAttributes& );
    bool endElement( const QString&, const QString&, const QString& );

    void setListView( QListView * );

private:
    QStack<QListViewItem> stack;
    QListView * table;
};                   

#endif
