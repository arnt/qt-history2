#include "qobjcoll.h"
#include "qsql_p.h"
#include "qsqlform.h"

#ifndef QT_NO_SQL

/*!
  
  \class QSqlForm qsqlform.h
  \brief Class used for creating SQL forms
  
  \module database
  
  This class can be used to create SQL form widgets for accessing,
  updating, inserting and deleting data from a database easily.  
*/

/*!
  
  Constructs a SQL form.
*/
QSqlForm::QSqlForm( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    d = new QSqlPrivate();
}
/*!
  
  Destructor.
*/
QSqlForm::~QSqlForm()
{
    delete d;
}

/*!
  
  This function is used to associate a widget with a database field.
*/
void QSqlForm::associate( QWidget * widget, int field )
{
    fieldMap[ widget ] = field;
}

/*!
  
  Returns index of the field associated with \a widget. -1 is returned
  if no field is associated with \a widget.
*/
int QSqlForm::whichField( QWidget * widget ) const
{
    if( fieldMap.contains( widget ) )
	return fieldMap[ widget ];
    else
	return -1;
}


/*!
  
  Returns a pointer to the QWidget associated with \a field. 0 is
  returned if no widget is associated with \a field.
*/
QWidget * QSqlForm::whichWidget( int field ) const
{
    QMap< QWidget *, int >::ConstIterator it;
    for( it = fieldMap.begin(); it != fieldMap.end(); ++it ){
	if( *it == field )
	    return it.key();
    }

    return 0;
}

/*!
  
  Set the SQL query that the widgets in the form should be associated
  with.
*/
void QSqlForm::setQuery( const QSql & query )
{
    d->resetMode( QSqlPrivate::Sql );
    QSql * s = d->sql();
    (*s) = query;
}

/*!
  
  Set the SQL rowset that the widgets in the form should be associated
  with.
*/
void QSqlForm::setRowset( const QSqlRowset & rset )
{
    d->resetMode( QSqlPrivate::Rowset );
    QSqlRowset * r = d->rowset();
    (*r) = rset;
}

/*!
  
  Set the SQL view that the widgets in the form should be associated
  with.
*/
void QSqlForm::setView( const QSqlView & view )
{
    d->resetMode( QSqlPrivate::View );
    QSqlView * v = d->view();
    (*v) = view;
}

/*!
  
  Refresh the properties of the widgets in the form that are associated
  with SQL fields.
*/
void QSqlForm::refresh()
{
    QSql * s = d->sql();
    QObjectListIt it( *children() );
    QObject * obj;
    QSqlField f;
    
    if( !s ) return;
 
    for( ; it.current(); ++it ){
	obj = it.current();
	int i = whichField( (QWidget *) obj );
	if( i < 0 ) continue;
	f = s->fields().field( i );
	if( obj->isWidgetType() ){
	    m.setProperty( obj, f.value() );
	}
    }   
}

void QSqlForm::refreshFields()
{
    QSqlRowset * s = d->rowset();
    QObjectListIt it( *children() );
    QObject * obj;
    
    if( !s ) return;
 
    for( ; it.current(); ++it ){
	obj = it.current();
	int i = whichField( (QWidget *) obj );
	if( i < 0 ) continue;
	if( obj->isWidgetType() ){
	    s->setValue( i, m.property( obj ) );
	}
    }
    
}

void QSqlForm::first()
{
    QSql * s = d->sql();
    if( s && s->first() ){
	refresh();
    }
}

void QSqlForm::last()
{
    QSql * s = d->sql();
    if( s && s->last() ){
	refresh();
    }
}

void QSqlForm::next()
{
    QSql * s = d->sql();
    if( s && s->next() ){
	refresh();
    }
}

void QSqlForm::previous()
{
    QSql * s = d->sql();
    if( s && s->previous() ){
	refresh();
    }
}

void QSqlForm::insert()
{
    qDebug("insert(): not implemented!"); 
}

void QSqlForm::update()
{
    QSqlView * v = d->view();
    
    if( v ){
	refreshFields();
	v->update( v->primaryIndex() );
	v->select( v->sort() );
    }
}

void QSqlForm::del()
{
    qDebug("delete(): not implemented!");    
}

void QSqlForm::commitAll()
{
    qDebug("commitAll(): not implemented!");
}

void QSqlForm::rejectAll()
{
    qDebug("rejectAll(): not implemented!");
}

#endif // QT_NO_SQL
