#include <qsettings.h>

int main ( int argc, char **argv )
{
    QSettings settings;
    settings.writeEntry( "/Trolltech/QSettings/string", "String" );
    settings.writeEntry( "/Trolltech/QSettings/int", 0xffff );
    settings.writeEntry( "/Trolltech/QSettings/bool", true );
    settings.writeEntry( "/Trolltech/QSettings/double", 3.1415927 );

    QStringList subkeys = settings.subkeyList( "/Trolltech" );
    for ( QStringList::Iterator it = subkeys.begin(); it != subkeys.end(); ++it ) {
	qDebug( "/Trolltech/%s", (*it).latin1() );
    }

    qDebug( settings.readEntry( "/Trolltech/QSettings/string" ) );
    qDebug( "%d", settings.readNumEntry( "/Trolltech/QSettings/int" ) );
    qDebug( "%d", settings.readBoolEntry( "/Trolltech/QSettings/bool" ) );
    qDebug( "%g", settings.readDoubleEntry( "/Trolltech/QSettings/double" ) );
    QString str = settings.readEntry( "/Trolltech/QSettings/foo" );
    qDebug( "%s", str );

    settings.removeEntry( "/Trolltech/QSettings/string" );
    settings.removeEntry( "/Trolltech/QSettings/bool" );
    settings.removeEntry( "/Trolltech/QSettings/double" );
    settings.removeEntry( "/Trolltech/QSettings/int" );

    return 0;
}
