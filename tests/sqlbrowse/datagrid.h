#ifndef DATAGRID_H
#define DATAGRID_H

#include <qtable.h>
#include <qstring.h>
#include <qsqlview.h>
#include <qpainter.h>

class DataGrid : public QTable
{
    Q_OBJECT
public:
    DataGrid ( QWidget * parent = 0, const char * name = 0 );
    virtual ~DataGrid();
    void free();
    void take( const QSqlView& r );
protected:
    void paintCell ( QPainter * p, int row, int col, const QRect & cr,
		     bool selected );
    QWidget * createEditor( int row, int col, bool initFromCell ) const;
    void      setCellContentFromEditor( int row, int col );

protected slots:
    void columnClicked ( int col );
private:
    QSqlView* table;
};

#endif
