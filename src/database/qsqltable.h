#ifndef QSQLTABLE_H
#define QSQLTABLE_H

#ifndef QT_H
#include "qsqltablebase.h"
#include "qstring.h"
#include "qsql.h"
#include "qpainter.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlGrid : public QSqlTableBase
{
public:
    QSqlGrid ( QWidget * parent = 0, const char * name = 0 );
    ~QSqlGrid();

    void setQuery( const QString& query, const QString& databaseName = QSqlConnection::defaultDatabase, bool autoPopulate = TRUE );
    void setQuery( const QSql& query, bool autoPopulate = TRUE );
    QSql query() const;

protected:
    void paintCell ( QPainter * p, int row, int col, const QRect & cr,
		     bool selected );
    void setNumRows( int r );
private:
    QSql sql;
};

#endif
#endif
