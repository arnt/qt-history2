#include <qsettings.h>

int main ( int argc, char **argv )
{
    {
	QSettings settings;
	settings.setPath( "trolltech.com", "QSettings", QSettings::User );

	settings.writeEntry( "/string", "String" );
	settings.writeEntry( "/int", 0xffff );
	settings.writeEntry( "/bool", true );
	settings.writeEntry( "/double", 3.1415927 );

	settings.resetGroup();

	QStringList subkeys = settings.subkeyList( "/Trolltech" );
	for ( QStringList::Iterator it = subkeys.begin(); it != subkeys.end(); ++it ) {
	    qDebug( "/Trolltech/%s", (*it).latin1() );
	    QStringList entries = settings.entryList( "/Trolltech/" + *it );
	    for ( QStringList::Iterator it2 = entries.begin(); it2 != entries.end(); ++it2 ) {
		qDebug( "/Trolltech/%s/%s", (*it).latin1(), (*it2).latin1() );
	    }
	}
    }

    {
	QSettings settings;
	qDebug( settings.readEntry( "/Trolltech/QSettings/string" ) );
	qDebug( "%d", settings.readNumEntry( "/Trolltech/QSettings/int" ) );
	qDebug( "%d", settings.readBoolEntry( "/Trolltech/QSettings/bool" ) );
	qDebug( "%g", settings.readDoubleEntry( "/Trolltech/QSettings/double" ) );
	bool ok;
	QString str = settings.readEntry( "/Trolltech/QSettings/foo", QString::null, &ok );
	if ( ok )
	    qDebug( "%s", str );

	settings.removeEntry( "/Trolltech/QSettings/string" );
	settings.removeEntry( "/Trolltech/QSettings/bool" );
	settings.removeEntry( "/Trolltech/QSettings/double" );
	settings.removeEntry( "/Trolltech/QSettings/int" );
    }

    return 0;
}
