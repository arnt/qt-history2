#include <qapplication.h>
#include "qfontdialog.h"
#include <qpushbutton.h>


main(int argc, char** argv)
{
    QApplication app(argc, argv);

    // QFont fnt( "utopia", 8, 48, FALSE );
    //fnt.setCharSet( QFont::ISO_8859_1 );
    //app.setFont( fnt );

#if 1
    bool ok;
    QFont f = QFontDialog::getFont( &ok );
    if ( ok ) {
	qWarning( "OK!" );
    } else {
	qWarning( "CANCEL!" );
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
