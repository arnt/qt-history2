#include <qapplication.h>
#include <qscrollview.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>

class Bazar : public QFrame {
public:
	Bazar( QWidget* parent=0, const char* name=0 );
	QSize sizeHint() const;
	QSize minimumSizeHint() const;
	QSize minimumSize() const;
	QSize maximumSize() const;
	void resize( int w, int h );
};

Bazar::Bazar( QWidget* parent, const char* name ) : QFrame( parent, name ) {
	QBoxLayout *layout = new QVBoxLayout( this );
	for ( int i = 0; i < 100; ++i ) {
		QString contents;
		contents.setNum( i+1 );
		QLineEdit *lineedit = new QLineEdit( contents, this );
		lineedit->setAlignment( AlignRight );
		layout->addWidget( lineedit );
	}
}

QSize Bazar::sizeHint() const {
	qDebug( "Bazar::sizeHint()" );
	return QFrame::sizeHint();
}

QSize Bazar::minimumSizeHint() const {
	qDebug( "Bazar::minimumSizeHint()" );
	return QFrame::minimumSizeHint();
}

QSize Bazar::minimumSize() const {
	qDebug( "Bazar::minimumSize()" );
	return QFrame::minimumSize();
}

QSize Bazar::maximumSize() const {
	qDebug( "Bazar::maximumSize()" );
	return QFrame::maximumSize();
}

void Bazar::resize( int w, int h ) {
	qDebug( "Bazar::resize( %d, %d )", w, h );
	return QFrame::resize( w, h );
}

int main( int argc, char* argv[] )
{
	QApplication application( argc, argv );
	QScrollView* scrollview = new QScrollView( 0 );
	Bazar* bazar = new Bazar( scrollview->viewport() );
	// scrollview->setResizePolicy( QScrollView::AutoOne );
	scrollview->setResizePolicy( QScrollView::AutoOneFit );
	scrollview->addChild( bazar );
	application.setMainWidget( scrollview );
	scrollview->show();
	return application.exec();
}
