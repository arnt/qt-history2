#ifndef QSQLFIELDMAPPER_H
#define QSQLFIELDMAPPER_H

#ifndef QT_H
#include "qmap.h"
#include "qsqlpropertymanager.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSql;

class QSqlFieldMapper
{
public:
    QSqlFieldMapper( QSql * set = 0 );
    
    void      add( QWidget * widget, int field );
    int       whichField( QWidget * widget ) const;
    QWidget * whichWidget( int field ) const;
    void      syncWidgets();
    void      syncFields();
    void      setQuery( QSql * set );

private:
    QMap< QWidget *, int > map;
    QSqlPropertyManager m;
    QSql * s;
};

#endif // QT_NO_SQL
#endif // QSQLFIELDMAPPER_H
