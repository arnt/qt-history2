#include "qsqleditorfactory.h"

#ifndef QT_NO_SQL
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>


QSqlEditorFactory::QSqlEditorFactory()
{
    
}

/*!
  Creates and returns the appropriate editor for the QSqlField.
*/
QWidget * QSqlEditorFactory::createEditor( const QSqlField & field )
{
}


/*!
  Returns a pointer to the single existing instance of a 
  QSqlEditorFactory.
*/
QSqlEditorFactory * QSqlEditorFactory::instance()
{
    static QSqlEditorFactory * _instance = 0;
    
    if( _instance == 0 ){
	_instance = new QSqlEditorFactory();
    }
    
    return _instance;
}
#endif
