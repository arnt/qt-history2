#ifndef DATAGRID_H
#define DATAGRID_H

#include <qvaluelist.h>
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
    
    void take( QSqlRowset* r, bool autoCreate = TRUE );
    
    void setNullText( const QString& nullText ) { nullTxt = nullText; }
    QString nullText() const { return nullTxt; }
    
    void addColumn( const QSqlField& field );
    void removeColumn( uint col );
    void setColumn( uint col, const QSqlField& field );
    
protected:
    void paintCell ( QPainter * p, int row, int col, const QRect & cr,
		     bool selected );
    QWidget * createEditor( int row, int col, bool initFromCell ) const;
    
private:
    QSqlRowset* rset;
    QString nullTxt;
    typedef QValueList< uint > ColIndex;
    ColIndex colIndex;
        
    //    void      setCellContentFromEditor( int row, int col );
    //protected slots:
//    void columnClicked ( int col );
};

#endif
