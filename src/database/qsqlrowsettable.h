#ifndef QSQLROWSETTABLE_H
#define QSQLROWSETTABLE_H

#ifndef QT_H
#include "qsqltablebase.h"
#include "qstring.h"
#include "qsqlrowset.h"
#include "qpainter.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlRowsetTable : public QSqlSortedTable
{
    Q_OBJECT
public:
    QSqlRowsetTable ( QWidget * parent = 0, const char * name = 0 );
    ~QSqlRowsetTable();

    void setRowset( const QString& name, const QString& databaseName = QSqlConnection::defaultDatabase, bool autoPopulate = TRUE );
    void setRowset( const QSqlRowset& rowset, bool autoPopulate = TRUE );
    QSqlRowset rowset() const;

protected:
    void paintCell ( QPainter * p, int row, int col, const QRect & cr,
		     bool selected );
    bool         rowExists( int r );
    QSqlIndex currentSort();
    QSqlField field( int i );
    void setSort( QSqlIndex i );

private:
    QSqlRowset rset;
};

#endif
#endif
