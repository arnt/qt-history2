#include <qsettings.h>

int main ( int argc, char **argv )
{
    QSettings settings;
    settings.writeEntry( "/Trolltech/Qt/QSettings/string", "String" );
    settings.writeEntry( "/Trolltech/Qt/QSettings/int", 0xffff );
    settings.writeEntry( "/Trolltech/Qt/QSettings/bool", true );
    settings.writeEntry( "/Trolltech/Qt/QSettings/double", 3.1415927 );

    qDebug( settings.readEntry( "/Trolltech/Qt/QSettings/string" ) );
    qDebug( "%d", settings.readNumEntry( "/Trolltech/Qt/QSettings/int" ) );
    qDebug( "%d", settings.readBoolEntry( "/Trolltech/Qt/QSettings/bool" ) );
    qDebug( "%g", settings.readDoubleEntry( "/Trolltech/Qt/QSettings/double" ) );

    return 0;
}
