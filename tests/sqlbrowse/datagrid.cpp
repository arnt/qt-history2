#include "datagrid.h"
#include <qsqlindex.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>

#include <qsqleditor.h>
#include <qsqleditorfactory.h>

DataGrid::DataGrid ( QWidget * parent = 0, const char * name = 0 )
    : QTable(parent,name)
{
    setMinimumWidth(300);
    setMinimumHeight(300);
}

DataGrid::~DataGrid()
{
}

void DataGrid::free()
{
    if ( table )
	delete table;
}

void DataGrid::take( const QSqlView& r )
{
    free();
    table = new QSqlView( r );
    setNumCols( table->count() );
    int rows = table->size();
    if (rows > 0)
	setNumRows(rows);
    QHeader* h = horizontalHeader();
    for ( int j = 0; j < numCols(); ++j )
	h->setLabel( j, table->field(j).name() );
}

void DataGrid::paintCell( QPainter * p, int row, int col, const QRect & cr,
			  bool selected )
{
    QTable::paintCell(p,row,col,cr,selected);
    if ( table->seek(row) ) {
	QString text = (*table)[col].toString();
	if ( table->isNull(col) )
	    text = "<null>";
	p->drawText( 0,0, cr.width(), cr.height(), AlignLeft + AlignVCenter,
		     text );
    }
}

void DataGrid::columnClicked ( int col )
{
    if ( table ) {
	QSqlIndex newSort = QSqlIndex( table->name() );
	newSort.append( table->field( col ).name() );
	table->select( newSort );
	viewport()->repaint( TRUE );
    }
}

QWidget * DataGrid::createEditor( int row, int col, bool initFromCell ) const
{
    if( table->seek(row) ){
	qDebug("row:" + QString::number(row));
	qDebug("creating editor,id:" + (*table)["id"].toString() );	
	QWidget * p = QSqlEditorFactory::instance()->createEditor(
	                            viewport(),	table->field(col) );
	return 	p;
    } else {
	return NULL;
    }
}

void DataGrid::setCellContentFromEditor( int row, int col )
{
    QWidget *editor = cellWidget( row, col );
    if ( !editor )
        return;
    if ( editor->inherits("QSqlEditor") ) {
	table->seek( row );
	qDebug("done editing,id:" + (*table)["id"].toString() );    	
	( (QSqlEditor *) editor)->syncToField();
	table->update( table->primaryIndex() );
	table->select();
    }
}
