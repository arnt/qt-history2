#include "languageinterfaceimpl.h"
#include <qobject.h>

LanguageInterfaceImpl::LanguageInterfaceImpl()
    : ref( 0 )
{
}

QUnknownInterface *LanguageInterfaceImpl::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_LanguageInterface )
	iface = (LanguageInterface*)this;

    if ( iface )
	iface->addRef();
    return iface;
}

unsigned long LanguageInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long LanguageInterfaceImpl::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

QStringList LanguageInterfaceImpl::featureList() const
{
    QStringList lst;
    lst << "C++";
    return lst;
}

class NormalizeObject : public QObject
{
public:
    NormalizeObject() : QObject() {}
    static QCString normalizeSignalSlot( const char *signalSlot ) { return QObject::normalizeSignalSlot( signalSlot ); }
};

void LanguageInterfaceImpl::functions( const QString &code, QMap<QString, QString> *functionMap ) const
{
    QString text( code );
    QString func;
    QString body;

    qDebug(text);

    int i = 0;
    int j = 0;
    int k = 0;
    while ( i != -1 ) {
	i = text.find( "::", i );
	if ( i == -1 )
	    break;
	int nl = -1;
	if ( ( nl = text.findRev( "\n", i ) ) != -1 &&
	     text[ nl + 1 ] == ' ' || text[ nl + 1 ] == '\t' ) {
	    i += 2;
	    continue;
	}
	for ( j = i + QString( "::").length(); j < (int)text.length(); ++j ) {
	    if ( text[ j ] != ' ' && text[ j ] != '\t' )
		break;
	}
	if ( j == (int)text.length() - 1 )
	    break;
	k = text.find( ")", j );
	func = text.mid( j, k - j + 1 );
	func = func.stripWhiteSpace();
	func = func.simplifyWhiteSpace();
	func = NormalizeObject::normalizeSignalSlot( func.latin1() );

	i = k;
	i = text.find( "{", i );
	if ( i == -1 )
	    break;
	int open = 0;
	for ( j = i; j < (int)text.length(); ++j ) {
	    if ( text[ j ] == '{' )
		open++;
	    else if ( text[ j ] == '}' )
		open--;
	    if ( !open )
		break;
	}
	body = text.mid( i, j - i + 1 );

	functionMap->insert( func, body );
    }
}

QString LanguageInterfaceImpl::createFunctionStart( const QString &className, const QString &func )
{
    return "void " + className + "::" + func;
}

QStringList LanguageInterfaceImpl::definitions() const
{
}

QStringList LanguageInterfaceImpl::definitionEntries( const QString &definition, QUnknownInterface *designerIface ) const
{
}

void LanguageInterfaceImpl::setDefinitionEntries( const QString &definition, const QStringList &entries, QUnknownInterface *designerIface )
{
}
