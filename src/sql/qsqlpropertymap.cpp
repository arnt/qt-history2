/****************************************************************************
**
** Definition of QSqlPropertyMap class
**
** Created : 2000-11-20
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsqlpropertymap.h"

#ifndef QT_NO_SQL

#include "qwidget.h"
#include "qcleanuphandler.h"
#include "qobjcoll.h"
#include "qmap.h"

class QSqlPropertyMap::QSqlPropertyMapPrivate
{
public:
    QSqlPropertyMapPrivate() {}
    QMap< QString, QString > propertyMap;
};

/*!
  \class QSqlPropertyMap qsqlpropertymap.h
  \module sql
  \brief The QSqlPropertyMap class is used to map widgets to SQL fields.

  The SQL module uses Qt <a href="properties.html">object properties</a>
  to insert and extract values from editor widgets.

  This class is used to map editors to SQL fields. This works by
  associating SQL editor class names to the properties used to insert
  and extract values to/from the editor.

  For example, a QLineEdit can be used to edit text strings and other
  data types in QDataTables or QSqlForms. Several properties are
  defined in QLineEdit, but only the \e text property is used to
  insert and extract text from a QLineEdit. Both QDataTable and QSqlForm
  use the global QSqlPropertyMap for inserting and extracting values
  to and from an editor widget.  The global property map defines
  several common widgets and properties that are suitable for many
  applications.  You can add and remove widget properties to suit your
  specific needs.

  If you want to use custom editors with your QDataTable or QSqlForm,
  you have to install your own QSqlPropertyMap for that table or form.
  Example:

  \code
  QSqlPropertyMap *myMap  = new QSqlPropertyMap();
  QSqlForm        *myForm = new QSqlForm( this );
  MyEditor	   myEditor( this );

  // Set the QSqlForm's record buffer to the update buffer of
  // a pre-existing QSqlCursor called 'cur'.
  myForm->setRecord( cur->primeUpdate() );

  // Install the customized map
  myMap->insert( "MyEditor", "content" );
  myForm->installPropertyMap( myMap ); // myForm now owns myMap
  ...
  // Insert a field into the form that uses a myEditor to edit the
  // field 'somefield'
  myForm->insert( &myEditor, "somefield" );

  // Update myEditor with the value from the mapped database field
  myForm->readFields();
  ...
  // Let the user edit the form
  ...
  // Update the database fields with the values in the form
  myForm->writeFields();
  ...
  \endcode

  You can also replace the global QSqlPropertyMap that is used by
  default. (Bear in mind that QSqlPropertyMap takes ownership of the new
  default map.)

  \code
  QSqlPropertyMap *myMap = new QSqlPropertyMap;

  myMap->insert( "MyEditor", "content" );
  QSqlPropertyMap::installDefaultMap( myMap );
  ...
  \endcode

  \sa QDataTable, QSqlForm, QSqlEditorFactory
*/

/*!

  Constructs a QSqlPropertyMap.

    The default property mappings used by Qt widgets are:
    <ul>
    <li>QButton -- text
    <li>QCheckBox -- checked
    <li>QComboBox -- currentItem
    <li>QDateEdit -- date
    <li>QDateTimeEdit -- dateTime
    <li>QDial -- value
    <li>QLabel -- text
    <li>QLCDNumber -- value
    <li>QLineEdit -- text
    <li>QListBox -- currentItem
    <li>QMultiLineEdit -- text
    <li>QPushButton -- text
    <li>QRadioButton -- text
    <li>QScrollBar -- value
    <li>QSlider -- value
    <li>QSpinBox -- value
    <li>QTextBrowser -- source
    <li>QTextEdit -- text
    <li>QTextView -- text
    <li>QTimeEdit -- time
    </ul>

 */
QSqlPropertyMap::QSqlPropertyMap()
{
    d = new QSqlPropertyMapPrivate();
    d->propertyMap["QButton"]        = "text";
    d->propertyMap["QCheckBox"]      = "checked";
    d->propertyMap["QComboBox"]      = "currentItem";
    d->propertyMap["QDateEdit"]      = "date";
    d->propertyMap["QDateTimeEdit"]  = "dateTime";
    d->propertyMap["QDial"]          = "value";
    d->propertyMap["QLabel"]         = "text";
    d->propertyMap["QLCDNumber"]     = "value";
    d->propertyMap["QLineEdit"]      = "text";
    d->propertyMap["QListBox"]       = "currentItem";
    d->propertyMap["QMultiLineEdit"] = "text";
    d->propertyMap["QPushButton"]    = "text";
    d->propertyMap["QRadioButton"]   = "text";
    d->propertyMap["QScrollBar"]     = "value";
    d->propertyMap["QSlider"]        = "value";
    d->propertyMap["QSpinBox"]       = "value";
    d->propertyMap["QTextBrowser"]   = "source";
    d->propertyMap["QTextEdit"]      = "text";
    d->propertyMap["QTextView"]      = "text";
    d->propertyMap["QTimeEdit"]      = "time";
}

/*!

  Destroys the QSqlPropertyMap.

  Note that if the QSqlPropertyMap is installed with
  installPropertyMap() the object it was installed into, e.g. the
  QSqlForm, takes ownership and will delete the QSqlPropertyMap when
  necessary.
 */
QSqlPropertyMap::~QSqlPropertyMap()
{
    delete d;
}

/*!

  Returns the \a widget's property as a QVariant.
*/
QVariant QSqlPropertyMap::property( QWidget * widget )
{
    if( !widget ) return QVariant();
#ifdef QT_CHECK_RANGE
    if ( !d->propertyMap.contains( QString(widget->metaObject()->className()) ) )
	qWarning("QSqlPropertyMap::property: %s does not exist", widget->metaObject()->className() );
#endif
    return widget->property( d->propertyMap[ widget->metaObject()->className() ] );
}

/*!

  Sets the \a widget's property to \a value.

*/
void QSqlPropertyMap::setProperty( QWidget * widget, const QVariant & value )
{
    if( !widget ) return;

    widget->setProperty( d->propertyMap[ widget->metaObject()->className() ],
			 value );
}

/*!

  Insert a new classname/property pair, which is used for custom SQL
  field editors. There \e must be a Q_PROPERTY clause in the \a classname
  class declaration for the \a property.

*/
void QSqlPropertyMap::insert( const QString & classname,
			      const QString & property )
{
    d->propertyMap[ classname ] = property;
}

/*!

  Removes \a classname's classname/property pair from the map.

*/
void QSqlPropertyMap::remove( const QString & classname )
{
    d->propertyMap.remove( classname );
}

static QSqlPropertyMap * defaultmap = 0;
static QCleanupHandler< QSqlPropertyMap > qsql_cleanup_property_map;

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

  Replaces the global default property map with \a map. All QDataTable and
  QSqlForm instantiations will use this new map for inserting and
  extracting values to and from editors. <em>QSqlPropertyMap takes
  ownership of \a map, and destroys it when it is no longer needed. </em>
*/
void QSqlPropertyMap::installDefaultMap( QSqlPropertyMap * map )
{
    if( map == 0 ) return;

    if( defaultmap != 0 ){
	qsql_cleanup_property_map.remove( defaultmap );
	delete defaultmap;
    }
    defaultmap = map;
    qsql_cleanup_property_map.add( defaultmap );
}

#endif // QT_NO_SQL
