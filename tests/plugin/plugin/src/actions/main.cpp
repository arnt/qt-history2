#include "../../../qactioninterface.h"
#include "../../../qcleanuphandler.h"
#include "../../../qdualinterface.h"
#include "../../../qwidgetfactory.h"

#include <qaction.h>
#include <qpopupmenu.h>
#include <qdialog.h>
#include <qapplication.h>
#include <qvariant.h>
#include <qinputdialog.h>
#include <qpainter.h>

#ifdef _WS_WIN_
#undef LIBEXPORT
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

QCleanUpHandler<QPixmap> pixmaps;

static QPixmap *unknown_icon = 0;

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

    QCleanUpHandler<QAction> actions;
    QCleanUpHandler<QWidget> widgets;
    QStrList queryInterfaceList() const;

public slots:
    void openDialog();
    void toggleText();
    void selectWidget();
    void changed( bool );

protected:
    QAction* actionTurnOnText;
    void connectNotify( const QCString& iface );

private:
    QClientInterface* cIface;
};

TestInterface::TestInterface()
{
    cIface = 0;
}

TestInterface::~TestInterface()
{
    delete cIface;
}

QStringList TestInterface::featureList()
{
    QStringList list;

    list << "Open Dialog";
    list << "Show Text";
    list << "Set central widget";

    return list;
}

QStrList TestInterface::queryInterfaceList() const
{
    QStrList list;

    list.append( "PlugMainWindowInterface" );

    return list;
}

void TestInterface::connectNotify( const QCString& iface )
{
    qDebug( "Handshake!" );
    if ( iface == "PlugMainWindowInterface" ) {
	clientInterface( iface )->requestSignal( SIGNAL(usesTextLabelChanged(bool)), this, SLOT(changed(bool)));
    }
}

QAction* TestInterface::create( const QString& actionname, QObject* parent )
{
    if ( !unknown_icon ) {
	unknown_icon = new QPixmap( 22, 22 );
	unknown_icon->fill( white );

	QPainter paint( unknown_icon );
	paint.setPen( black );
	paint.drawText( 0, 0, 22, 22, AlignHCenter | AlignVCenter, "?" );
	paint.end();

	pixmaps.addCleanUp( unknown_icon );
    }
    if ( actionname == "Open Dialog" ) {
	QAction* a = new QAction( actionname, QIconSet(*unknown_icon), "Open &dialog", Qt::CTRL + Qt::Key_D, parent, actionname );
	connect( a, SIGNAL(activated()), this, SLOT(openDialog()) );
	actions.addCleanUp( a );
	return a;
    } else if ( actionname == "Show Text" ) {
	actionTurnOnText = new QAction( actionname, QIconSet(*unknown_icon), "&Show Text", Qt::CTRL + Qt::Key_T, parent, actionname, TRUE );
	connect( actionTurnOnText, SIGNAL(activated()), this, SLOT(toggleText()) );
	actions.addCleanUp( actionTurnOnText );
	return actionTurnOnText;
    } else if ( actionname == "Set central widget" ) {
	QAction* a = new QAction( actionname, QIconSet(*unknown_icon), "Set central &widget", Qt::CTRL + Qt::Key_W, parent, actionname );
	connect( a, SIGNAL(activated()), this, SLOT(selectWidget()) );
	actions.addCleanUp( a );
	return a;
    } else {
	return 0;
    }
}

void TestInterface::openDialog()
{
    QVariant obj;
    QClientInterface* ifc;
    if ( (  ifc = clientInterface( "PlugMainWindowInterface" ) ) )
	ifc->requestProperty( "mainWindow", obj );
    QDialog* dialog = new QDialog( 0, 0, FALSE );
    widgets.addCleanUp( dialog );
    dialog->show();
}

void TestInterface::toggleText()
{
    QClientInterface* ifc; 
    if ( ( ifc = clientInterface( "PlugMainWindowInterface" ) ) ) {
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

void TestInterface::selectWidget()
{
    QClientInterface* ifc;
    if ( ( ifc = clientInterface( "PlugMainWindowInterface" ) ) ) {
	bool ok = FALSE;
	QString wc = QInputDialog::getText( "Set central widget", "Enter widget class", "QWidget", &ok );
	if ( ok ) {
	    ifc->requestSetProperty( "centralWidget", wc );
	}
    }
}

void TestInterface::changed( bool on )
{
    qDebug( "Something changed!" );
}

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QActionInterface* loadInterface()
{
    return new TestInterface();
}

LIBEXPORT bool onConnect( QApplication* theApp )
{
    qDebug("Action-Plugin: I've been loaded by %p", theApp );
    return TRUE;
}

LIBEXPORT bool onDisconnect( QApplication* theApp )
{
    qDebug("Action-Plugin: I've been unloaded by %p", theApp);

    return TRUE;
}

#if defined(__cplusplus)
}
#endif // __cplusplus

#include "main.moc"
