#include <qapplication.h>
#include <qfontdialog.h>
#include <qpushbutton.h>


main(int argc, char** argv)
{
    QApplication app(argc, argv);

#if 1
    bool ok;
    QFont f = QFontDialog::getFont( &ok );
    if ( ok ) {
	warning( "OK!" );
    } else {
	warning( "CANCEL!" );
    }

    QPushButton quit( 0 );
    quit.resize( 200, 200 );
    quit.setFont( f );
    quit.setText( "Quit" );
    quit.show();
    app.setMainWidget(&quit);
    QObject::connect( &quit, SIGNAL(clicked()),
	     &app, SLOT(quit()) );
#else
    QFontDialog d;
    app.setMainWidget(&d);
    d.show();
#endif
    return app.exec();
}
