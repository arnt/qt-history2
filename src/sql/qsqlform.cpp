#include "qobjcoll.h"
#include "qsqldatabase.h"
#include "qsqlfield.h"
#include "qsqlform.h"
#include "qsqlview.h"
#include "qsqlresult.h"

#ifndef QT_NO_SQL

/*!
  \class QSqlPropertyMap qsqlform.h
  \brief Class used for mapping SQL editor values to SQL fields and vice versa

  \module sql

  This class is used to associate a class with a specific property. This
  is used on the GUI side of the database module to map SQL field
  editor data to SQL fields and vice versa.
 */

/*!
  Constructs a QSqlPropertyMap.
 */
QSqlPropertyMap::QSqlPropertyMap()
{
    propertyMap["QLineEdit"]    = "text";
    propertyMap["QSpinBox"]     = "value";
    propertyMap["QDial"]        = "value";
    propertyMap["QCheckButton"] = "checked";
    propertyMap["QSlider"]      = "value";
    propertyMap["QComboBox"]    = "currentItem";
    propertyMap["QDateEdit"]    = "date";
    propertyMap["QTimeEdit"]    = "time";
}

/*!

  Returns the QVariant which is a property of \a object.
*/
QVariant QSqlPropertyMap::property( QObject * object )
{
    if( !object ) return QVariant();

    return object->property( propertyMap[ object->metaObject()->className() ] );
}

/*!

  Sets the property associated with \a object to \a value.
*/
void QSqlPropertyMap::setProperty( QObject * object, const QVariant & value )
{
    if( !object ) return;

    qDebug("setting property for " + QString(object->name()) + " to value of type:" + QString(value.typeName()) + " of value:" + value.toString() );
    object->setProperty( propertyMap[ object->metaObject()->className() ],
			 value );
}

/*!

  Insert a new classname/property pair, which is used for custom SQL
  field editors. Remember to add a Q_PROPERTY clause in the \a classname
  class declaration.
*/
void QSqlPropertyMap::insert( const QString & classname,
			      const QString & property )
{
    propertyMap[ classname ] = property;
}

/*!

  Removes a classname/property pair from the map.
*/
void QSqlPropertyMap::remove( const QString & classname )
{
    propertyMap.remove( classname );
}


/*!
  \class QSqlFormMap qsqlform.h
  \brief Class used for mapping SQL fields to Qt widgets and vice versa

  \module sql

  This class is used to associate a class with a specific property. This
  is used on the GUI side of the database module to map SQL fields
  to Qt widgets and vice versa.
 */

/*!

  Constructs a QSqlFormMap.
*/
QSqlFormMap::QSqlFormMap()
{
    m = new QSqlPropertyMap;
}

/*!

  Destructor.
*/
QSqlFormMap::~QSqlFormMap()
{
    if( m )
	delete m;
}

/*!

  Installs a custom QSqlPropertyMap. This is useful if you plan to
  create your own custom editor widgets. NB! QSqlFormMap takes
  possession of the \a map, and \a map is deleted when the object goes
  out of scope.
*/
void QSqlFormMap::installPropertyMap( QSqlPropertyMap * pmap )
{
    if( m )
	delete m;

    if( pmap )
	m = pmap;
    else
	m = new QSqlPropertyMap;
}


/*!

  Insert a widget/field pair into the map.
*/
void QSqlFormMap::insert( QWidget * widget, QSqlField * field )
{
    map[widget] = field;
}

/*!

  Remove a widget/field pair from the map.
*/
void QSqlFormMap::remove( QWidget * widget )
{
    map.remove( widget );
}


/*!

  Returns the field number widget \a widget is mapped to.
*/
QSqlField * QSqlFormMap::whichField( QWidget * widget ) const
{
    if( map.contains( widget ) )
	return map[widget];
    else
	return 0;
}

