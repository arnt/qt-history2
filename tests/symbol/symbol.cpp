#include <qapplication.h>
#include <qfont.h>
#include <iostream.h>

int main( int argc, char **argv ) {
	QApplication a( argc, argv );

	cout << "QFont( \"Symbol\", 12, QFont::Normal, false, QFont::AnyCharSet )" << endl;
	QFont f1 = QFont( "Symbol", 12, QFont::Normal, false, QFont::AnyCharSet );
	QFontInfo* fi1 = new QFontInfo( f1 );
	cout << "Family     : " << fi1->family() << endl;
	cout << "CharSet    : " << QFont::encodingName( fi1->charSet() ) << endl;
	cout << "Exact match: " << ( fi1->exactMatch() ? "Yes" : "No" ) << endl;
	delete  fi1;

	cout << "QFont( \"Symbol\" )" << endl;
	QFont f2 = QFont( "Symbol" );
	QFontInfo* fi2 = new QFontInfo( f2 );
	cout << "Family     : " << fi2->family() << endl;
	cout << "CharSet    : " << QFont::encodingName( fi2->charSet() ) << endl;
	cout << "Exact match: " << ( fi2->exactMatch() ? "Yes" : "No" ) << endl;
	delete  fi2;

	return 0;
}
