#include <qapplication.h>
#include <qlabel.h>

int main( int argc, char *argv[] )
{
	QApplication application( argc, argv );
	QChar unicode[] = { 917, 955, 955, 951, 957, 953, 954, 940 };
	QString s( unicode, 8 );
	QLabel label( s, 0 );
	application.setMainWidget( &label );
	label.show();
	return application.exec();
}
