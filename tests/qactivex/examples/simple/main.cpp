#include <qapplication.h>
#include <qlayout.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <quuid.h>
#include <qmessagebox.h>
#define QT_ACTIVEX_DEFAULT
#include <qactiveqt.h>

class QSimpleAX : public QWidget, public QActiveQt
{
    Q_OBJECT
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( int value READ value WRITE setValue )
public:
    QSimpleAX( QWidget *parent = 0, const char *name = 0 )
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
	if ( !requestPropertyChange( "text" ) )
	    return;

	edit->blockSignals( TRUE );
	edit->setText( string );
	edit->blockSignals( FALSE );
	emit someSignal();
	emit textChanged( string );

	propertyChanged( "text" );
    }
    void about()
    {
	QMessageBox::information( this, "About QSimpleAX", "This is a Qt widget, and this slot has been\n"
							  "called through ActiveX/OLE dispatching!" );
    }
    void setValue( int i )
    {
	if ( !requestPropertyChange( "value" ) )
	    return;
	slider->blockSignals( TRUE );
	slider->setValue( i );
	slider->blockSignals( FALSE );
	LCD->display( i );
	emit valueChanged( i );

	propertyChanged( "value" );
    }

private:
    QSlider *slider;
    QLCDNumber *LCD;
    QLineEdit *edit;
};

#include "main.moc"

QT_ACTIVEX(QSimpleAX,
	   "{DF16845C-92CD-4AAB-A982-EB9840E74669}",
	   "{616F620B-91C5-4410-A74E-6B81C76FFFE0}",
	   "{E1816BBA-BF5D-4A31-9855-D6BA432055FF}",
	   "{EC08F8FC-2754-47AB-8EFE-56A54057F34E}",	   
	   "{A095BA0C-224F-4933-A458-2DD7F6B85D8F}")

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QWidget *w = 0;

    bool isActiveX = FALSE;
    for ( int arg = 0; arg < argc; ++arg ) {
	if ( !qstrcmp(argv[arg], "-activex") ) {
	    isActiveX = TRUE;
	    break;
	}
    }
    if ( !isActiveX ) {
	w = new QSimpleAX();
	app.setMainWidget( w );
	w->show();
    }

    int res = app.exec();
    delete w;

    return res;
}
