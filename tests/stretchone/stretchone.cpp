#include "stretchone.h"
#include <qapplication.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qvbox.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>


Stretch::Stretch( QWidget* parent, const char* name ) : QFrame( parent, name ) {
	QBoxLayout *layout = new QVBoxLayout( this );
	for ( int i = 0; i < 10; ++i ) {
		QString contents;
		contents.setNum( i+1 );
		QLineEdit *lineedit = new QLineEdit( contents, this );
		lineedit->setAlignment( AlignRight );
		layout->addWidget( lineedit );
	}
}

QSize Stretch::sizeHint() const {
	QSize s = QFrame::sizeHint();
	qDebug( "Stretch::sizeHint() = %d x %d", s.width(), s.height() );
	return s;
}

QSize Stretch::minimumSizeHint() const {
	QSize s = QFrame::minimumSizeHint();
	qDebug( "Stretch::minimumSizeHint() = %d x %d", s.width(), s.height() );
	return s;
}

QSize Stretch::minimumSize() const {
	QSize s = QFrame::minimumSize();
	qDebug( "Stretch::minimumSize() = %d x %d", s.width(), s.height() );
	return s;
}

QSize Stretch::maximumSize() const {
	QSize s = QFrame::maximumSize();
	qDebug( "Stretch::maximumSize() = %d x %d", s.width(), s.height() );
	return s;
}

void Stretch::resize( int w, int h ) {
	qDebug( "Stretch::resize( %d, %d )", w, h );
	return QFrame::resize( w, h );
}


MyScrollView::MyScrollView( QWidget* parent, const char* name, WFlags f )
	: QScrollView( parent, name, f ) {
	installEventFilter( this );

}

MyScrollView::~MyScrollView() {
}

void MyScrollView::setResizePolicy( ResizePolicy r ) {
	const char* s;
	switch ( r ) {
	case Default:
		s = "Default";
		break;
	case Manual:
		s = "Manual";
		break;
	case AutoOne:
		s = "AutoOne";
		break;
	case StretchOne:
		s = "StretchOne";
		break;
	};
	qDebug( "MyScrollView::setResizePolicy( %s )", s );
	QScrollView::setResizePolicy( r );
}

void MyScrollView::addChild( QWidget* child, int x=0, int y=0 ) {
	qDebug( "MyScrollView::addChild( %d, %d )", x, y );
	QScrollView::addChild( child, x, y );
}

void MyScrollView::moveChild( QWidget* child, int x=0, int y=0 ) {
	qDebug( "MyScrollView::moveChild( %d, %d )", x, y );
	QScrollView::moveChild( child, x, y );
}

void MyScrollView::resizeEvent( QResizeEvent* e ) {
	qDebug( "MyScrollView::resizeEvent()" );
	QScrollView::resizeEvent( e );
}

bool MyScrollView::eventFilter( QObject* obj, QEvent* e ) {
	switch ( e->type() ) {
	case QEvent::LayoutHint:
		qDebug( "MyScrollView::eventFilter( \"%s\", LayoutHint )",
			obj->className() );
		break;
	case QEvent::Resize:
		qDebug( "MyScrollView::eventFilter( \"%s\", Resize )",
			obj->className() );
		break;
	default:
		break;
	}
	return QScrollView::eventFilter( obj, e );
}

void MyScrollView::show() {
	qDebug( "MyScrollView::show()" );
	QScrollView::show();
}

void MyScrollView::resizeContents( int w, int h ) {
	qDebug( "MyScrollView::resizeContents( %d, %d )", w, h );
	QScrollView::resizeContents( w, h );
}

void MyScrollView::setContentsPos( int x, int y ) {
	qDebug( "MyScrollView::setContentsPos( %d, %d )", x, y );
	QScrollView::setContentsPos( x, y );
}

void MyScrollView::setResizePolicySlot( int id ) {
	switch ( id ) {
	case 0:
		setResizePolicy( AutoOne );
		break;
	case 1:
		setResizePolicy( StretchOne );
		break;
	default:
		qFatal( "internal error in %s:%d", __FILE__, __LINE__ );
	};
}


int main( int argc, char* argv[] )
{
	QApplication application( argc, argv );

	MyScrollView* sv = new MyScrollView( 0, "sv" );
	Stretch* fit = new Stretch( sv->viewport(), "fit" );
	sv->addChild( fit );
	sv->show();

	QButtonGroup* bg = new QVButtonGroup( "Resize policy:", 0, "bg" );
	bg->resize( 200, 200 );
	QRadioButton* b1 = new QRadioButton( "AutoOne", bg, "autoone" );
	QRadioButton* b2 = new QRadioButton( "StretchOne", bg, "autoonefit" );
	QObject::connect( bg, SIGNAL(clicked(int)),
		sv, SLOT(setResizePolicySlot(int)) );
	sv->setResizePolicy( QScrollView::StretchOne );
	b2->setChecked( TRUE );
	bg->resize( bg->sizeHint() );
	bg->show();

	application.setMainWidget( sv );
	return application.exec();
}
