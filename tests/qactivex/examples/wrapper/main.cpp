#include <qapplication.h>
#include <qmessagebox.h>
#include <qactiveqt.h>

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

const IID IID_IQButton = {0x6DA689FB,0x928F,0x423c,{0x86,0x32,0x67,0x8C,0x3D,0x36,0x06,0xDB}};
const IID IID_IQButtonEvents = {0x73A5D03F,0x8ADE,0x4d84,{0x9A,0xE0,0xA9,0x3B,0x4F,0x85,0xA1,0x30}};
const IID CLSID_QButton = {0x23F5012A,0x7333,0x43d3,{0xBC,0xA8,0x83,0x6A,0xAB,0xC6,0x1B,0x4A}};

const IID IID_IQCheckBox = {0x4FD39DD7,0x2DE0,0x43c1,{0xA8,0xC2,0x27,0xC5,0x1A,0x05,0x28,0x10}};
const IID IID_IQCheckBoxEvents = {0xFDB6FFBE,0x56A3,0x4e90,{0x8F,0x4D,0x19,0x84,0x88,0x41,0x8B,0x3A}};
const IID CLSID_QCheckBox = {0x6E795DE9,0x872D,0x43cf,{0xA8,0x31,0x49,0x6E,0xF9,0xD8,0x6C,0x68}};

const IID IID_IQRadioButton = {0x7CC8AE30,0x206C,0x48a3,{0xA0,0x09,0xB0,0xA0,0x88,0x02,0x6C,0x2F}};
const IID IID_IQRadioButtonEvents = {0x73EE4860,0x684C,0x4a66,{0xBF,0x63,0x9B,0x9E,0xFF,0xA0,0xCB,0xE5}};
const IID CLSID_QRadioButton = {0xAFCF78C8,0x446C,0x409a,{0x93,0xB3,0xBA,0x29,0x59,0x03,0x91,0x89}};

const IID IID_IQPushButton = {0x06831CC9,0x59B6,0x436a,{0x95,0x78,0x6D,0x53,0xE5,0xAD,0x03,0xD3}};
const IID IID_IQPushButtonEvents = {0x3CC3F17F,0xEA59,0x4b58,{0xBB,0xD3,0x84,0x2D,0x46,0x71,0x31,0xDD}};
const IID CLSID_QPushButton = {0x2B262458,0xA4B6,0x468b,{0xB7,0xD4,0xCF,0x5F,0xEE,0x0A,0x70,0x92}};

const IID LIBID_QAxLib = {0x3B756301,0x0075,0x4e40,{0x8B,0xE8,0x5A,0x81,0xDE,0x24,0x26,0xB7}};

class ActiveQtFactory : public QActiveQtFactory
{
public:
    ActiveQtFactory( const QUuid &lib, const QUuid &app ) 
	: QActiveQtFactory( lib, app ) 
    {}
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
	if ( QUuid(key) == CLSID_QCheckBox )
	    return new QCheckBox( parent, name );
	if ( QUuid(key) == CLSID_QRadioButton )
	    return new QRadioButton( parent, name );
	if ( QUuid(key) == CLSID_QPushButton )
	    return new QPushButton( parent, name );

	return 0;
    }
    QMetaObject *metaObject( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QButton )
	    return QButton::staticMetaObject();
	if ( QUuid(key) == CLSID_QCheckBox )
	    return QCheckBox::staticMetaObject();
	if ( QUuid(key) == CLSID_QRadioButton )
	    return QRadioButton::staticMetaObject();
	if ( QUuid(key) == CLSID_QPushButton )
	    return QPushButton::staticMetaObject();

	return 0;
    }
    QUuid interfaceID( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QButton )
	    return IID_IQButton;
	if ( QUuid(key) == CLSID_QCheckBox )
	    return IID_IQCheckBox;
	if ( QUuid(key) == CLSID_QRadioButton )
	    return IID_IQRadioButton;
	if ( QUuid(key) == CLSID_QPushButton )
	    return IID_IQPushButton;

	return QUuid();
    }
    QUuid eventsID( const QString &key ) const
    {
	if ( QUuid(key) == CLSID_QButton )
	    return IID_IQButtonEvents;
	if ( QUuid(key) == CLSID_QCheckBox )
	    return IID_IQCheckBoxEvents;
	if ( QUuid(key) == CLSID_QRadioButton )
	    return IID_IQRadioButtonEvents;
	if ( QUuid(key) == CLSID_QPushButton )
	    return IID_IQPushButtonEvents;

	return QUuid();
    }
};

Q_EXPORT_ACTIVEX( ActiveQtFactory, LIBID_QAxLib, "{AB068077-4924-406a-BBAF-42D91C8727DD}" )

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
