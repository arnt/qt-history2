#include <qapplication.h>
#include <qcstring.h>
#include <qtextcodec.h>
#include <qlabel.h>
#include <iostream.h>


const int ISO_8859_7_MIB = 10;


int main( int argc, char *argv[] )
{
	QApplication application( argc, argv );

	cout << "This program tries to display ISO 8859-7 encoded characters "
	        "under any locale,\nprovided Greek fonts are available."
	     << endl;

	/* create ISO 8859-7 encoded text */
	const char* iso_8859_7_text = "Ελληνικά";

	/* translate ISO 8859-7 to Unicode to display in a QLabel */
	QTextCodec* codec = QTextCodec::codecForMib( ISO_8859_7_MIB );
	Q_ASSERT( codec );
	QString unicode_text = codec->toUnicode( iso_8859_7_text );
	QLabel label( unicode_text, 0 );

	/* load a Greek font and set QLabel to use it */
	QFont font( QString::null, 12, QFont::Normal, QFont::ISO_8859_7 );
	cout << "Raw Name: " << font.rawName() << endl;
	QFontInfo info( font );
	cout << "Family:   " << info.family() << endl;
	cout << "Size:     " << info.pointSize() << " pts" << endl;
	cout << "CharSet:  " << QFont::encodingName( info.charSet() ) << endl;
	label.setFont( font );

	application.setMainWidget( &label );
	label.show();
	return application.exec();
}
