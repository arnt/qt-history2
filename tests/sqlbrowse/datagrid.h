#ifndef DATAGRID_H
#define DATAGRID_H

#include <qtable.h>
#include <qstring.h>
#include <qsqlrowset.h>
#include <qpainter.h>

class DataGrid : public QTable
{
    Q_OBJECT
public:
    DataGrid ( QWidget * parent = 0, const char * name = 0 );
    virtual ~DataGrid();
    void free();
    void take( const QSqlRowset& r );
protected:
    void paintCell ( QPainter * p, int row, int col, const QRect & cr, bool selected );
protected slots:
    void columnClicked ( int col );
private:
    QSqlRowset* rset;
};

#endif
