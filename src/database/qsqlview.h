#ifndef QSQLVIEW_H
#define QSQLVIEW_H

#ifndef QT_H
#include "qstring.h"
#include "qsqlrowset.h"
#include "qsqlindex.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDatabase;

class QSqlView : public QSqlRowset
{
public:
    QSqlView( QSqlDatabase * db, const QString & name );
    QSqlView( const QSqlView & s );

    QSqlIndex     primaryIndex() const;
    int           insert();
    int           update( const QSqlIndex & filter = QSqlIndex() );
    int           del( const QSqlIndex & filter = QSqlIndex() );
protected:
    int           update( const QString & filter );
    int           del( const QString & filter );
};

/*
class QSqlRelation
{
public:
    QSqlRelation();
    ~QSqlRelation();
    void  link( const QSqlIndex& parentIndex, const QSqlIndex& childIndex );
private:
    QSqlIndex pIdx;
    QSqlIndex cIdx;
}
*/

#endif // QT_NO_SQL
#endif
