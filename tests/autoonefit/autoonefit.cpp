#include <qapplication.h>
#include <qscrollview.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qvbox.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>

class Fit : public QFrame {
public:
	Fit( QWidget* parent=0, const char* name=0 );
	QSize sizeHint() const;
	QSize minimumSizeHint() const;
	QSize minimumSize() const;
	QSize maximumSize() const;
	void resize( int w, int h );
};

Fit::Fit( QWidget* parent, const char* name ) : QFrame( parent, name ) {
	QBoxLayout *layout = new QVBoxLayout( this );
	for ( int i = 0; i < 100; ++i ) {
		QString contents;
		contents.setNum( i+1 );
		QLineEdit *lineedit = new QLineEdit( contents, this );
		lineedit->setAlignment( AlignRight );
		layout->addWidget( lineedit );
	}
}

QSize Fit::sizeHint() const {
	QSize s = QFrame::sizeHint();
	qDebug( "Fit::sizeHint() = %d x %d", s.width(), s.height() );
	return s;
}

QSize Fit::minimumSizeHint() const {
	QSize s = QFrame::minimumSizeHint();
	qDebug( "Fit::minimumSizeHint() = %d x %d", s.width(), s.height() );
	return s;
}

QSize Fit::minimumSize() const {
	QSize s = QFrame::minimumSize();
	qDebug( "Fit::minimumSize() = %d x %d", s.width(), s.height() );
	return s;
}

QSize Fit::maximumSize() const {
	QSize s = QFrame::maximumSize();
	qDebug( "Fit::maximumSize() = %d x %d", s.width(), s.height() );
	return s;
}

void Fit::resize( int w, int h ) {
	qDebug( "Fit::resize( %d, %d )", w, h );
	return QFrame::resize( w, h );
}


class MyScrollView : public QScrollView {
	Q_OBJECT
public:
	MyScrollView( QWidget* parent=0, const char* name=0, WFlags f=0 );
	~MyScrollView();
	void setResizePolicy( ResizePolicy r );
	void addChild( QWidget* child, int x=0, int y=0 );
	void moveChild( QWidget* child, int x=0, int y=0 );
	void resizeEvent( QResizeEvent* e );
	bool eventFilter( QObject *, QEvent *e );
	void show();

public slots:
	void resizeContents( int w, int h );
    	void setContentsPos( int x, int y );
	void setResizePolicySlot( int id );
};

MyScrollView::MyScrollView( QWidget* parent, const char* name, WFlags f )
	: QScrollView( parent, name, f ) {
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
	case AutoOneFit:
		s = "AutoOneFit";
		break;
	};
	qDebug( "QScrollView::setResizePolicy( %s )", s );
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
	if ( e->type() == QEvent::LayoutHint )
		qDebug( "MyScrollView::eventFilter( QEvent::LayoutHint )" );
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
		setResizePolicy( AutoOneFit );
		break;
	default:
		qFatal( "internal error in %s:%d", __FILE__, __LINE__ );
	};
}


#include "autoonefit.moc"


int main( int argc, char* argv[] )
{
	QApplication application( argc, argv );

	MyScrollView* sv = new MyScrollView( 0, "sv" );
	Fit* fit = new Fit( sv->viewport(), "fit" );
	sv->setResizePolicy( QScrollView::AutoOne );
	//sv->setResizePolicy( QScrollView::AutoOneFit );
	sv->addChild( fit );
	sv->show();

	QButtonGroup* bg = new QVButtonGroup( "Resize policy:", 0, "bg" );
	bg->resize( 200, 200 );
	QRadioButton* b1 = new QRadioButton( "AutoOne", bg, "autoone" );
	QRadioButton* b2 = new QRadioButton( "AutoOneFit", bg, "autoonefit" );
	QObject::connect( bg, SIGNAL(clicked(int)),
		sv, SLOT(setResizePolicySlot(int)) );
//	sv->setResizePolicySlot(0);
	b1->setChecked( TRUE );
	bg->resize( bg->sizeHint() );
	bg->show();

	application.setMainWidget( sv );
	return application.exec();
}
