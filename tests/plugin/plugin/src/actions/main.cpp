#include "../../../qactioninterface.h"
#include "../../../qcleanuphandler.h"
#include "../../../qapplicationinterfaces.h"

#include <qaction.h>
#include <qpopupmenu.h>
#include <qdialog.h>
#include <qapplication.h>
#include <qvariant.h>

#ifdef _WS_WIN_
#undef LIBEXPORT
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

class TestInterface : public QObject, public QActionInterface
{
    Q_OBJECT
public:
    TestInterface();
    ~TestInterface();
    QString name() { return "Test Actionplugin"; }
    QString description() { return "Test implementation of the QActionInterface"; }
    QString author() { return "vohi"; }

    QStringList featureList();
    QAction* create( const QString &actionname, QObject* parent = 0 );

    QCleanUpHandler<QAction>* actions;
    QStrList queryInterfaceList() const;

public slots:
    void onOpenModalDialog();
    void turnOnText();

protected:
    QAction* actionTurnOnText;

private:
    QClientInterface* cIface;
};

TestInterface::TestInterface()
{
    cIface = 0;
    actions = new QCleanUpHandler<QAction>;
}

TestInterface::~TestInterface()
{
    delete cIface;
    delete actions;
}

QStringList TestInterface::featureList()
{
    QStringList list;

    list << "Open Dialog";
    list << "Show Text";

    return list;
}

QStrList TestInterface::queryInterfaceList() const
{
    QStrList list;

    list.append( "PlugMainWindowInterface" );

    return list;
}

QAction* TestInterface::create( const QString& actionname, QObject* parent )
{
    if ( actionname == "Open Dialog" ) {
	QAction* a = new QAction( actionname, QIconSet(), "Open &dialog", Qt::CTRL + Qt::Key_D, parent, actionname, FALSE );
	connect( a, SIGNAL(activated()), this, SLOT(onOpenModalDialog()) );
	actions->addCleanUp( a );
	return a;
    } else if ( actionname == "Show Text" ) {
	actionTurnOnText = new QAction( actionname, QIconSet(), "&Show Text", Qt::CTRL + Qt::Key_T, parent, actionname, TRUE );
	connect( actionTurnOnText, SIGNAL(activated()), this, SLOT(turnOnText()) );
	actions->addCleanUp( actionTurnOnText );
	return actionTurnOnText;
    } else 
	return 0;
}

void TestInterface::onOpenModalDialog()
{
    QVariant obj;
    QClientInterface* ifc;
    if ( (  ifc = clientInterface( "PlugMainWindowInterface" ) ) )
	ifc->requestProperty( "mainWindow", obj );
    QDialog dialog( (QWidget*)obj.toUInt(), 0, TRUE );
    dialog.show();
}

void TestInterface::turnOnText()
{
    QClientInterface* ifc; 
    if ( ( ifc = clientInterface("PlugMainWindowInterface") ) ) {
	QVariant onOff;
	ifc->requestProperty( "usesTextLabel", onOff );
	bool on = onOff.toBool();
	if ( !on ) {
	    actionTurnOnText->setText( "Hide Text" );
	    actionTurnOnText->setMenuText( "&Hide Text" );
	} else {
	    actionTurnOnText->setText( "Show Text" );
	    actionTurnOnText->setMenuText( "&Show Text" );
	}
	ifc->requestSetProperty( "usesTextLabel", !on );
    }
}

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QActionInterface* loadInterface()
{
    return new TestInterface();
}

LIBEXPORT bool onConnect( QApplication* myapp )
{
    qDebug("I've been loaded by %p");
    return TRUE;
}

LIBEXPORT bool onDisconnect( QApplication* myapp )
{
    if ( myapp ) {
        qDebug("I've been unloaded by %p", myapp);
    } else {
	qDebug("I've been unloaded on application destruction" );
    }
    return TRUE;
}

#if defined(__cplusplus)
}
#endif // __cplusplus

#include "main.moc"
