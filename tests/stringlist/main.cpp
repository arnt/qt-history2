#include <qstringlist.h>

static void printStringList( const QString &s, const QStringList &lst ) 
{
    qDebug( "String: '%s'", s.latin1() );
    for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it )
	qDebug( "  `%s`", ( *it ).latin1() );
    qDebug( "\n------------------------\n" );
}

int main( int, char ** )
{
    qDebug( "no empty entries:\n" );
    printStringList( "", QStringList::split( ".", "", FALSE ) ); 
    printStringList( "abc", QStringList::split( ".", "abc", FALSE ) ); 
    printStringList( "abc.def.ghi", QStringList::split( ".", "abc.def.ghi", FALSE ) ); 
    printStringList( ".123.xyz", QStringList::split( ".", ".123.xyz", FALSE ) ); 
    printStringList( "aaa..bbb", QStringList::split( ".", "aaa..bbb", FALSE ) ); 
    printStringList( "xxx..", QStringList::split( ".", "xxx..", FALSE ) ); 
    printStringList( "...", QStringList::split( ".", "...", FALSE ) ); 
    printStringList( ".abc..def.ghi", QStringList::split( ".", ".abc..def.ghi", FALSE ) ); 

    qDebug( "allow empty entries:\n" );
    printStringList( "", QStringList::split( ".", "", TRUE ) ); 
    printStringList( "abc", QStringList::split( ".", "abc", TRUE ) ); 
    printStringList( "abc.def.ghi", QStringList::split( ".", "abc.def.ghi", TRUE ) ); 
    printStringList( ".123.xyz", QStringList::split( ".", ".123.xyz", TRUE ) ); 
    printStringList( "aaa..bbb", QStringList::split( ".", "aaa..bbb", TRUE ) ); 
    printStringList( "xxx..", QStringList::split( ".", "xxx..", TRUE ) ); 
    printStringList( "...", QStringList::split( ".", "...", TRUE ) ); 
    printStringList( ".abc..def.ghi", QStringList::split( ".", ".abc..def.ghi", TRUE ) ); 
}
