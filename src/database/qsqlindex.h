#ifndef QSQLINDEX_H
#define QSQLINDEX_H

#ifndef QT_H
#include "qstring.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlIndex
{
public:
    QSqlIndex( const QString& tablename = QString::null, const QString& name = QString::null );
    QSqlIndex( const QSqlIndex& other );
    ~QSqlIndex();
    QSqlIndex&       operator=( const QSqlIndex& other );
    QString          tableName() const { return table; }
    QSqlFieldList    fields() const;
    uint             count() const;
    void             clear();
    void             append( QSqlField field );
    void             setName( const QString& name );
    QString          name() const;
    QString          toString() const;

private:
    QString          flist;
    QString          table;
    QSqlFieldList    fieldList;
    QString          nm;
};

#endif	// QT_NO_SQL
#endif
