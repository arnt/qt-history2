#include "qcleanuphandler.h"
#include "qwidget.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qspinbox.h"
#include "qcombobox.h"
#include "qlayout.h"

#include "qeditorfactory.h"
#include "qdatetimeedit.h"

#ifndef QT_NO_SQL

/*!  Constructs a SQL editor factory

*/

QEditorFactory::QEditorFactory ( QObject * parent, const char * name )
    : QObject( parent, name )
{

}

/*! Destroys the object and frees any allocated resources.

*/

QEditorFactory::~QEditorFactory()
{

}

static QEditorFactory * defaultfactory = 0;
QCleanupHandler< QEditorFactory > q_cleanup_editor_factory;

/*! Destroys the object and frees any allocated resources.

*/

QEditorFactory * QEditorFactory::defaultFactory()
{
    if( defaultfactory == 0 ){
	defaultfactory = new QEditorFactory();
	q_cleanup_editor_factory.add( defaultfactory );
    }

    return defaultfactory;
}

/*!

  Creates and returns the appropriate editor for the QVariant \a v.
  If the QVariant is invalid, 0 is returned.

*/

QWidget * QEditorFactory::createEditor( QWidget * parent, const QVariant & v )
{
    QWidget * w = 0;
    switch( v.type() ){
	case QVariant::Invalid:
	    w = 0;
	    break;
	case QVariant::Bool:
	    w = new QComboBox( parent );
	    ((QComboBox *) w)->insertItem( "False" );
	    ((QComboBox *) w)->insertItem( "True" );
	    break;
	case QVariant::UInt:
	case QVariant::Int:
	    w = new QSpinBox( -999999, 999999, 1, parent );
	    break;
	case QVariant::String:
	case QVariant::CString:
	case QVariant::Double:
	    w = new QLineEdit( parent );
	    break;
	case QVariant::Date:
	    w = new QDateEdit( parent );
	    break;
	case QVariant::Time:
	    w = new QTimeEdit( parent );
	    break;
    case QVariant::DateTime: {
	w = new QDateTimeEdit( parent );
	break;
    }
	case QVariant::Pixmap:
	    w = new QLabel( parent );
	    break;
	case QVariant::Palette:
	case QVariant::ColorGroup:
	case QVariant::Color:
	case QVariant::Font:
	case QVariant::Brush:
	case QVariant::Bitmap:
	case QVariant::Cursor:
	case QVariant::Map:
	case QVariant::StringList:
	case QVariant::Rect:
	case QVariant::Size:
	case QVariant::IconSet:
	case QVariant::Point:
	case QVariant::PointArray:
	case QVariant::Region:
	case QVariant::SizePolicy:
	case QVariant::ByteArray:
	default:
	    w = new QWidget( parent );
	    //	    w = new QLineEdit( parent );
	    break;
    }
    return w;
}

/*!

  Creates and returns the appropriate editor for the field \a f.

*/

QWidget * QEditorFactory::createEditor( QWidget * parent, const QSqlField & f )
{
    QVariant v = f.value();
    return createEditor( parent, v );
}

#endif // QT_NO_SQL