/*!

  Returns the widget which field \a field is mapped to.
*/
QWidget * QSqlFormMap::whichWidget( QSqlField * field ) const
{
    QMap< QWidget *, QSqlField * >::ConstIterator it;
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
void QSqlFormMap::syncWidgets()
{
    QSqlField * f;
    QMap< QWidget *, QSqlField * >::Iterator it;

    for(it = map.begin() ; it != map.end(); ++it ){
	f = whichField( it.key() );
	if( !f ) continue;
	m->setProperty( it.key(), f->value() );
    }
}

/*!

  Update the actual database fields with values from the widgets.
*/
void QSqlFormMap::syncFields()
{
    QSqlField * f;
    QMap< QWidget *, QSqlField * >::Iterator it;

    for(it = map.begin() ; it != map.end(); ++it ){
	f = whichField( it.key() );
	if( !f ) continue;
	f->setValue( m->property( it.key() ) );
    }
}

/*!

  \class QSqlForm qsqlform.h
  \brief Class used for creating SQL forms

  \module sql
  
  This class is used to create SQL forms for accessing, updating,
  inserting and deleting data from a database. Populate the form with
  widgets created by the QSqlEditorFactory class, to get the proper
  widget for a certain field. The form needs a valid QSqlView on which
  to perform its operations.
  Some sample code to initialize a form successfully:
  <code>
     QSqlForm form;
     QSqlEditorFactory factory;
     QWidget * w;

     // Set the view the form should operate on
     form.setView( &myView );
     
     // Create an appropriate widget for displaying/editing 
     // field 0 in myView.
     w = factory.createEditor( &form, myView.field( 0 ) );
     
     // Associate the newly created widget with field 0 in myView
     form.associate( w, myView.field( 0 ) );
     
     // Now, update the contents of the form from the fields in the form.
     form.syncWidgets();
  <\code>
  
  If you want to use custom editors for displaying/editing data fields,
  you will have to install a custom QSqlPropertyMap. The form uses this
  object to get or set the value of a widget (ie. the text in a QLineEdit,
  the index in a QComboBox).
  You will also have use the Q_PROPERTY macro in the class definition,
  and define a pair of functions that can get or set the value of the 
  widget.
*/

/*!

  Constructs a SQL form.
*/
QSqlForm::QSqlForm( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    v = 0;
    map = new QSqlFormMap();
}
/*!
  Destructs the form.
*/
QSqlForm::~QSqlForm()
{
    delete map;
}

/*!

  This function is used to associate a widget with a database field.
*/
void QSqlForm::associate( QWidget * widget, QSqlField * field )
{
    map->insert( widget, field );
}

/*!

  Set the SQL view that the widgets in the form should be associated
  with. <em> Do not delete the \a view until the QSqlForm goes out of
  scope.<\em>
*/
void QSqlForm::setView( QSqlView * view )
{
    v = view;
}

/*!

  Returns the QSqlView that this form is associated with.
*/
QSqlView * QSqlForm::view() const
{
    return v;
}

/*!
 Installs a custom QSqlPropertyMap. Used together with custom
 field editors. Please note that the QSqlForm class will
 take ownership of the propery map, so don't delete it!
*/
void QSqlForm::installPropertyMap( QSqlPropertyMap * m )
{
    map->installPropertyMap( m );
}

/*!
  
  Refresh the widgets in the form with values from the associated SQL
  fields. Also emits a signal to indicate that the form state has
  changed.
*/
void QSqlForm::syncWidgets()
{
    if( v ){
	map->syncWidgets();
	emit stateChanged( v->at() );
    } else
	qWarning( "QSqlForm: No view associated with this form." );
}

/*!

  Refresh the SQL fields with values from the associated widgets.
*/
void QSqlForm::syncFields()
{
    if( v )
	map->syncFields();
    else
	qWarning( "QSqlForm: No view associated with this form." );
}

/*!
 
  Move to the first record in the associated view.
*/
void QSqlForm::first()
{
    if( v && v->first() ){
	syncWidgets();
    }
}

/*!
 
  Move to the last set record in the associated view.
*/
void QSqlForm::last()
{
    if( v && v->last() ){
	syncWidgets();
    }
}

/*!
 
  Move to the next record in the associated view.
*/
void QSqlForm::next()
{

    if( v ){
	v->next();
	if( v->at() == QSqlResult::AfterLast ){
	    v->last();
	}
	syncWidgets();
    }
}

/*!
 
  Move to the previous record in the associated view.
*/
void QSqlForm::previous()
{
    if( v ){
        v->previous();
	if( v->at() == QSqlResult::BeforeFirst ){
	    v->first();
	}
	syncWidgets();
    }
}

/*!
 
  Insert a new record in the associated view.
*/
bool QSqlForm::insert()
{
    if( v ){
	syncFields();
	v->insert();
	return TRUE;
    }
    return FALSE;
}

/*!
 
  Update the current record in the associated view.
*/
bool QSqlForm::update()
{
    if( v ){
	syncFields();
	if( v->update( v->primaryIndex() ) )
	    return TRUE;
    }
    return FALSE;
}

/*!
 
  Delete the current record from the associated view.
*/
bool QSqlForm::del()
{
    if( v && v->del( v->primaryIndex() ) ){
	syncWidgets();
	return TRUE;
    }
    return FALSE;
}

/*!
 
  Seek to the i'th record in the associated view.
*/
void QSqlForm::seek( int i )
{
    if( v && v->seek( i ) ){
	syncWidgets();
    }
}
#endif // QT_NO_SQL
