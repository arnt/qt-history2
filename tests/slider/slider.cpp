
#include "slider.h"
#include "qslider.h"
#include "qapplication.h"

/*!

*/

MainParent::MainParent( QWidget* parent, const char* name, int f )
    : QWidget( parent, name, f )
{
    QSlider* a = new QSlider( this );
    a->setGeometry( 10, 10, 20, 50 );
    QSlider* b = new QSlider( this );
    b->setGeometry( 40, 10, 20, 50 );
    connect( a, SIGNAL(valueChanged(int)), b, SLOT(setValue(int)) );
}




main(int argc, char** argv)
{
    QApplication app(argc, argv);

    MainParent m;
    app.setMainWidget(&m);
    m.show();

    return app.exec();
}
