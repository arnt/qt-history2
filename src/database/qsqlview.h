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
    int           update( const QString & filter );
    int           update( const QSqlIndex & filter = QSqlIndex() );
    int           del( const QString & filter );
    int           del( const QSqlIndex & filter = QSqlIndex() );
};

#endif // QT_NO_SQL
#endif
