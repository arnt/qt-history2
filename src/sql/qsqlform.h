#ifndef QSQLFORM_H
#define QSQLFORM_H

#ifndef QT_H
#include "qwidget.h"
#include "qmap.h"
#include "qvariant.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlField;
class QSqlRecord;
class QSqlCursor;
class QSqlEditorFactory;

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QMap<QString,QString>;
template class Q_EXPORT QMap<QWidget*, QSqlField*>;
// MOC_SKIP_END
#endif

class Q_EXPORT QSqlPropertyMap {
public:
    QSqlPropertyMap();

    QVariant property( QWidget * widget );
    void     setProperty( QWidget * widget, const QVariant & value );

    void     insert( const QString & classname, const QString & property );
    void     remove( const QString & classname );

    static QSqlPropertyMap * defaultMap();

private:
    QMap< QString, QString > propertyMap;
};

class Q_EXPORT QSqlFormMap
{
public:
    QSqlFormMap();
    ~QSqlFormMap();

    void        insert( QWidget * widget, QSqlField * field );
    void        remove( QWidget * widget );
    void        clear();
    uint        count() const;

    QWidget *   widget( uint i ) const;
    QSqlField * widgetToField( QWidget * widget ) const;
    QWidget *   fieldToWidget( QSqlField * field ) const;

    void        readRecord();
    void        writeRecord();

    void        installPropertyMap( QSqlPropertyMap * map );

private:
    QMap< QWidget *, QSqlField * > map;
    QSqlPropertyMap * m;
};

class Q_EXPORT QSqlForm : public QObject
{
    Q_OBJECT
public:
    QSqlForm( QObject * parent = 0, const char * name = 0 );
    QSqlForm( QWidget * widget, QSqlRecord * fields, uint columns = 1, 
	      QObject * parent = 0, const char * name = 0 );
    ~QSqlForm();

    void associate( QWidget * widget, QSqlField * field );
    void populate( QWidget * widget, QSqlRecord * fields, uint columns = 1 );
    
    void setReadOnly( bool enable );
    bool isReadOnly() const;

    void installEditorFactory( QSqlEditorFactory * f );
    void installPropertyMap( QSqlPropertyMap * m );

public slots:
    void readRecord();
    void writeRecord();
    void clear();
    
private:
    bool readOnly;
    QSqlFormMap map;
    QSqlEditorFactory * factory;
};

#endif // QT_NO_SQL
#endif // QSQLFORM_H
