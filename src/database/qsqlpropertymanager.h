#ifndef QSQLPROPERTYMANAGER
#define QSQLPROPERTYMANAGER

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#include "qwidget.h"
#include "qmap.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlPropertyManager {
public:
    QSqlPropertyManager();
    
    QVariant property( QObject * object );
    void     setProperty( QObject * object, const QVariant & value );   
    void     addClass( const QString & classname, const QString & property );
    
private:
    QMap< QString, QString > propertyMap;
};

#endif // QT_NO_SQL
#endif // QSQLPROPERTYMANAGER
