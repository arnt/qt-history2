#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcombobox.h>

#include "qsqleditorfactory.h"
#include "qsqleditor.h"

#ifndef QT_NO_SQL

QSqlEditorFactory::QSqlEditorFactory()
{

}

/*!

  Creates and returns the appropriate editor for the QVariant \a v.  A
  null pointer is returned if the QVariant is invalid.
  
*/
QWidget * QSqlEditorFactory::createEditor( QWidget * parent, const QVariant & v )
{
    QWidget * w;
    
    switch( v.type() ){
	
	case QVariant::Invalid:
	    w = 0;
	    break;
	case QVariant::Bool:
	    w = new QComboBox( parent );
	    ((QComboBox *) w)->insertItem( "False" );
	    ((QComboBox *) w)->insertItem( "True" );
	    break;
	case QVariant::Date:
/*	    w = new DateBookMonth( 0 ); 
	    w->reparent( 0, QWidget::WStyle_Customize + 
			 QWidget::WStyle_NoBorder, QPoint( 0, 0 ) );
	    w->resize( 100, 100 );
*/
	    break;
	case QVariant::UInt:
	case QVariant::Int:
	    w = new QSpinBox( -99999, 99999, 1, parent );
	    break;
	case QVariant::String:
	case QVariant::CString:
	    w = new QLineEdit( parent);
	    break;
	case QVariant::Map:
	case QVariant::StringList:
	case QVariant::Font:
	case QVariant::Brush:
	case QVariant::Pixmap:
	case QVariant::Rect:
	case QVariant::Size:
	case QVariant::Color:
	case QVariant::Palette:
	case QVariant::ColorGroup:
	case QVariant::IconSet:
	case QVariant::Point:
	case QVariant::Double:
	case QVariant::PointArray:
	case QVariant::Region:
	case QVariant::Bitmap:
	case QVariant::Cursor:
	case QVariant::SizePolicy:
	case QVariant::Time:
	case QVariant::DateTime:
	case QVariant::ByteArray:
	default:
	    w = new QLineEdit( parent );
	    break;
    }
    return w;
}


/*!
  
  Returns a pointer to the sole instance of QSqlEditorFactory.
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
