#ifndef QSQLFORM_H
#define QSQLFORM_H

#ifndef QT_H
#include "qwidget.h"
#include "qmap.h"
#include "qvariant.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlField;
class QSqlView;

class QSqlPropertyMap {
public:
    QSqlPropertyMap();
    
    QVariant property( QObject * object );
    void     setProperty( QObject * object, const QVariant & value );   
    void     insert( const QString & classname, const QString & property );
    void     remove( const QString & classname );
    
private:
    QMap< QString, QString > propertyMap;
};

class QSqlFormMap
{
public:
    QSqlFormMap();
    ~QSqlFormMap();
    
    void        insert( QWidget * widget, QSqlField * field );
    void        remove( QWidget * widget );
    QSqlField * whichField( QWidget * widget ) const;
    QWidget *   whichWidget( QSqlField * field ) const;
    void        syncWidgets();
    void        syncFields();
    void        installPropertyMap( QSqlPropertyMap * pmap );
    
private:
    QMap< QWidget *, QSqlField * > map;
    QSqlPropertyMap * m;
};

class Q_EXPORT QSqlForm : public QWidget
{
    Q_OBJECT
public:
    QSqlForm( QWidget * parent = 0, const char * name = 0 );
    ~QSqlForm();
    
    void       associate( QWidget * widget, QSqlField * field );
    void       setView( QSqlView * view );
    QSqlView * view() const;
   
public slots:
    virtual void syncWidgets();
    virtual void syncFields();
    
    virtual void first();    
    virtual void previous();
    virtual void next();
    virtual void last();    
    virtual bool insert();
    virtual bool update();
    virtual bool del();
    virtual void seek( int i );
        
signals:
    void recordChanged( int i );
    
private:
    QSqlView * v;
    QSqlFormMap * map;
};

#endif // QT_NO_SQL
#endif // QSQLFORM_H
