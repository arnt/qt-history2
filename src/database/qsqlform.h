#ifndef QSQLFORM_H
#define QSQLFORM_H

#ifndef QT_H
#include "qwidget.h"
#include "qmap.h"
#include "qsqlpropertymanager.h"
#include "qsqlview.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlField;
class QSqlFormPrivate;

class Q_EXPORT QSqlForm : public QWidget
{
    Q_OBJECT
public:
    QSqlForm( QWidget * parent = 0, const char * name = 0 );
    ~QSqlForm();
    
    void        associate( QWidget * widget, int field );
    int         whichField( QWidget * widget ) const;
    QWidget *   whichWidget( int field ) const;
    
    void        setQuery( const QSql & query );
    void        setRowset( const QSqlRowset & rset );
    void        setView( const QSqlView & view );
   
public slots:
    virtual void refresh();
    virtual void refreshFields();
    virtual void first();    
    virtual void previous();
    virtual void next();
    virtual void last();    
    virtual void insert();
    virtual void update();
    virtual void del();
    virtual void commitAll();
    virtual void rejectAll();
        
protected:

private:
    QSqlFormPrivate * d;
    QMap< QWidget *, int > fieldMap;
    QSqlPropertyManager m;
};

#endif // QT_NO_SQL
#endif // QSQLFORM_H
