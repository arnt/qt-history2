#include <qapplication.h>
#include <qlabel.h>

int main( int argc, char *argv[] )
{
	QApplication application( argc, argv );
	QChar unicode[] = { 3041, 3042, 3043, 3044, 3045, 3046, 3047, 3048 };
	QString s( unicode, 8 );
	QLabel label( s, 0 );
	application.setMainWidget( &label );
	label.show();
	return application.exec();
}
