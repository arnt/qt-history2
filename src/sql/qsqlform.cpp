#include "qcleanuphandler.h"
#include "qobjcoll.h"
#include "qsqldatabase.h"
#include "qsqlfield.h"
#include "qsqlform.h"
#include "qsqlcursor.h"
#include "qsqlresult.h"
#include "qeditorfactory.h"
#include "qlabel.h"
#include "qlayout.h"

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
    propertyMap["QLabel"]       = "pixmap";
}

/*!

  Returns the QVariant which is a property of \a widget.
*/
QVariant QSqlPropertyMap::property( QWidget * widget )
{
    if( !widget ) return QVariant();
#ifdef CHECK_RANGE
    if ( !propertyMap.contains( QString(widget->metaObject()->className()) ) )
	qWarning("QSqlPropertyMap::property: %s does not exist", widget->metaObject()->className() );
#endif
    return widget->property( propertyMap[ widget->metaObject()->className() ] );
}

/*!

  Sets the property associated with \a widget to \a value.
*/
void QSqlPropertyMap::setProperty( QWidget * widget, const QVariant & value )
{
    if( !widget ) return;

    //    qDebug("setting property for " + QString(widget->name()) + " to value of type:" + QString(value.typeName()) + " of value:" + value.toString() );
    widget->setProperty( propertyMap[ widget->metaObject()->className() ],
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

static QSqlPropertyMap * defaultmap = 0;
QCleanUpHandler< QSqlPropertyMap > qsql_cleanup_property_map;

/*!

  Returns the application global QSqlPropertyMap.
*/
QSqlPropertyMap * QSqlPropertyMap::defaultMap()
{
    if( defaultmap == 0 ){
	defaultmap = new QSqlPropertyMap();
	qsql_cleanup_property_map.addCleanUp( defaultmap );
    }
    return defaultmap;
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
    m = 0;
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
  ownership of the \a pmap, and \a pmap is deleted when the object goes
  out of scope.

  \sa installEditorFactory()
*/
void QSqlFormMap::installPropertyMap( QSqlPropertyMap * pmap )
{
    if( m )
	delete m;
    m = pmap;
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

  Clears the values of all fields in the map.
*/
void QSqlFormMap::clear()
{
    QMap< QWidget *, QSqlField * >::Iterator it;
    for( it = map.begin(); it != map.end(); ++it ){
	(*it)->clear();
    }
    readFields();
}

/*!

  Returns the number of widgets in the map.
*/
uint QSqlFormMap::count() const
{
    return map.count();
}

/*!

  Returns the i'th widget in the map to. Useful for traversing the
  map.
*/
QWidget * QSqlFormMap::widget( uint i ) const
{
    QMap< QWidget *, QSqlField * >::ConstIterator it;
    uint cnt = 0;
    
    if( i > map.count() ) return 0;
    for( it = map.begin(); it != map.end(); ++it ){
	if( cnt++ == i )
	    return it.key();
    }
    return 0;
}

/*!

  Returns the widget which field \a field is mapped to.
*/
QWidget * QSqlFormMap::fieldToWidget( QSqlField * field ) const
{
    QMap< QWidget *, QSqlField * >::ConstIterator it;
    for( it = map.begin(); it != map.end(); ++it ){
	if( *it == field )
	    return it.key();
    }
    return 0;
}

/*!

  Returns the field number widget \a widget is mapped to.
*/
QSqlField * QSqlFormMap::widgetToField( QWidget * widget ) const
{
    if( map.contains( widget ) )
	return map[widget];
    else
	return 0;
}

/*!

  Update the widgets in the map with values from the actual database
  fields.
*/
void QSqlFormMap::readFields()
{
    QSqlField * f;
    QMap< QWidget *, QSqlField * >::Iterator it;
    QSqlPropertyMap * pmap = (m == 0) ? QSqlPropertyMap::defaultMap() : m;
        
    for(it = map.begin() ; it != map.end(); ++it ){
	f = widgetToField( it.key() );
	if( !f ) continue;
	pmap->setProperty( it.key(), f->value() );
    }
}

/*!

  Update the actual database fields with values from the widgets.
*/
void QSqlFormMap::writeFields()
{
    QSqlField * f;
    QMap< QWidget *, QSqlField * >::Iterator it;
    QSqlPropertyMap * pmap = (m == 0) ? QSqlPropertyMap::defaultMap() : m;

    for(it = map.begin() ; it != map.end(); ++it ){
	f = widgetToField( it.key() );
	if( !f ) continue;
	f->setValue( pmap->property( it.key() ) );
    }
}

/*!

  \class QSqlForm qsqlform.h
  \brief Class used for creating SQL forms

  \module sql

  This class is used to create SQL forms for accessing, updating,
  inserting and deleting data from a database. Populate the form with
  widgets created by the QSqlEditorFactory class, to get the proper
  widget for a certain field. The form needs a valid QSqlCursor on which
  to perform its operations.
  Some sample code to initialize a form successfully:
  \code
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
  form.readFields();
  \endcode

  If you want to use custom editors for displaying/editing data fields,
  you will have to install a custom QSqlPropertyMap. The form uses this
  object to get or set the value of a widget (ie. the text in a QLineEdit,
  the index in a QComboBox).
  You will also have to use the Q_PROPERTY macro in the class definition,
  and define a pair of functions that can get or set the value of the
  widget.
*/

/*!

  Constructs a SQL form.
*/
QSqlForm::QSqlForm( QObject * parent, const char * name )
    : QObject( parent, name ),
      autodelete( FALSE ),
      readOnly( FALSE ),
      v( 0 ),
      factory( 0 )
{
}

/*!

  Constructs a SQL form. This version of the constructor automatically
  generates a form, were \a widget becomes the parent of the generated
  widgets. The form fields will be spread across \a columns number of
  columns.
  
  \sa populate()
*/
QSqlForm::QSqlForm( QWidget * widget, QSqlCursor * view, uint columns = 1, 
		    QObject * parent, const char * name )
    : QObject( parent, name ),
      autodelete( FALSE ),
      readOnly( FALSE ),
      factory( 0 )
{
    populate( widget, view, columns );
}

/*!
  
  Destructs the form.
*/
QSqlForm::~QSqlForm()
{
    if( autodelete && v )
	delete v;
}

/*!

  This function is used to associate a widget with a database field.
*/
void QSqlForm::associate( QWidget * widget, QSqlField * field )
{
    map.insert( widget, field );
}

/*!

  Set the SQL view that the widgets in the form should be associated
  with. #### QSqlForm takes ownership of the \a view pointer, and it will
  be deleted when a new view is set, or when the object goes out of
  scope. ### crap!
*/
void QSqlForm::setView( QSqlCursor * view )
{
    if( autodelete && v )
	delete v;
    v = view;
}

/*!

  Returns the QSqlCursor that this form is associated with.
*/
QSqlCursor * QSqlForm::view() const
{
    return v;
}

/*!
  
  Installs a custom QEditorFactory. This is used in the populate()
  function to automatically create the widgets in the form.
  
  \sa installPropertyMap(QSqlPropertyMap *), QEditorFactory
 */
void QSqlForm::installEditorFactory( QEditorFactory * f )
{
    if( factory )
	delete factory;
    factory = f;
}

/*!
  
 Installs a custom QSqlPropertyMap. Used together with custom field
 editors. Please note that the QSqlForm class will take ownership of
 the propery map, so don't delete it!
 
 \sa installEditorFactory(QEditorFactory *), QSqlPropertyMap
*/
void QSqlForm::installPropertyMap( QSqlPropertyMap * m )
{
    map.installPropertyMap( m );
}

/*!
  
  Sets the form state.
 */
void QSqlForm::setReadOnly( bool state )
{
    if( map.count() ){
	for( uint i = 0; i < map.count(); i++ ){
	    QWidget * w = map.widget( i );
	    if( w ) w->setEnabled( !state );
	}
	readOnly = state;
    }
}

/*!
  
  Returns the form state.
 */
bool QSqlForm::isReadOnly() const
{
    return readOnly;
}

/*!
  
  Sets the auto-delete option of the form.
  
  Enabling auto-delete (\a enable is TRUE) will delete the view the
  form currently operates on.
  
  Disabling auto-delete (\a enable is FALSE) will <em>not<\em> delete
  the view when the form goes out of scope, or is deleted.
  
  The default setting is FALSE.
  
  \sa autoDelete().
 */
void QSqlForm::setAutoDelete( bool enable )
{
    autodelete = enable;
}
/*!
  
  Returns the setting of the auto-delete option (default is FALSE).
  
  \sa setAutoDelete().
 */
bool QSqlForm::autoDelete() const
{
    return autodelete;
}

/*!

  Refresh the widgets in the form with values from the associated SQL
  fields. Also emits a signal to indicate that the form state has
  changed.
*/
void QSqlForm::readFields()
{
    if( v ){
	map.readFields();
	emit stateChanged( v->at() );
    } else
	qWarning( "QSqlForm: No view associated with this form." );
}

/*!

  Refresh the SQL fields with values from the associated widgets.
*/
void QSqlForm::writeFields()
{
    if( v )
	map.writeFields();
    else
	qWarning( "QSqlForm: No view associated with this form." );
}

/*!

  Clears the form, i.e. all fields are set to be empty.
*/
void QSqlForm::clear()
{
    map.clear();
    readFields();
}

/*!

  Move to the first record in the associated view.
*/
void QSqlForm::first()
{
    if( v && v->first() ){
	readFields();
    }
}

/*!

  Move to the last set record in the associated view.
*/
void QSqlForm::last()
{
    if( v && v->last() ){
	readFields();
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
	readFields();
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
	readFields();
    }
}

/*!

  Insert a new record in the associated view. This function will most
  likely have to be re-implementet by the user.
*/
bool QSqlForm::insert()
{
    if( !readOnly && v ){
	writeFields();
	v->insert();
	return TRUE;
    }
    return FALSE;
}

/*!

  Update the current record in the associated view. This function will
  most likely have to be re-implementet by the user.
*/
bool QSqlForm::update()
{
    if( !readOnly && v ){
	writeFields();
	if( v->update( v->primaryIndex() ) )
	    return TRUE;
    }
    return FALSE;
}

/*!

  Delete the current record from the associated view. This function
  will most likely have to be re-implementet by the user.
*/
bool QSqlForm::del()
{
    if( !readOnly && v && v->del( v->primaryIndex() ) ){
	readFields();
	return TRUE;
    }
    return FALSE;
}

/*!

  Seek to the i'th record in the associated view.
*/
void QSqlForm::seek( uint i )
{
    if( v && v->seek( i ) ){
	readFields();
    }
}

/*!
  
  This is a convenience function used to automatically populate a form
  with fields based on a QSqlCursor. The form will contain a name label
  and an editor widget for each of the fields in the view. The widgets
  are layed out vertically in a QVBoxLayout, across \a columns number
  of columns. \a widget will become the parent of the generated widgets.
 */

void QSqlForm::populate( QWidget * widget, QSqlCursor * view, uint columns )
{
    // ### Remove the children before populating?
    
    if( !widget || !view ) return;
    
    QEditorFactory * f = (factory == 0) ? QEditorFactory::defaultFactory() :
	                                  factory;
    QWidget * editor;
    QLabel * label; 
    QVBoxLayout * vb = new QVBoxLayout( widget );
    QGridLayout * g  = new QGridLayout( vb );
    
    g->setMargin( 5 );
    g->setSpacing( 3 );
    
    QString pi = view->primaryIndex().toString();
    
    if( columns < 1 ) columns = 1;
    int numPerColumn = view->count() / columns;
    
    if( (view->count() % columns) > 0)
	numPerColumn++;
    
    int col = 0, currentCol = 0;
    
    for(uint i = 0; i < view->count(); i++){
	if( col >= numPerColumn ){
	    col = 0;
	    currentCol += 2;
	}
	
	// Do not show primary index fields in the form
	QString name = view->field( i )->name();
	if( name == pi ) continue;
	
	// ### crap - use the field displayLabel() instead!
	name[0] = name[0].upper(); // capitalize the first letter
	label = new QLabel( name, widget );
	
	g->addWidget( label, col, currentCol );
	
	editor = f->createEditor( widget, view->value( i ) );
	g->addWidget( editor, col, currentCol + 1 );
	associate( editor, view->field( i ) );
	col++;
    }
    
    setView( view );
    readFields();
}


#endif // QT_NO_SQL
