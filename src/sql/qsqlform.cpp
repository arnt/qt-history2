/****************************************************************************
**
** Implementation of QSqlForm class
**
** Created : 2000-11-03
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

#ifndef QT_NO_SQL

#include "qsqlfield.h"
#include "qsqlform.h"
#include "qsqlpropertymap.h"

/*!

  \class QSqlForm qsqlform.h
  \brief Class used for managing and creating data entry forms

  \module sql

  This class is used to create and manage data entry forms.

  Typical use of a QSqlForm consists of the following steps:

  <ol>
  <li>Create the QSqlForm.
  <li>Create the widgets you want to appear in the form.
  <li>Map each editor widget in the form to the respective QSqlField.
  <li>Use readFields() to update the editor widgets with values from the data fields.
  <li>Display the form and let the user edit values etc.
  <li>Use writeFields() to update the data field values with the values in the editor widgets.
  </ol>
  
  Note that a QSqlForm does \a not access the database directly, but
  most often via QSqlFields which are part of a QSqlCursor. An
  insert()/update() call to QSqlCursor is needed to actually write
  values to the database.
  
  Some sample code to initialize a form successfully:

  \code
  QSqlCursor myCursor( "mytable" );
  QSqlForm myForm;
  QWidget * w;

  // Create an appropriate widget for displaying/editing the field 
  // 'myfield'  in myCursor.
  w = QLineEdit( this );

  // Get the update buffer from myCursor
  QSqlRecord * buf = myCursor.updateBuffer();

  // Associate the newly created widget with the field 'myfield' in myCursor
  myForm.insert( w, buf->field( "myfield" ) );

  // Update the widgets in the form with values from the fields in the form.
  myForm.readFields();

  // show/edit the form etc...

  // When done, write the field values back to myCursor
  myForm.writeFields();
  
  // Update the actual database.
  myCursor.update();

  \endcode

  If you want to use custom editors for displaying/editing data
  fields, you will have to install a custom QSqlPropertyMap. The form
  uses this object to get or set the value of a widget (ie. the text
  in a QLineEdit, the index in a QComboBox).

  \sa installPropertyMap(), QSqlPropertyMap
*/


/*!

  Constructs a QSqlForm.
*/
QSqlForm::QSqlForm( QObject * parent, const char * name )
    : QObject( parent, name ),
      propertyMap( 0 )
{
}

/*!

  Destructor.
*/
QSqlForm::~QSqlForm()
{
    if( propertyMap )
	delete propertyMap;
}

/*!

  Installs a custom QSqlPropertyMap. This is useful if you plan to
  create your own custom editor widgets. <em>QSqlForm takes
  ownership of \a pmap, and \a pmap is therefore deleted when
  QSqlForm goes out of scope.</em>

  \sa installEditorFactory()
*/
void QSqlForm::installPropertyMap( QSqlPropertyMap * pmap )
{
    if( propertyMap )
	delete propertyMap;
    propertyMap = pmap;
}


/*!

  Insert a widget/field pair into the map.
*/
void QSqlForm::insert( QWidget * widget, QSqlField * field )
{
    map[widget] = field;
}

/*!

  Remove a widget/field pair from the map.
*/
void QSqlForm::remove( QWidget * widget )
{
    map.remove( widget );
}

/*!

  Clears the values of all fields in the map.
*/
void QSqlForm::clearValues()
{
    QMap< QWidget *, QSqlField * >::Iterator it;
    for( it = map.begin(); it != map.end(); ++it ){
	(*it)->clear();
    }
    readFields();
}

/*!

  Clears the map of all fields.
*/
void QSqlForm::clear()
{
    QMap< QWidget *, QSqlField * >::Iterator it;
    for( it = map.begin(); it != map.end(); ++it ){
	map.remove( it );
    }
}

/*!

  Returns the number of widgets in the map.
*/
uint QSqlForm::count() const
{
    return map.count();
}

/*!

  Returns the i'th widget in the map to. Useful for traversing the
  map.
*/
QWidget * QSqlForm::widget( uint i ) const
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
QWidget * QSqlForm::fieldToWidget( QSqlField * field ) const
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
QSqlField * QSqlForm::widgetToField( QWidget * widget ) const
{
    if( map.contains( widget ) )
	return map[widget];
    else
	return 0;
}

/*!

  Update the widgets in the map with values from the associated fields.

*/
void QSqlForm::readFields()
{
    QSqlField * f;
    QMap< QWidget *, QSqlField * >::Iterator it;
    QSqlPropertyMap * pmap = (propertyMap == 0) ? 
			     QSqlPropertyMap::defaultMap() : propertyMap;

    for(it = map.begin() ; it != map.end(); ++it ){
	f = widgetToField( it.key() );
	if( !f ) continue;
	pmap->setProperty( it.key(), f->value() );
    }
}

/*!

  Update the actual database fields with values from the widgets.
*/
void QSqlForm::writeFields()
{
    QSqlField * f;
    QMap< QWidget *, QSqlField * >::Iterator it;
    QSqlPropertyMap * pmap = (propertyMap == 0) ? 
			     QSqlPropertyMap::defaultMap() : propertyMap;

    for(it = map.begin() ; it != map.end(); ++it ){
	f = widgetToField( it.key() );
	if( !f ) continue;
	f->setValue( pmap->property( it.key() ) );
    }
}

/*!

  Update the widget \a widget with the value from the mapped database field.
  Nothing happens if no database field is mapped to \a widget.

*/
void QSqlForm::readField( QWidget * widget )
{
    QSqlField * field = 0;
    QSqlPropertyMap * pmap = (propertyMap == 0) ? 
			     QSqlPropertyMap::defaultMap() : propertyMap;
    
    field = widgetToField( widget );
    if( field )
        pmap->setProperty( widget, field->value() );
}

/*!

  Update the database field mapped to \a widget with the value from \a widget.
  Nothing happens if no database field is mapped to \a widget.
*/
void QSqlForm::writeField( QWidget * widget )
{
    QSqlField * field = 0;
    QSqlPropertyMap * pmap = (propertyMap == 0) ? 
			     QSqlPropertyMap::defaultMap() : propertyMap;

    field = widgetToField( widget );
    if( field ) 
	field->setValue( pmap->property( widget ) );
}

#endif // QT_NO_SQL
