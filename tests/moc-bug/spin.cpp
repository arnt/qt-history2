
#include "spin.h"
#include <qspinbox.h>
#include <qslider.h>
#include <qapplication.h>



MainParent::MainParent( QWidget* parent, const char* name, int f )
    : QWidget( parent, name, f )
{
    QSpinBox* b = new QSpinBox( this );
    b->setGeometry( 10, 10, 100, 20 );
    QSlider* s = new QSlider( QSlider::Horizontal, this );
    s->setGeometry( 10, 40, 100, 20 );
    connect( s, SIGNAL(valueChanged(int)), b, SLOT(setValue(int)));
}



main(int argc, char** argv)
{
    QApplication app(argc, argv);

    MainParent* m = new MainParent;
    app.setMainWidget(m);
    m->show();

    int result = app.exec();
    delete m;
    return result;
}
