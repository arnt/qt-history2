#ifndef BROWSE_H
#define BROWSE_H

#include <qtable.h>
#include <qstring.h>
#include <qsql.h>
#include <qsqlfield.h>
#include <qsqldatabase.h>
#include <qsqlresult.h>
#include <qmessagebox.h>
#include <qpainter.h>

class Browse : public QTable
{
    Q_OBJECT
public:
    Browse ( QWidget * parent = 0, const char * name = 0 )
	: QTable(parent,name),
	  res(0)
    {
        setMinimumWidth(300);
	setMinimumHeight(300);
    }
    ~Browse()
    {
	free();
    }
    void free()
    {
	if ( res ) {
	    delete res;
	    res = 0;
	}
    }
    void take( const QSql& r )
    {
	free();
	res = new QSql( r );
	QSqlFieldList flist = res->fields();
	setNumCols( flist.count() );
	int rows = res->size();
	if (rows > 0)
	    setNumRows(rows);
	else
	    setNumRows(10);
	QHeader* h = horizontalHeader();
	for ( int j = 0; j < numCols(); ++j )
	    h->setLabel( j, flist[j].name() );
    }
protected:
    void paintCell ( QPainter * p, int row, int col, const QRect & cr, bool selected )
    {
        QTable::paintCell(p,row,col,cr,selected);
	if ( res->seek(row) ) {
 	    QString text = (*res)[col].toString();
 	    if ( res->isNull(col) )
 		text = "<null>";
             p->drawText( 0,0, cr.width(), cr.height(), AlignLeft + AlignVCenter, text );
	}
    }
private:
    QSql* res;
};

#endif
