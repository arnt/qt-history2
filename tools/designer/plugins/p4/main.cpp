#include "../designerinterface.h"
#include "p4.h"

#include <qcleanuphandler.h>
#include <qaction.h>
#include <qapplication.h>

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
    void p4Aware( bool );
    void p4Sync();
    void p4Edit();
    void p4Submit();
    void p4Revert();
    void p4Add();
    void p4Delete();
    void p4Diff();
    void p4MightEdit( bool b, const QString &fn );

private:
    bool aware;

    QGuardedCleanUpHandler<QAction> actions;
    QGuardedPtr<QApplicationInterface> appInterface;

};

P4Interface::P4Interface()
{
    aware = FALSE;
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
    list << "P4 Aware" << "P4 Sync" << "P4 Edit" << "P4 Submit" << "P4 Revert" << "P4 Add" << "P4 Delete" << "P4 Diff";
    return list;
}

QAction* P4Interface::create( const QString& actionname, QObject* parent )
{
    QAction* a = 0;
    if ( actionname == "P4 Aware" ) {
	a = new QAction( actionname, QIconSet(), "A&ware", 0, parent, actionname, TRUE );
	connect( a, SIGNAL( toggled(bool) ), this, SLOT( p4Aware(bool) ) );
    } else if ( actionname == "P4 Sync" ) {
	a = new QAction( actionname, QIconSet(), "&Sync", 0, parent, actionname );
	connect( a, SIGNAL( activated() ), this, SLOT( p4Sync() ) );
    } else if ( actionname == "P4 Edit" ) {
	a = new QAction( actionname, QIconSet(), "&Edit", 0, parent, actionname );
	connect( a, SIGNAL( activated() ), this, SLOT( p4Edit() ) );
    } else if ( actionname == "P4 Submit" ) {
	a = new QAction( actionname, QIconSet(), "&Submit", 0, parent, actionname );
	connect( a, SIGNAL( activated() ), this, SLOT( p4Submit() ) );
    } else if ( actionname == "P4 Revert" ) {
	a = new QAction( actionname, QIconSet(), "&Revert", 0, parent, actionname );
	connect( a, SIGNAL( activated() ), this, SLOT( p4Revert() ) );
    } else if ( actionname == "P4 Add" ) {
	a = new QAction( actionname, QIconSet(), "&Add", 0, parent, actionname );
	connect( a, SIGNAL( activated() ), this, SLOT( p4Add() ) );
    } else if ( actionname == "P4 Delete" ) {
	a = new QAction( actionname, QIconSet(), "&Delete", 0, parent, actionname );
	connect( a, SIGNAL( activated() ), this, SLOT( p4Delete() ) );
    } else if ( actionname == "P4 Diff" ) {
	a = new QAction( actionname, QIconSet(), "Di&ff", 0, parent, actionname );
	connect( a, SIGNAL( activated() ), this, SLOT( p4Diff() ) );
    } else {
	return 0;
    }

    actions.addCleanUp( a );
    return a;
}

QString P4Interface::group( const QString & )
{
    return "P4";
}

void P4Interface::p4Aware( bool on )
{
    aware = on;
}

void P4Interface::p4Sync()
{
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

void P4Interface::p4Revert()
{
}

void P4Interface::p4Add()
{
}

void P4Interface::p4Delete()
{
}

void P4Interface::p4Diff()
{
}

void P4Interface::p4MightEdit( bool b, const QString &s )
{
    if ( !aware || !b || !appInterface )
	return;
    QComponentInterface *mwIface = 0;
    if ( !( mwIface = appInterface->queryInterface( "DesignerMainWindowInterface" ) ) )
	return;
    P4Edit *edit = new P4Edit( s, mwIface, TRUE );
    edit->edit();
}

#include "main.moc"

Q_EXPORT_INTERFACE(ActionInterface, P4Interface)
