#include <qapplication.h>
#include <qmessagebox.h>

#include "ax1.h"
#include "ax2.h"

#include "tmp/iid_i.c"

class QActiveQtFactory : public QActiveQtFactoryInterface 
{
public:
    QActiveQtFactory() {}
    Q_REFCOUNT
	QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface )
    {
	*iface = 0;
	if ( iid == IID_QUnknown )
	    *iface = this;
	else if ( iid == IID_QFeatureList )
	    *iface = this;
	else if ( iid == IID_QActiveQtFactory )
	    *iface = this;
	else
	    return QE_NOINTERFACE;
	addRef();
	return QS_OK;
    }
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
	else if ( QUuid(key) == CLSID_QAxWidget2 )
	    return new QAxWidget2( parent, name );
	return 0;
    }
    QMetaObject *metaObject( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QAxWidget1 )
	    return QAxWidget1::staticMetaObject();
	else if ( QUuid(key) == CLSID_QAxWidget2 )
	    return QAxWidget2::staticMetaObject();
	return 0;
    }
    QUuid interfaceID( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QAxWidget1 )
	    return IID_IQAxWidget1;
	else if ( QUuid(key) == CLSID_QAxWidget2 )
	    return IID_IQAxWidget2;
	return QUuid();
    }
    QUuid eventsID( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QAxWidget1 )
	    return DIID_IQAxWidget1Events;
	else if ( QUuid(key) == CLSID_QAxWidget2 )
	    return DIID_IQAxWidget2Events;
	return QUuid();
    }
    QUuid typeLibID() const
    {
	return LIBID_multipleaxLib;
    }
    QUuid appID() const
    {
	return QUuid( "{05828915-AD1C-47ab-AB96-D6AD1E25F0E2}" );
    }
};

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( QActiveQtFactory )
}

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
