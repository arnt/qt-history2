#include "../../designer/actioniface.h"
#include "../../designer/designerappiface.h"

#include <qcleanuphandler.h>
#include <qapplicationinterface.h>
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
    DesignerMainWindowInterface *mwIface = 0;
    DesignerFormWindowInterface *fwIface = 0;
    if ( !( mwIface = (DesignerMainWindowInterface*)appInterface->requestInterface( "DesignerMainWindowInterface" ) ) ||
	 !( fwIface = (DesignerFormWindowInterface*)mwIface->requestInterface( "DesignerFormWindowInterface" ) ) )
	return;
    qDebug( "P4Interface::p4Edit %s", fwIface->requestProperty( "fileName" ).toString().latin1() );
}

void P4Interface::p4Submit()
{
    if ( !appInterface )
	return;
    DesignerMainWindowInterface *mwIface = 0;
    DesignerFormWindowInterface *fwIface = 0;
    if ( !( mwIface = (DesignerMainWindowInterface*)appInterface->requestInterface( "DesignerMainWindowInterface" ) ) ||
	 !( fwIface = (DesignerFormWindowInterface*)mwIface->requestInterface( "DesignerFormWindowInterface" ) ) )
	return;
    qDebug( "P4Interface::p4Submit %s", fwIface->requestProperty( "dileName" ).toString().latin1() );
}

#include "main.moc"

QtExportInterface(ActionInterface, P4Interface)
