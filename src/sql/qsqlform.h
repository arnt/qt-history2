#ifndef QSQLFORM_H
#define QSQLFORM_H

#ifndef QT_H
#include "qwidget.h"
#include "qmap.h"
#include "qsqlview.h"
#include "qsqlfieldmapper.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlPrivate;

class Q_EXPORT QSqlForm : public QWidget
{
    Q_OBJECT
public:
    QSqlForm( QWidget * parent = 0, const char * name = 0 );
    ~QSqlForm();
    
    void associate( QWidget * widget, int field );

    void setQuery( const QSql & query );
    void setRowset( const QSqlRowset & rset );
    void setView( const QSqlView & view );
   
public slots:
    virtual void syncWidgets();
    virtual void syncFields();
    virtual void first();    
    virtual void previous();
    virtual void next();
    virtual void last();    
    virtual void insert();
    virtual void update();
    virtual void del();
    virtual void seek( int i );
        
signals:
    void recordChanged( int i );
    
private:
    QSqlPrivate * d;
    QSqlFieldMapper * fieldMap;
};

#endif // QT_NO_SQL
#endif // QSQLFORM_H
