
#include <qapplication.h>
#include "qimpeninput.h"

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    QIMPenInput w( 0, 0, QWidget::WStyle_Customize | QWidget::WStyle_NoBorder 
			    | QWidget::WStyle_StaysOnTop );
    w.setFrameStyle( QFrame::Box | QFrame::Plain );
    w.setLineWidth( 1 );
    w.addCharSet( "asciilower.qpt" );
    w.addCharSet( "asciiupper.qpt" );
    w.addCharSet( "numeric.qpt" );
    w.show();
    w.resize( w.width(), w.sizeHint().height() + 1 );
    w.move( 0, app.desktop()->height() - w.height() );
    w.hideShow();

    app.setMainWidget( &w );
    app.exec();

    return 0;
}
