#include <qapplication.h>
#include <qmessagebox.h>

#define QT_ACTIVEX_IMPL
#include "ax1.h"
#include "ax2.h"

class ActiveQtFactory : public QActiveQtFactory
{
public:
    ActiveQtFactory( const QUuid &lib, const QUuid &app )
	: QActiveQtFactory( lib, app )
    {}
    QStringList featureList() const
    {
	QStringList list;
	list << QUuid(CLSID_QAxWidget1);
	list << QUuid(CLSID_QAxWidget2);
	return list;
    }
    QWidget *create( const QString &key, QWidget *parent, const char *name )
    {
	if ( QUuid(key) == CLSID_QAxWidget1 )
	    return new QAxWidget1( parent, name );
	if ( QUuid(key) == CLSID_QAxWidget2 )
	    return new QAxWidget2( parent, name );
	return 0;
    }
    QMetaObject *metaObject( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QAxWidget1 )
	    return QAxWidget1::staticMetaObject();
	if ( QUuid(key) == CLSID_QAxWidget2 )
	    return QAxWidget2::staticMetaObject();
	return 0;
    }
    QUuid interfaceID( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QAxWidget1 )
	    return IID_IQAxWidget1;
	if ( QUuid(key) == CLSID_QAxWidget2 )
	    return IID_IQAxWidget2;
	return QUuid();
    }
    QUuid eventsID( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QAxWidget1 )
	    return IID_IQAxWidget1Events;
	if ( QUuid(key) == CLSID_QAxWidget2 )
	    return IID_IQAxWidget2Events;
	return QUuid();
    }
};

Q_EXPORT_ACTIVEX( ActiveQtFactory, IID_QAxWidget1Lib, IID_QAxWidget1App )

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    bool isActiveX = FALSE;
    for ( int arg = 0; arg < argc; ++arg ) {
	if ( !qstrcmp(argv[arg], "-activex") ) {
	    isActiveX = TRUE;
	    break;
	}
    }
    if ( !isActiveX ) {
	QMessageBox::critical( 0, "Cannot Run stand alone!", "This executable is a server for ActiveX controls.\nIt cannot be run stand alone." );
	return -1;
    }

    return app.exec();
}
