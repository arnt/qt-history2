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
    fieldMap = new QSqlFieldMapper();
}
/*!
  
  Destructor.
*/
QSqlForm::~QSqlForm()
{
    delete d;
    delete fieldMap;
}

/*!
  
  This function is used to associate a widget with a database field.
*/
void QSqlForm::associate( QWidget * widget, int field )
{
    fieldMap->add( widget, field );
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
    fieldMap->setQuery( s );
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
    fieldMap->setQuery( r );
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
    fieldMap->setQuery( v );
}

/*!
  
  Refresh the widgets in the form with values from the associated SQL
  fields.
*/
void QSqlForm::syncWidgets()
{
    QSql * s = d->sql();
    fieldMap->syncWidgets();
    emit recordChanged( s->at() );
}

/*!
  
  Refresh the SQL fields with values from the associated widgets.
*/
void QSqlForm::syncFields()
{
    fieldMap->syncFields();
}

void QSqlForm::first()
{
    QSql * s = d->sql();
    if( s && s->first() ){
	syncWidgets();
    }
}

void QSqlForm::last()
{
    QSql * s = d->sql();
    if( s && s->last() ){
	syncWidgets();
    }
}

void QSqlForm::next()
{
    QSql * s = d->sql();
    if( s && s->next() ){
	syncWidgets();
    }
}

void QSqlForm::previous()
{
    QSql * s = d->sql();
    if( s && s->previous() ){
	syncWidgets();
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
	syncFields();
	int at = v->at();
	v->update( v->primaryIndex() );
	v->select( v->sort() );
	v->seek( at );
    }
}

void QSqlForm::del()
{
    qDebug("delete(): not implemented!");    
}

void QSqlForm::seek( int i )
{
    QSql * s = d->sql();
    if( s && s->seek(i) ){
	syncWidgets();
    }
}
#endif // QT_NO_SQL
