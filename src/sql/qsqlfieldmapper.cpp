#include "qsqlfieldmapper.h"
#include "qsqlview.h"

#ifndef QT_NO_SQL

/*!
  
  Constructs a SQL field mapper.
*/

QSqlFieldMapper::QSqlFieldMapper( QSql * set = 0 )
    : s( set )
{
}

/*!

  Add a widget/field pair to the current map.
*/
void QSqlFieldMapper::add( QWidget * widget, int field )
{
    map[widget] = field;
}
    
/*!

  Returns the field number widget \a widget is mapped to.
*/
int QSqlFieldMapper::whichField( QWidget * widget ) const 
{
    if( map.contains( widget ) )
	return map[widget];
    else
	return -1;
}

/*!

  Returns the widget which field \a field is mapped to.
*/
QWidget * QSqlFieldMapper::whichWidget( int field ) const
{
    QMap< QWidget *, int >::ConstIterator it;
    for( it = map.begin(); it != map.end(); ++it ){
	if( *it == field )
	    return it.key();
    }
    return 0;
}

/*!
  
  Update the widgets in the map with values from the actual database
  fields.
*/
void QSqlFieldMapper::syncWidgets()
{
    QObject * obj;
    QSqlField f;
    QMap< QWidget *, int >::Iterator it;
	
    if( !s ) return;
 
    for(it = map.begin() ; it != map.end(); ++it ){
	obj = it.key();
	int i = whichField( (QWidget *) obj );
	if( i < 0 ) continue;
	f = s->fields().field( i );
	if( obj->isWidgetType() ){
	    m.setProperty( obj, f.value() );
	}
    }       
}

/*!
  
  Update the actual database fields with values from the widgets.
*/  
void QSqlFieldMapper::syncFields()
{
    QSqlRowset * r = (QSqlRowset *) s;
    QObject * obj;
    QMap< QWidget *, int >::Iterator it;
	
    if( !s ) return;
 
    for(it = map.begin() ; it != map.end(); ++it ){
	obj = it.key();
	int i = whichField( (QWidget *) obj );
	if( i < 0 ) continue;
	if( obj->isWidgetType() ){
	    r->setValue( i, m.property( obj ) );
	}
    }       
}

/*!

  Set the query which this fieldmapper is associated with.
*/
void QSqlFieldMapper::setQuery( QSql * set )
{ 
    s = set; 
}
#endif // QT_NO_SQL
