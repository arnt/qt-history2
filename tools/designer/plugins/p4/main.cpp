#include "../designerinterface.h"
#include "p4.h"

#include <qcleanuphandler.h>
#include <qaction.h>
#include <qapplication.h>

class QToolBar;

class P4Interface : public QObject, public ActionInterface
{
    Q_OBJECT

public:
    P4Interface();
    ~P4Interface();

    QString name() { return "P4 Integration"; }
    QString description() { return "Integrates P4 Source Control into the Qt Designer"; }
    QString author() { return "Trolltech"; }

    bool connectNotify( QApplication* a );

    QStringList featureList();
    QAction* create( const QString &actionname, QObject* parent = 0 );
    QString group( const QString &actionname );

private slots:
    void p4Edit();
    void p4Submit();
    void p4MightEdit( bool b, const QString &fn );

private:
    QGuardedCleanUpHandler<QAction> actions;
    QGuardedPtr<QApplicationInterface> appInterface;

};

P4Interface::P4Interface()
{
}

P4Interface::~P4Interface()
{
}

bool P4Interface::connectNotify( QApplication* theApp )
{
    if ( !theApp )
	return FALSE;

    appInterface = theApp->requestApplicationInterface();
    if ( !appInterface )
	return FALSE;

    QComponentInterface *mwIface = 0;
    QComponentInterface *fwIface = 0;
    if ( ( mwIface = appInterface->queryInterface( "DesignerMainWindowInterface" ) ) &&
	 ( fwIface = mwIface->queryInterface( "DesignerFormWindowInterface" ) ) )
	fwIface->requestConnect( SIGNAL( modificationChanged( bool, const QString & ) ), 
				 this, SLOT( p4MightEdit( bool, const QString & ) ) );

    return TRUE;
}

QStringList P4Interface::featureList()
{
    QStringList list;
    list << "P4 Edit" << "P4 Submit";
    return list;
}

QAction* P4Interface::create( const QString& actionname, QObject* parent )
{
    if ( actionname == "P4 Edit" ) {
	QAction* a = new QAction( actionname, QIconSet(), "P4 &Edit", 0, parent, actionname );
	connect( a, SIGNAL( activated() ), this, SLOT( p4Edit() ) );
	actions.addCleanUp( a );
	return a;
    } else if ( actionname == "P4 Submit" ) {
	QAction* a = new QAction( actionname, QIconSet(), "P4 &Submit", 0, parent, actionname );
	connect( a, SIGNAL( activated() ), this, SLOT( p4Submit() ) );
	actions.addCleanUp( a );
	return a;
    } else {
	return 0;
    }
}

QString P4Interface::group( const QString & )
{
    return "P4";
}

void P4Interface::p4Edit()
{
    if ( !appInterface )
	return;
    QComponentInterface *mwIface = 0;
    QComponentInterface *fwIface = 0;
    if ( !( mwIface = appInterface->queryInterface( "DesignerMainWindowInterface" ) ) ||
	 !( fwIface = mwIface->queryInterface( "DesignerFormWindowInterface" ) ) )
	return;

    P4Edit *edit = new P4Edit( fwIface->requestProperty( "fileName" ).toString().latin1(), mwIface, FALSE );
    edit->edit();
}

void P4Interface::p4Submit()
{
    if ( !appInterface )
	return;
    QComponentInterface *mwIface = 0;
    QComponentInterface *fwIface = 0;
    if ( !( mwIface = appInterface->queryInterface( "DesignerMainWindowInterface" ) ) ||
	 !( fwIface = mwIface->queryInterface( "DesignerFormWindowInterface" ) ) )
	return;
    qDebug( "P4Interface::p4Submit %s", fwIface->requestProperty( "dileName" ).toString().latin1() );
}

void P4Interface::p4MightEdit( bool b, const QString &s )
{
    if ( !b || !appInterface )
	return;
    QComponentInterface *mwIface = 0;
    if ( !( mwIface = appInterface->queryInterface( "DesignerMainWindowInterface" ) ) )
	return;
    P4Edit *edit = new P4Edit( s, mwIface, TRUE );
    edit->edit();
}

#include "main.moc"

Q_EXPORT_INTERFACE(ActionInterface, P4Interface)
