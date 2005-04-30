/*
    usage: scanui filename.ui functionname1 functionname2 ...

    Will output the specified function from the specified .ui file to
    stdout. It is used, e.g. in the Designer manual, to include code
    snippets from .ui files.
*/
#include <qdom.h>
#include <qfile.h>
#include <iostream.h>
#include <qregexp.h>


void error( const QString &msg = QString() );
void printFunction(
	const QString &className, const QString &fn, const QDomDocument & ui );


int main( int argc, char **argv )
{
    if ( argc < 3 ) error();

    QString fn( argv[2] );

    QFile uiFile( argv[1] );
    if ( ! uiFile.open( IO_ReadOnly ) )
	error( QString( "failed to open " ) + argv[1] );

    QDomDocument ui( "uifile" );
    if ( ! ui.setContent( &uiFile ) )
	error( QString( "failed to DOM " ) + argv[1] );

    QString className;
    QDomNodeList list = ui.elementsByTagName( "class" );
    for ( uint i = 0; i < list.length(); ++i ) {
	QDomNode node = list.item( i );
	QDomCharacterData nameData = node.firstChild().toCharacterData();
	className = nameData.data();
	break;
    }

    for ( int i = 2; i < argc; i++ )
	printFunction( className, argv[i], ui );

    uiFile.close();

    return 0;
}


void printFunction(
	const QString &className, const QString &fn, const QDomDocument & ui )
{
    QRegExp re( "^" + fn + "\\(" );
    QString returnType;
    QString fnName;

    QDomNodeList list = ui.elementsByTagName( "slot" );
    for ( uint i = 0; i < list.length(); ++i ) {
	QDomNode node = list.item( i );

	QString tmpRet;
	QString temp = node.toElement().attribute( "returnType" );
	if ( ! temp.isNull() ) tmpRet = temp;

	QDomCharacterData nameData = node.firstChild().toCharacterData();
	temp = nameData.data();
	if ( re.search( temp ) > -1 ) {
	    if ( temp.length() > fnName.length() ) fnName = temp;
	    returnType = tmpRet;
	}
    }
    if ( returnType.isNull() ) returnType = "void";
    if ( fnName.isNull() ) error( QString( "function not found: " ) + fn );
    cout << returnType << " " << className << "::" << fnName << "\n";

    QString lines;
    list = ui.elementsByTagName( "function" );
    for ( uint i = 0; i < list.length(); ++i ) {
	QDomNode node = list.item( i );
	QString name = node.toElement().attribute( "name" );
	if ( re.search( name ) > -1 ) {
	    QDomCharacterData nameData = node.firstChild().toCharacterData();
	    cout << nameData.data() << "\n";
	    break;
	}
    }
}


void error( const QString & msg )
{
    if ( ! msg.isNull() )
	cout << "scanui error: " << msg << "\n";
    else
	cout << "usage: scanui file.ui functionname\n";
    exit(0);
}
