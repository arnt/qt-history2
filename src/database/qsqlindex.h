#ifndef QSQLINDEX_H
#define QSQLINDEX_H

#ifndef QT_H
#include "qstring.h"
#include "qstringlist.h"
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

    void             append( QSqlField field );
    void             setName( const QString& name );
    QString          name() const;
private:
    QString          table;
    QSqlFieldList    fieldList;
    QString          nm;
};

#endif	// QT_NO_SQL
#endif
