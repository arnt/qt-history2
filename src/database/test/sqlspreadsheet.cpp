#include "sqlspreadsheet.h"

SQLSpreadsheet::SQLSpreadsheet ( QWidget * parent, const char * name )
    : Spreadsheet(parent,name),
      db(0)
{
}

void SQLSpreadsheet::setDatabase( QSqlDatabase* database )
{
    db = database;
    createTable();
    QSql r = db->query("select r,c,e,d from sheet1;");
    while ( r.next() ) {
	QString data = r[3].toString().stripWhiteSpace();
	if ( r[2].toInt() ) // equation?
	    setItem( r[0].toInt(), r[1].toInt(), new EquationCell( data, this) );
	else
	    setItem( r[0].toInt(), r[1].toInt(), new SpreadsheetCell( data, this) );
    }
}

void SQLSpreadsheet::save()
{
    if ( db ) {
	db->exec("drop table sheet1;");
	db->transaction();
	createTable();
	for ( int r = 0; r < numRows(); ++r ) {
	    for ( int c = 0; c < numCols(); ++c ) {
		QTableItem* sc = item( r, c);
		if ( sc ) {
		    QString data;
		    if ( sc->editType() == QTableItem::Never )
			data = ((EquationCell*)sc)->equation();
		    else
			data = sc->text( );
		    insertData( r, c, sc->editType() == QTableItem::Never, data );
		}
	    }
	}
	db->commit();
    }
}

void SQLSpreadsheet::createTable()
{
    if ( db ) {
	db->exec("create table sheet1"
		 "(r int,"
		 "c int,"
		 "e int,"
		 "d char(50));"); // ignore errors
    }
}


void SQLSpreadsheet::insertData(int row, int col, bool e, QString data)
{
    if ( db && data.length() ) {
	db->exec("insert into sheet1 (r,c,e,d) values"
		 "(" + QString::number(row) + "," + QString::number(col) + "," + QString::number(e) + ",'" + data + "');");
    }
}

//////////

SQLSpreadsheetWindow::SQLSpreadsheetWindow ( QWidget * parent, const char * name, WFlags f )
    : SpreadsheetWindow(parent, name, f),
      db(qApp->argv()[1])
{
    db.reset(qApp->argv()[2], qApp->argv()[3], qApp->argv()[4], qApp->argv()[5]);
    if ( !db.open() ) {
	setCaption("Unable to Connect");
	qWarning(db.lastError().databaseText());
    } else {
	setCaption("Connected - " + QString(qApp->argv()[1]) + " " + QString(qApp->argv()[2]) + " " + QString(qApp->argv()[3]) + " " + QString(qApp->argv()[4]) + " " +  QString(qApp->argv()[5]) );
    }
    spreadsheet->setDatabase(&db);
}

SQLSpreadsheetWindow::~SQLSpreadsheetWindow()
{
    spreadsheet->save();
    db.close();
}


