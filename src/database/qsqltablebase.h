#ifndef QSQLTABLEBASE_H
#define QSQLTABLEBASE_H

#ifndef QT_H
#include "qvaluelist.h"
#include "qstring.h"
#include "qtable.h"
#include "qsqlfield.h"
#include "qsqlindex.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlTableBase : public QTable
{
    Q_OBJECT
public:
    QSqlTableBase ( QWidget * parent = 0, const char * name = 0 );
    ~QSqlTableBase();

    void setNullText( const QString& nullText ) { nullTxt = nullText; }
    QString nullText() const { return nullTxt; }

    void addColumn( const QSqlField& field );
    void removeColumn( uint col );
    void setColumn( uint col, const QSqlField& field );

protected:
    QWidget *    createEditor( int row, int col, bool initFromCell ) const;
    int          indexOf( uint i );
    virtual void reset();
    void         setNumRows ( int r );
    //    void         setCurrentCell( int row, int col );    
    virtual bool rowExists( int r ) = 0;

protected slots:    
    void         loadNextPage();
    void         loadNextLine();
    void         loadLine( int l );
    
private:
    QString      nullTxt;
    typedef      QValueList< uint > ColIndex;
    ColIndex     colIndex;
    bool         haveAllRows;    
    void         setNumCols ( int r );
};

class Q_EXPORT QSqlSortedTable : public QSqlTableBase
{
    Q_OBJECT
public:
    QSqlSortedTable ( QWidget * parent = 0, const char * name = 0 );
    ~QSqlSortedTable();

protected:
    void columnClicked ( int col );
    virtual QSqlIndex currentSort() = 0;
    virtual QSqlField field( int i ) = 0;
    virtual void setSort( QSqlIndex i );
};


#endif
#endif
