#include "qsqlpropertymanager.h"
#include "qmetaobject.h"
#include "qobject.h"

#ifndef QT_NO_SQL

/*!
  \class QSqlPropertyManager qsqlpropertymanager.h
  \brief Class used for mapping SQL editor values to SQL fields and vice versa
  
  \module database

  This class is used to associate a class with a specific property. This
  is used on the GUI side of the database module to map SQL field
  editor data to SQL fields and vice versa.
 */

/*!
  Constructs the manager.
 */
QSqlPropertyManager::QSqlPropertyManager()
{
    propertyMap["QLineEdit"]    = "text";
    propertyMap["QSpinBox"]     = "value";
    propertyMap["QDial"]        = "value";
    propertyMap["QCheckButton"] = "checked";
    propertyMap["QSlider"]      = "value";
    propertyMap["QComboBox"]    = "currentItem";
}

/*!
  
  Returns the QVariant which is a property of \a object.
*/
QVariant QSqlPropertyManager::property( QObject * object )
{
    if( !object ) return QVariant();
    
    return object->property( propertyMap[ object->metaObject()->className() ] );
}

/*!
  
  Sets the property associated with \a object to \a value.
*/
void QSqlPropertyManager::setProperty( QObject * object, const QVariant & value )
{
    if( !object ) return;
    
    object->setProperty( propertyMap[ object->metaObject()->className() ], 
			 value );
}

/*!
  
  Add a new classname/property pair, which is used for custom SQL
  field editors. Remember to add a Q_PROPERTY clause in the \a classname
  class declaration.
*/
void QSqlPropertyManager::addClass( const QString & classname, 
				    const QString & property )
{
    propertyMap[ classname ] = property;
}

#endif // QT_NO_SQL
