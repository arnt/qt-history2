#include <qapplication.h>
#include <qlabel.h>

class Keypad : public QLabel {
public:
    Keypad( QWidget *parent=0, const char *name=0 );
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );
};

Keypad::Keypad( QWidget *parent, const char *name ) : QLabel( parent, name )
{
    QFont f = font();
    f.setBold( TRUE );
    f.setPointSize( f.pointSize() * 2 );
    setFont( f );
}

void Keypad::keyPressEvent( QKeyEvent *e )
{

    
    if ( e->state() & Qt::Keypad ) {
	qDebug( "Keypad key pressed!" );
	setText( "Keypad!" );
    } else {
	setText( QString::number( e->ascii() ) );
    }
}

void Keypad::keyReleaseEvent( QKeyEvent *e )
{
    if ( e->state() & Qt::Keypad ) {
	qDebug( "Keypad key released!" );
	setText( "Keypad!" );
    } else {
	setText( QString::null );
    }
}

main(int argc, char** argv)
{
    QApplication application(argc, argv);
    Keypad keypad;
    keypad.show();
    application.setMainWidget( &keypad );
    return application.exec();
}
