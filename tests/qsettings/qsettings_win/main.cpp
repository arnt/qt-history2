#include <qsettings.h>

static void writeTest( QSettings &settings )
{
    settings.writeEntry( "/values/string", "String" );
    settings.writeEntry( "/values/int", 0xffff );
    settings.writeEntry( "/values/bool", true );
    settings.writeEntry( "/values/double", 3.1415927 );
    QStringList list;
    list << "a" << "b" << "c";
    settings.writeEntry( "/values/list", list );
    
    QStringList subkeys = settings.subkeyList( "/values" );
    for ( QStringList::Iterator it = subkeys.begin(); it != subkeys.end(); ++it ) {
	qDebug( "/values/%s", (*it).latin1() );
	QStringList entries = settings.entryList( "/values/" + *it );
	for ( QStringList::Iterator it2 = entries.begin(); it2 != entries.end(); ++it2 ) {
	    qDebug( "/values/%s/%s", (*it).latin1(), (*it2).latin1() );
	}
    }
}

static void readTest( QSettings &settings )
{
    qDebug( "%s", settings.readEntry( "/values/string" ).latin1() );
    qDebug( "%d", settings.readNumEntry( "/values/int" ) );
    qDebug( "%d", settings.readBoolEntry( "/values/bool" ) );
    qDebug( "%g", settings.readDoubleEntry( "/values/double" ) );
    QStringList list = settings.readListEntry( "/values/list" );
    qDebug( "%s", list.join(" ").latin1() );
    bool ok;
    QString str = settings.readEntry( "/values/foo", QString::null, &ok );
    if ( ok )
	qDebug( "%s", str );
    
    settings.removeEntry( "/values/string" );
    settings.removeEntry( "/values/bool" );
    settings.removeEntry( "/values/double" );
    settings.removeEntry( "/values/int" );
    settings.removeEntry( "/values/list" );
    settings.removeEntry( "/." );
}

int main ( int argc, char **argv )
{
    {
	QSettings settings( QSettings::Ini );
	settings.setPath( "trolltech.com", "QSettings", QSettings::User );
	
	writeTest( settings );
    }
    {
	QSettings settings( QSettings::Ini );
	settings.setPath( "trolltech.com", "QSettings", QSettings::User );
	
	readTest( settings );
    }
    {
	QSettings settings;
	settings.setPath( "trolltech.com", "QSettings" );
	
	writeTest( settings );
    }
    
    {
	QSettings settings;
	settings.insertSearchPath( QSettings::Windows, "/Trolltech/QSettings" );
	
	readTest( settings );
    }
    
    return 0;
}
