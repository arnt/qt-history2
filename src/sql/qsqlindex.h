#ifndef QSQLINDEX_H
#define QSQLINDEX_H

#ifndef QT_H
#include "qstring.h"
#include "qvaluelist.h"
#include "qsqlfield.h"
#include "qsqlrecord.h"
#endif // QT_H

#ifndef QT_NO_SQL

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QValueList<bool>;
// MOC_SKIP_END
#endif

class Q_EXPORT QSqlIndex : public QSqlRecord
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

    void             append( const QSqlField& field );
    void             append( const QSqlField& field, bool desc );

    bool             isDescending( int i ) const;
    void             setDescending( int i, bool desc );

    QString          toString( const QString& prefix = QString::null ) const;

private:
    QString          table;
    QString          nm;
    QValueList<bool> sorts;
};

#endif	// QT_NO_SQL
#endif


