#include <qapplication.h>
#include <qlayout.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <quuid.h>
#include <qmessagebox.h>

extern "C" const IID IID_ActiveQtApp = {0xA095BA0C,0x224F,0x4933,{0xA4,0x58,0x2D,0xD7,0xF6,0xB8,0x5D,0x8F}};

class QExeTest : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( int value READ value WRITE setValue )
public:
    QExeTest( QWidget *parent = 0, const char *name = 0 )
    : QWidget( parent, name )
    {
	QVBoxLayout *vbox = new QVBoxLayout( this );

	slider = new QSlider( 0, 100, 1, 0, QSlider::Horizontal, this );
	LCD = new QLCDNumber( 3, this );
	edit = new QLineEdit( this );

	connect( slider, SIGNAL( valueChanged( int ) ), this, SLOT( setValue(int) ) );
	connect( edit, SIGNAL(textChanged(const QString&)), this, SLOT(setText(const QString&)) );

	vbox->addWidget( slider );
	vbox->addWidget( LCD );
	vbox->addWidget( edit );
    }

    QString text() const 
    { 
	return edit->text(); 
    }
    int value() const
    {
	return slider->value();
    }

signals:
    void someSignal();
    void valueChanged(int);
    void textChanged(const QString&);

public slots:
    void setText( const QString &string )
    {
	edit->blockSignals( TRUE );
	edit->setText( string );
	edit->blockSignals( FALSE );
	emit someSignal();
	emit textChanged( string );
    }
    void about()
    {
	QMessageBox::information( this, "About QExtTest", "This is a Qt widget, and this slot has been\n"
							  "called through ActiveX/OLE dispatching!" );
    }
    void setValue( int i )
    {
	slider->blockSignals( TRUE );
	slider->setValue( i );
	slider->blockSignals( FALSE );
	LCD->display( i );
	emit valueChanged( i );
    }

private:
    QSlider *slider;
    QLCDNumber *LCD;
    QLineEdit *edit;
};

#include "main.moc"

QWidget *axmain( QWidget *parent )
{
    return new QExeTest( parent );
}

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QWidget *w = axmain( 0 );
    app.setMainWidget( w );
    w->show();

    return app.exec();
}
