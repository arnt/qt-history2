#include "../../../qactioninterface.h"
#include "../../../qcleanuphandler.h"
#include "../../../qdualinterface.h"
#include "sounddialog.h"

#include <qaction.h>
#include <qpopupmenu.h>
#include <qdialog.h>
#include <qapplication.h>
#include <qvariant.h>
#include <qinputdialog.h>
#include <qpainter.h>
#include <qsound.h>
#include <qcheckbox.h>
#include <qlineedit.h>

class QToolBar;

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
    void toggleMenu();
    void toolBarMoves( QToolBar* );
    void toolBarDropped( QToolBar* );

protected:
    QAction* actionTurnOnText;
    void connectNotify( const QCString& iface );

    bool eventFilter( QObject*, QEvent* );

private:
    QClientInterface* cIface;
    QGuardedPtr<SoundDialog> dialog;
};

TestInterface::TestInterface()
{
    cIface = 0;
    dialog = new SoundDialog( 0, 0, FALSE );
    widgets.addCleanUp( dialog );
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
    list << "Set central Widget";
    list << "Toggle Menu";

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
    if ( iface == "PlugMainWindowInterface" ) {
	clientInterface( iface )->requestSignal( SIGNAL(startMovingToolBar(QToolBar*)), this, SLOT(toolBarMoves(QToolBar*)));
	clientInterface( iface )->requestSignal( SIGNAL(endMovingToolBar(QToolBar*)), this, SLOT(toolBarDropped(QToolBar*)));
	clientInterface( iface )->requestEvents( this );
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
    } else if ( actionname == "Set central Widget" ) {
	QAction* a = new QAction( actionname, QIconSet(*unknown_icon), "Set central &Widget", Qt::CTRL + Qt::Key_W, parent, actionname );
	connect( a, SIGNAL(activated()), this, SLOT(selectWidget()) );
	actions.addCleanUp( a );
	return a;
    } else if ( actionname == "Toggle Menu" ) {
	QAction* a = new QAction( actionname, QIconSet(*unknown_icon), "&Toggle Menu", Qt::CTRL + Qt::Key_U, parent, actionname, TRUE );
	connect( a, SIGNAL(activated()), this, SLOT(toggleMenu()) );
	actions.addCleanUp( a );
	return a;
    } else {
	return 0;
    }
}

void TestInterface::openDialog()
{
    if ( dialog )
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
	QString wc = QInputDialog::getText( "Set central widget", "Enter widget class", QLineEdit::Normal, "QWidget", &ok );
	if ( ok ) {
	    ifc->requestSetProperty( "centralWidget", wc );
	}
    }
}

void TestInterface::toggleMenu()
{
    QClientInterface* ifc;
    if ( ( ifc = clientInterface( "PlugMainWindowInterface" ) ) ) {
	QVariant up;
	QClientInterface menuIfc;
	ifc->requestConnection( "PlugMenuInterface", &menuIfc );
	menuIfc.requestProperty( "defaultUp", up );
	menuIfc.requestSetProperty( "defaultUp", !up.toBool() );
    }
}

void TestInterface::toolBarMoves( QToolBar* )
{
    if ( !dialog || !dialog->checkMoving->isChecked() )
	return;
    QSound::play( dialog->fileMoving->text() );
}

void TestInterface::toolBarDropped( QToolBar* )
{
    if ( !dialog || !dialog->checkDocking->isChecked() )
	return;
    QSound::play( dialog->fileDocking->text() );
}

bool TestInterface::eventFilter( QObject* o, QEvent* e )
{
    if ( o->isA( "PlugMainWindow" ) ) {
	switch ( e->type() ) {
	case QEvent::Resize:
	    {
		if ( !dialog || !dialog->checkMaximized->isChecked() )
		    break;
		QWidget* w = (QWidget*)o;
		if ( w->isMaximized() )
		    QSound::play( dialog->fileMaximized->text() );
	    }
	    break;
	case QEvent::Show:
	    {
		if ( !dialog || !dialog->checkRestored->isChecked() )
		    break;
		QShowEvent* se = (QShowEvent*)e;
		if ( se->spontaneous() )
		    QSound::play( dialog->fileRestored->text() );
	    }
	    break;
	case QEvent::Hide:
	    {
		if ( !dialog || !dialog->checkMinimized->isChecked() )
		    break;
		QHideEvent* he = (QHideEvent*)e;
		if ( he->spontaneous() )
		    QSound::play( dialog->fileMinimized->text() );
	    }
	    break;
	default:
	    break;
	}
    }
    return QObject::eventFilter( o, e );
}

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QActionInterface* loadInterface()
{
    return new TestInterface();
}

#if defined(__cplusplus)
}
#endif // __cplusplus

#include "main.moc"
