#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include "qsqleditorfactory.h"
#include "qsqleditor.h"

#ifndef QT_NO_SQL

QSqlEditorFactory::QSqlEditorFactory()
{

}

/*!
  Creates and returns the appropriate editor for the QSqlField \a field.

*/
QWidget * QSqlEditorFactory::createEditor( QWidget * parent, const QVariant & v )
{
    QWidget * w;
    
    switch( v.type() ){
	case QVariant::Int:
/*	    w = new DateBookMonth( 0 ); 
	    w->reparent( 0, QWidget::WStyle_Customize + 
			 QWidget::WStyle_NoBorder, QPoint(0,0) );
	    w->resize( 100, 100 );*/
	    w = new QSpinBox( parent );
	    break;
	case QVariant::String:
	case QVariant::CString:
	    w = new QLineEdit( parent);
	    break;
	default:
	    w = new QLineEdit( parent );
	    break;
    }
    return w;
}


/*!
  Returns a pointer to the only existing instance of a
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

#endif // QT_NO_SQL
