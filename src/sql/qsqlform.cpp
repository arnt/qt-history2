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
  \module sql
  \brief A class used for mapping editor values to database fields in 
  QSqlForm and QSqlTable

  This class is used to map SQL editor class names to the properties
  used to insert and extract values into and from the editor.
  
  For instance, a QLineEdit is used to edit text strings and other
  data types in QSqlTable and QSqlForm. QLineEdit defines several
  properties, but it is only the "text" property that is used to
  insert and extract text into and from the QLineEdit. When QSqlTable
  wants to edit a field which uses a QLineEdit as its editor, a
  quick look in this map tells the table to use the "text" property to
  set/get values from the editor.
  
  If you want to use custom editors with your QSqlTable or QSqlForm,
  you have to install your own QSqlPropertyMap for that table/form.
  Example:
  
  \code
  MyEditorFactory myFactory;
  QSqlCursor cursor( "mytable" );
  QSqlForm form;
  QSqlPropertyMap myMap;
  
  myMap.insert( "MySuperEditor", "content" );
  form.installPropertyMap( &map );
  form.installEditorFactory( &myFactory );

  // Generate form, which uses MySuperEditor for special fields
  form.populate( myWidget, cursor, cursor->updateBuffer() );
  \endcode
  
  \sa QSqlTable, QSqlForm, QSqlEditorFactory
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
    propertyMap["QLabel"]       = "text";
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
  \brief Class used for mapping database fields to widgets and vice versa
  \module sql

  This class is used by the QSqlForm to manage the mapping between SQL
  data fields and actual widgets.
  
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
  \brief Class used for managing and creating data entry forms

  \module sql

  This class is used to create and manage data entry forms. 
  
  Populate the form with widgets created by the QSqlEditorFactory
  class to get the proper widget for a certain data field. Use the
  populate() function generate a form automatically. The generated
  form contains a label and an editor for each field in the
  QSqlRecord.
  
  The form needs a valid QSqlCursor on which to perform its operations
  like insert, update and delete.
  
  Some sample code to initialize a form successfully:
  
  \code
  QSqlForm form;
  QSqlEditorFactory * factory = QSqlEditorFactory::defaultFactory();
  QWidget * w;

  // Set the cursor the form should operate on
  form.setCursor( &myCursor );

  // Create an appropriate widget for displaying/editing
  // field 0 in myCursor.
  w = factory->createEditor( &form, myCursor.field( 0 ) );

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
  
  \sa installPropertyMap(), QSqlPropertyMap
*/

/*!

  Constructs an empty form.
*/
QSqlForm::QSqlForm( QObject * parent, const char * name )
    : QObject( parent, name ),
      readOnly( FALSE ),
      factory( 0 )
{
}

/*!

  This constructor automatically generates a form, were \a widget
  becomes the parent of the generated widgets. The form fields will be
  spread across \a columns number of columns.

  \sa populate()
*/
QSqlForm::QSqlForm( QWidget * widget, QSqlRecord * fields, uint columns, 
		    QObject * parent, const char * name )
    : QObject( parent, name ),
      readOnly( FALSE ),
      factory( 0 )
{
    populate( widget, fields, columns );
}

/*!

  Destructs the form.
*/
QSqlForm::~QSqlForm()
{
}

/*!

  Associates a widget with a database field.
  
*/
void QSqlForm::associate( QWidget * widget, QSqlField * field )
{
    map.insert( widget, field );
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

  Refresh the widgets in the form with values from the associated SQL
  cursor. Also emits the stateChanged() signal to indicate that the
  form state has changed.
  
*/
void QSqlForm::readRecord()
{
    map.readRecord();
}

/*!

  Refresh the SQL cursor with values from the associated widgets.
*/
void QSqlForm::writeRecord()
{
    map.writeRecord();
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

  This is a convenience function used to automatically populate a form
  with fields based on a QSqlCursor. The form will contain a name label
  and an editor widget for each of the fields in the cursor. The widgets
  are layed out vertically in a QVBoxLayout, across \a columns number
  of columns. \a widget will become the parent of the generated widgets.
 */

void QSqlForm::populate( QWidget * widget, QSqlRecord* fields, uint columns )
{
    // ### Remove the children before populating?

    if( !widget || !fields ) return;

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

	if( fields->field( j )->isPrimaryIndex() || 
	    !fields->field( j )->isVisible() )
	{
	    continue;
	}

	label = new QLabel( fields->field( j )->displayLabel(), widget );
	g->addWidget( label, col, currentCol );

	editor = f->createEditor( widget, fields->value( j ) );
	g->addWidget( editor, col, currentCol + 1 );
	associate( editor, fields->field( j ) );
	col++;
    }

    readRecord();
}
#endif // QT_NO_SQL
