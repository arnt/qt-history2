#ifndef QSQLINDEX_H
#define QSQLINDEX_H

#ifndef QT_H
#include "qstring.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlIndex : public QSqlFieldList
{
public:
    QSqlIndex( const QString& tablename = QString::null, const QString& name = QString::null );
    QSqlIndex( const QSqlIndex& other );
    ~QSqlIndex();
    QSqlIndex&       operator=( const QSqlIndex& other );
    QString          tableName() const { return table; }
    void             setTableName( const QString& tableName ) { table = tableName; }
    void             setName( const QString& name );
    QString          name() const;

private:
    QString          table;
    QString          nm;
};

#endif	// QT_NO_SQL
#endif


