#include "qcleanuphandler.h"
#include "qobjcoll.h"
#include "qsqldatabase.h"
#include "qsqlfield.h"
#include "qsqlform.h"
#include "qsqlrecord.h"
#include "qsqlcursor.h"
#include "qsqlresult.h"
#include "qsqleditorfactory.h"
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
    propertyMap["QDateTimeEdit"]= "dateTime";
    propertyMap["QLabel"]       = "pixmap";  // ### uh....? should be 'text'.
}

/*!

  Returns thw property of \a widget as a QVariant.
  
*/
QVariant QSqlPropertyMap::property( QWidget * widget )
{
    if( !widget ) return QVariant();
#ifdef QT_CHECK_RANGE
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
  field editors. There ust be a Q_PROPERTY clause in the \a classname
  class declaration for the \a property.
  
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
QCleanupHandler< QSqlPropertyMap > qsql_cleanup_property_map;

/*!

  Returns the application global QSqlPropertyMap.
*/
QSqlPropertyMap * QSqlPropertyMap::defaultMap()
{
    if( defaultmap == 0 ){
	defaultmap = new QSqlPropertyMap();
	qsql_cleanup_property_map.add( defaultmap );
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
    readRecord();
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

  Update the widgets in the map with values from the associated fields.
  
*/
void QSqlFormMap::readRecord()
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
void QSqlFormMap::writeRecord()
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

  // Set the cursor the form should operate on
  form.setCursor( &myCursor );

  // Create an appropriate widget for displaying/editing
  // field 0 in myCursor.
  w = factory.createEditor( &form, myCursor.field( 0 ) );

  // Get the insert buffer from the cursor
  QSqlRecord* buf = myCursor.insertBuffer();
  
  // Associate the newly created widget with field 0 in myCursor
  form.associate( w, buf->field( 0 ) );

  // Now, update the contents of the form from the fields in the form.
  form.readRecord();
  
  // edit/save record, etc...
  
  \endcode

  If you want to use custom editors for displaying/editing data
  fields, you will have to install a custom QSqlPropertyMap. The form
  uses this object to get or set the value of a widget (ie. the text
  in a QLineEdit, the index in a QComboBox).
  
  \sa installPropertyMap()
*/

/*!

  Constructs an empty form.
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

  This constructor automatically generates a form, were \a widget
  becomes the parent of the generated widgets. The form fields will be
  spread across \a columns number of columns.

  \sa populate()
*/
QSqlForm::QSqlForm( QWidget * widget, QSqlCursor * cursor, QSqlRecord* fields, 
		    uint columns, QObject * parent, const char * name )
    : QObject( parent, name ),
      autodelete( FALSE ),
      readOnly( FALSE ),
      factory( 0 )
{
    populate( widget, cursor, fields, columns );
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

  Associates a widget with a database field.
  
*/
void QSqlForm::associate( QWidget * widget, QSqlField * field )
{
    map.insert( widget, field );
}

/*!

  Set the cursor that the widgets in the form should be associated
  with.  QSqlForm takes ownership of the \a cursor pointer, and it will
  be deleted when a new cursor is set, or when the object goes out of
  scope. ### crap!
  
*/
void QSqlForm::setCursor( QSqlCursor * cursor )
{
    if( autodelete && v )
	delete v;
    v = cursor;
}

/*!

  Returns a pointer to the QSqlCursor that this form is associated
  with.
  
*/
QSqlCursor * QSqlForm::cursor() const
{
    return v;
}

/*!

  Installs a custom QSqlEditorFactory. This is used in the populate()
  function to automatically create the widgets in the form.

  \sa installPropertyMap(QSqlPropertyMap *), QSqlEditorFactory
 */
void QSqlForm::installEditorFactory( QSqlEditorFactory * f )
{
    if( factory )
	delete factory;
    factory = f;
}

/*!

 Installs a custom QSqlPropertyMap. Used together with custom field
 editors. Please note that the QSqlForm class will take ownership of
 the propery map, so don't delete it!

 \sa installEditorFactory(), QSqlPropertyMap
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

  Enabling auto-delete (\a enable is TRUE) will case the form to take
  ownership of the cursor (see setCursor() ).

  Disabling auto-delete (\a enable is FALSE) will \e not delete the
  cursor when the form goes out of scope.

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
  cursor. Also emits the stateChanged() signal to indicate that the
  form state has changed.
  
*/
void QSqlForm::readRecord()
{
    if( v ){
	map.readRecord();
	emit stateChanged( v->at() );
    } else
	qWarning( "QSqlForm::readRecord: no associated cursor" );
}

/*!

  Refresh the SQL cursor with values from the associated widgets.
*/
void QSqlForm::writeRecord()
{
    if( v )
	map.writeRecord();
    else
	qWarning( "QSqlForm::writeRecord: no associated cursor" );
}

/*!

  Clears the form, i.e. all fields are set to be empty.
*/
void QSqlForm::clear()
{
    map.clear();
    readRecord();
}

/*!

  Move to the first record in the associated cursor.
*/
void QSqlForm::first()
{
    if( v && v->first() ){
	readRecord();
    }
}

/*!

  Move to the last set record in the associated cursor.
*/
void QSqlForm::last()
{
    if( v && v->last() ){
	readRecord();
    }
}

/*!

  Move to the next record in the associated cursor.
*/
void QSqlForm::next()
{

    if( v ){
	v->next();
	if( v->at() == QSqlResult::AfterLast ){
	    v->last();
	}
	readRecord();
    }
}

/*!

  Move to the previous record in the associated cursor.
*/
void QSqlForm::prev()
{
    if( v ){
        v->prev();
	if( v->at() == QSqlResult::BeforeFirst ){
	    v->first();
	}
	readRecord();
    }
}

/*!

  Insert a new record in the associated cursor. This function will most
  likely have to be re-implementet by the user.
*/
bool QSqlForm::insert()
{
    if( !readOnly && v ){
	writeRecord();
	v->insert();
	return TRUE;
    }
    return FALSE;
}

/*!

  Update the current record in the associated cursor. This function will
  most likely have to be re-implementet by the user.
*/
bool QSqlForm::update()
{
    if( !readOnly && v ){
	writeRecord();
	if( v->update() )
	    return TRUE;
    }
    return FALSE;
}

/*!

  Delete the current record from the associated cursor. This function
  will most likely have to be re-implementet by the user.
*/
bool QSqlForm::del()
{
    if( !readOnly && v && v->del() ){
	readRecord();
	return TRUE;
    }
    return FALSE;
}

/*!

  Seek to the i'th record in the associated cursor.
*/
void QSqlForm::seek( uint i )
{
    if( v && v->seek( i ) ){
	readRecord();
    }
}

/*!

  This is a convenience function used to automatically populate a form
  with fields based on a QSqlCursor. The form will contain a name label
  and an editor widget for each of the fields in the cursor. The widgets
  are layed out vertically in a QVBoxLayout, across \a columns number
  of columns. \a widget will become the parent of the generated widgets.
 */

void QSqlForm::populate( QWidget * widget, QSqlCursor * cursor, QSqlRecord* fields, uint columns )
{
    // ### Remove the children before populating?

    if( !widget || !cursor || !fields ) return;

    QSqlEditorFactory * f = (factory == 0) ? 
			    QSqlEditorFactory::defaultFactory() : factory;
    QWidget * editor;
    QLabel * label;
    QVBoxLayout * vb = new QVBoxLayout( widget );
    QGridLayout * g  = new QGridLayout( vb );

    g->setMargin( 5 );
    g->setSpacing( 3 );

    int visibleFields = 0;
    for( uint i = 0; i < fields->count(); i ++ ){
	if( fields->field( i )->isVisible() )
	    visibleFields++;
    }

    if( columns < 1 ) columns = 1;
    int numPerColumn = visibleFields / columns;

    if( (visibleFields % columns) > 0)
	numPerColumn++;

    int col = 0, currentCol = 0;

    for( uint j = 0; j < fields->count(); j++ ){
	if( col >= numPerColumn ){
	    col = 0;
	    currentCol += 2;
	}

	if( fields->field( j )->isPrimaryIndex() || !fields->field( j )->isVisible() )
	    continue;

	label = new QLabel( fields->field( j )->displayLabel(), widget );
	g->addWidget( label, col, currentCol );

	editor = f->createEditor( widget, fields->value( j ) );
	g->addWidget( editor, col, currentCol + 1 );
	associate( editor, fields->field( j ) );
	col++;
    }

    setCursor( cursor );
    readRecord();
}
#endif // QT_NO_SQL
