#ifndef SQLSPREADSHEET_H
#define SQLSPREADSHEET_H

#include "spreadsheet.h"
#include <qsql.h>
#include <qsqldatabase.h>
#include "sqlspreadsheetwindow.h"

class SQLSpreadsheet : public Spreadsheet
{
    Q_OBJECT
public:
    SQLSpreadsheet ( QWidget * parent = 0, const char * name = 0 );
    ~SQLSpreadsheet(){}
    void setDatabase( QSqlDatabase* database );
    void save();
protected:
    void createTable();
public slots:
    void newSelection( int row, int col );
signals:
    void newTextSelected( const QString& text);
private:
    void insertData(int row, int col, bool e, QString data);
    QSqlDatabase* db;
};

class SQLSpreadsheetWindow : public SpreadsheetWindow
{
    Q_OBJECT
public:
    SQLSpreadsheetWindow ( QWidget * parent=0, const char * name=0, WFlags f=0 );
    ~SQLSpreadsheetWindow();
private:
    QSqlDatabase db;
};

#endif
