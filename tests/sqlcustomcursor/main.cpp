#include <qapplication.h>
#include <qmainwindow.h>
#include <qlayout.h>
#include "sqlex.h"


class SqlExW: public QMainWindow
{
    Q_OBJECT

public:
    SqlExW( QWidget * parent = 0, const char *name = 0): QMainWindow( parent, name )
    {
	SqlEx* se = new SqlEx( this, "SqlExplorer" );
	setCentralWidget( se );
    }

    virtual ~SqlExW();
};

SqlExW::~SqlExW() {}

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    SqlExW sqlw( 0, "MainWindow" );
    app.setMainWidget( &sqlw );
    sqlw.show();
    return app.exec();
}


#include "main.moc"

