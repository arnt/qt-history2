#include <qapplication.h>
#include <qlabel.h>

int main( int argc, char *argv[] )
{
	QApplication application( argc, argv );
	QChar unicode[] = { 1112, 1161, 1102, 1100, 1173, 1105 };
	QString s( unicode, 6 );
	QLabel label( s, 0 );
	application.setMainWidget( &label );
	label.show();
	return application.exec();
}
