#include <qsettings.h>

int main ( int argc, char **argv )
{
    {
	QSettings settings;
	settings.setPath( "trolltech.com", "QSettings" );

	settings.writeEntry( "/string", "String" );
	settings.writeEntry( "/int", 0xffff );
	settings.writeEntry( "/bool", true );
	settings.writeEntry( "/double", 3.1415927 );
	QStringList list;
	list << "a" << "b" << "c";
	settings.writeEntry( "/list", list );

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
	settings.insertSearchPath( QSettings::Windows, "/Trolltech/QSettings" );
	qDebug( "%s", settings.readEntry( "/string" ).latin1() );
	qDebug( "%d", settings.readNumEntry( "/int" ) );
	qDebug( "%d", settings.readBoolEntry( "/bool" ) );
	qDebug( "%g", settings.readDoubleEntry( "/double" ) );
	QStringList list = settings.readListEntry( "/list" );
	qDebug( "%s", list.join(" ").latin1() );
	bool ok;
	QString str = settings.readEntry( "/Trolltech/QSettings/foo", QString::null, &ok );
	if ( ok )
	    qDebug( "%s", str );

	settings.removeEntry( "/Trolltech/QSettings/string" );
	settings.removeEntry( "/Trolltech/QSettings/bool" );
	settings.removeEntry( "/Trolltech/QSettings/double" );
	settings.removeEntry( "/Trolltech/QSettings/int" );
	settings.removeEntry( "/Trolltech/QSettings/list" );
    }

    return 0;
}
