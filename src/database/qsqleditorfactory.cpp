#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>

#include "qsqleditorfactory.h"
#include "qsqleditor.h"

#ifndef QT_NO_SQL

QSqlEditorFactory::QSqlEditorFactory()
{

}

/*!
  Creates and returns the appropriate editor for the QSqlField.

*/
QWidget * QSqlEditorFactory::createEditor( QWidget * parent,
					   QSqlField & field )
{
    QSqlEditor* w;
    switch ( field.type() ) {
    case QVariant::Int:
	w = new QSqlSpinBox( parent, field );
	break;
    default:
    case QVariant::String:
    case QVariant::CString:
	w = new QSqlLineEdit( parent, field );
	break;
    }
    w->syncToEditor();
    return w;
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
