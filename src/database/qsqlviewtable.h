#ifndef QSQLVIEWTABLE_H
#define QSQLVIEWTABLE_H

#ifndef QT_H
#include "qsqltablebase.h"
#include "qstring.h"
#include "qsqlview.h"
#include "qpainter.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlViewTable : public QSqlSortedTable
{
    Q_OBJECT
public:
    QSqlViewTable ( QWidget * parent = 0, const char * name = 0 );
    ~QSqlViewTable();

    void setView( const QString& name, const QString& databaseName = QSqlConnection::defaultDatabase, bool autoPopulate = TRUE );
    void setView( const QSqlView& view, bool autoPopulate = TRUE );
    QSqlView view() const;

protected:
    void paintCell ( QPainter * p, int row, int col, const QRect & cr,
		     bool selected );
    bool         rowExists( int r );    
    QSqlIndex currentSort();
    QSqlField field( int i );
    void setSort( QSqlIndex i );

private:
    QSqlView vw;
};

#endif
#endif
