#include <qapplication.h>
#include <qlabel.h>
#include <iostream.h>

int main( int argc, char *argv[] )
{
	QApplication application( argc, argv );

	QChar unicode[] = { 1112, 1161, 1102, 1100, 1173, 1105 };
	QString s( unicode, 6 );
	QLabel label( s, 0 );
	QFont font = label.font();
	cout << "Raw Name: " << font.rawName() << endl;
	QFontInfo info = label.fontInfo();
	cout << "Family:   " << info.family() << endl;
	cout << "Size:     " << info.pointSize() << " pts" << endl;
	cout << "CharSet:  " << QFont::encodingName( info.charSet() ) << endl;

	application.setMainWidget( &label );
	label.show();
	return application.exec();
}
