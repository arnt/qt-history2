#ifndef QSQLINDEX_H
#define QSQLINDEX_H

#ifndef QT_H
#include "qstring.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlIndex
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
    QString          toString( const QString& prefix = QString::null ) const;

private:
    QString          flist;
    QString          table;
    QSqlFieldList    fieldList;
    QString          nm;
};

class Q_EXPORT QSqlRelation
{
public:
    QSqlRelation( const QSqlIndex& parentIndex, const QSqlIndex& childIndex, const QString& name = QString::null );
    virtual ~QSqlRelation();
    QString     name() const { return nm; }
private:
    QSqlIndex pIdx;
    QSqlIndex cIdx;
    QString nm;
};

#endif	// QT_NO_SQL
#endif
