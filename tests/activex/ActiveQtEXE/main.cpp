#include <qapplication.h>
#include <qlayout.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <quuid.h>
#include <qt_windows.h>

extern "C" const IID IID_ActiveQtApp = {0xA095BA0C,0x224F,0x4933,{0xA4,0x58,0x2D,0xD7,0xF6,0xB8,0x5D,0x8F}};

class QExeTest : public QWidget
{
    Q_OBJECT
public:
    QExeTest( QWidget *parent = 0, const char *name = 0 )
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

public slots:
    void doSomething()
    {
	Beep( 440, 1000 );
    }
};

#include "main.moc"

QWidget *axmain( QWidget *parent )
{
    return new QExeTest( parent );
}
