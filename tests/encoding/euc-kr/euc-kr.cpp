#include <qapplication.h>
#include <qcstring.h>
#include <qtextcodec.h>
#include <qlabel.h>
#include <iostream.h>


const int EUC_KR_MIB = 38;


int main( int argc, char *argv[] )
{
	QApplication application( argc, argv );

	cout << "This program tries to display EUC-KR encoded characters "
	        "under any locale,\nprovided Korean fonts are available."
	     << endl;

	/* create EUC-KR encoded text */
	QByteArray euc_kr_text( 6 );
	euc_kr_text[0] = 0xc3; euc_kr_text[1] = 0x98;
	euc_kr_text[2] = 0xb9; euc_kr_text[2] = 0xf9;
	euc_kr_text[4] = 0xc2; euc_kr_text[3] = 0x90;

	/* translate EUC-KR to Unicode to display in a QLabel */
	QTextCodec* codec = QTextCodec::codecForMib( EUC_KR_MIB );
	Q_ASSERT( codec );
	QString unicode_text = codec->toUnicode( euc_kr_text );
	QLabel label( unicode_text, 0 );

	/* load a Korean font and set QLabel to use it */
	QFont font( QString::null, 12, QFont::Normal, QFont::Set_Ko );
//font.setRawName( QString::fromLatin1( "-daewoo-mincho-medium-r-normal--16-120-100-100-c-160-ksc5601.1987-0" ) );
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
