#ifndef QSQLVIEW_H
#define QSQLVIEW_H

#ifndef QT_H
#include "qstring.h"
#include "qsqlrowset.h"
#include "qsqlindex.h"
#include "qsqlconnection.h"
#endif // QT_H

#ifndef QT_NO_SQL

class Q_EXPORT QSqlDatabase;

class QSqlView : public QSqlRowset
{
public:
    QSqlView( const QString & name = QString::null, const QString& databaseName = QSqlConnection::defaultDatabase );
    QSqlView( const QSqlView & s );
    QSqlView& operator=( const QSqlView& s );
    ~QSqlView();

    QSqlIndex     primaryIndex() const;
    virtual int   insert( bool invalidate = TRUE );
    virtual int   update( const QSqlIndex & filter = QSqlIndex(), bool invalidate = TRUE );
    virtual int   del( const QSqlIndex & filter = QSqlIndex(), bool invalidate = TRUE );
protected:
    virtual int   update( const QString & filter, bool invalidate = TRUE );
    virtual int   del( const QString & filter, bool invalidate = TRUE );
private:
    int           apply( const QString& q, bool invalidate );
};

#endif // QT_NO_SQL
#endif
