#include <qsettings.h>

int main ( int argc, char **argv )
{
    QSettings settings;
    settings.writeEntry( "/Software/Trolltech/Qt/QSettings/string", "String" );
	settings.writeEntry( "/Software/Trolltech/Qt/QSettings/int", 0xffff );
	settings.writeEntry( "/Software/Trolltech/Qt/QSettings/bool", true );
	settings.writeEntry( "/Software/Trolltech/Qt/QSettings/double", 3.1415927 );

	qDebug( settings.readEntry( "/Software/Trolltech/Qt/QSettings/string" ) );
	qDebug( "%d", settings.readNumEntry( "/Software/Trolltech/Qt/QSettings/int" ) );
	qDebug( "%d", settings.readBoolEntry( "/Software/Trolltech/Qt/QSettings/bool" ) );
	qDebug( "%g", settings.readDoubleEntry( "/Software/Trolltech/Qt/QSettings/double" ) );

	return 0;
}
