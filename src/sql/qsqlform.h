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
class QEditorFactory;

class QSqlPropertyMap {
public:
    QSqlPropertyMap();
    
    QVariant property( QWidget * widget );
    void     setProperty( QWidget * widget, const QVariant & value );   
 
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
    void        clear();
    uint        count() const;
    
    QWidget *   widget( uint i ) const;
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
    QSqlForm( QSqlView * view, uint columns = 1, QWidget * parent = 0,
	      const char * name = 0 );
    ~QSqlForm();
    
    void       associate( QWidget * widget, QSqlField * field );
    
    void       setView( QSqlView * view );
    QSqlView * view() const;
    
    void       setReadOnly( bool state );
    bool       isReadOnly() const;
    
    void       installEditorFactory( QEditorFactory * f );
    void       installPropertyMap( QSqlPropertyMap * m );
    void       populate( QSqlView * view, uint columns = 1 );
    
public slots:
    void syncWidgets();
    void syncFields();   
    void clear();
    
    virtual void first();    
    virtual void previous();
    virtual void next();
    virtual void last();    
    virtual bool insert();
    virtual bool update();
    virtual bool del();
    virtual void seek( uint i );
        
signals:
    void stateChanged( uint i );
    
private:
    bool readOnly;
    QSqlView * v;
    QSqlFormMap * map;
};

#endif // QT_NO_SQL
#endif // QSQLFORM_H
