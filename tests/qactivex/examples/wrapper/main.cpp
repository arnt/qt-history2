#include <qapplication.h>
#include <qmessagebox.h>
#include <qactiveqt.h>

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

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
	list << QUuid(CLSID_QButton);
	list << QUuid(CLSID_QCheckBox);
	list << QUuid(CLSID_QRadioButton);
	list << QUuid(CLSID_QPushButton);
	return list;
    }
    QWidget *create( const QString &key, QWidget *parent, const char *name )
    {
	if ( QUuid(key) == CLSID_QButton)
	    return new QButton( parent, name );
	else if ( QUuid(key) == CLSID_QCheckBox )
	    return new QCheckBox( parent, name );
	else if ( QUuid(key) == CLSID_QRadioButton )
	    return new QRadioButton( parent, name );
	else if ( QUuid(key) == CLSID_QPushButton )
	    return new QPushButton( parent, name );

	return 0;
    }
    QMetaObject *metaObject( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QButton )
	    return QButton::staticMetaObject();
	else if ( QUuid(key) == CLSID_QCheckBox )
	    return QCheckBox::staticMetaObject();
	else if ( QUuid(key) == CLSID_QRadioButton )
	    return QRadioButton::staticMetaObject();
	else if ( QUuid(key) == CLSID_QPushButton )
	    return QPushButton::staticMetaObject();

	return 0;
    }
    QUuid interfaceID( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QButton )
	    return IID_IQButton;
	else if ( QUuid(key) == CLSID_QCheckBox )
	    return IID_IQCheckBox;
	else if ( QUuid(key) == CLSID_QRadioButton )
	    return IID_IQRadioButton;
	else if ( QUuid(key) == CLSID_QPushButton )
	    return IID_IQPushButton;

	return QUuid();
    }
    QUuid eventsID( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QButton )
	    return DIID_IQButtonEvents;
	else if ( QUuid(key) == CLSID_QCheckBox )
	    return DIID_IQCheckBoxEvents;
	else if ( QUuid(key) == CLSID_QRadioButton )
	    return DIID_IQRadioButtonEvents;
	else if ( QUuid(key) == CLSID_QPushButton )
	    return DIID_IQPushButtonEvents;

	return QUuid();
    }
    QUuid typeLibID() const
    {
	return QUuid(LIBID_QAxLib);
    }
    QUuid appID() const
    {
	return QUuid("{AB068077-4924-406a-BBAF-42D91C8727DD}");
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
