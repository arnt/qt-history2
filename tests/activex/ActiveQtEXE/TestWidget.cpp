#include "testwidget.h"
#include <qlayout.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qlineedit.h>


TestWidget::TestWidget( QWidget *parent, const char *name )
: QWidget( parent, name )
{
    QVBoxLayout *vbox = new QVBoxLayout( this );

    QSlider *slider = new QSlider( 0, 100, 1, 50, QSlider::Horizontal, this );
    QLCDNumber *LCD = new QLCDNumber( 3, this );
    QLineEdit *edit = new QLineEdit( this );
    QLineEdit *edit2 = new QLineEdit( this );

    QObject::connect( slider, SIGNAL( valueChanged( int ) ), LCD, SLOT( display( int ) ) );

    vbox->addWidget( slider );
    vbox->addWidget( LCD );
    vbox->addWidget( edit );
    vbox->addWidget( edit2 );
}
