#define Q_UUIDIMPL
#include <qcleanuphandler.h>
#include <designerinterface.h>
#include <qfeatures.h>
#include <qwidget.h>
#include "sqlformwizardimpl.h"
#include "mainwindowwizard.h"

class StandardTemplateWizardInterface : public TemplateWizardInterface
{
public:
    StandardTemplateWizardInterface();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;

    void setup( const QString &templ, QWidget *widget, DesignerFormWindow *fw, QUnknownInterface *aIface );

private:
    unsigned long ref;
    QUnknownInterface *appIface;

};

StandardTemplateWizardInterface::StandardTemplateWizardInterface()
    : ref( 0 ), appIface( 0 )
{
}

QStringList StandardTemplateWizardInterface::featureList() const
{
    QStringList list;

    list << "QSqlDialog" << "QDesignerSqlDialog" << "QSqlWidget" << "QDesignerSqlWidget" << "QMainWindow" << "QSqlTable";

    return list;
}

void StandardTemplateWizardInterface::setup( const QString &templ, QWidget *widget, DesignerFormWindow *fw, QUnknownInterface *aIface )
{
#ifndef QT_NO_SQL
    appIface = aIface;
    if ( templ == "QDesignerSqlWidget" ||
	 templ == "QDesignerSqlDialog" ||
	 templ == "QSqlWidget" ||
	 templ == "QSqlDialog" ||
	 templ == "QSqlTable" ) {
	SqlFormWizard *wizard = new SqlFormWizard( appIface, widget, 0, fw, 0, TRUE );
	wizard->exec();
    }
#endif
    if ( templ == "QMainWindow" ) {
	MainWindowWizardBase *wizard = new MainWindowWizardBase( 0, 0, TRUE );
	wizard->exec();
    }
}

QUnknownInterface *StandardTemplateWizardInterface::queryInterface( const QUuid& uuid )
{
    QUnknownInterface *iface = 0;

    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_TemplateWizardInterface )
	iface = (TemplateWizardInterface*)this;

    if ( iface )
	iface->addRef();

    return iface;
}

unsigned long StandardTemplateWizardInterface::addRef()
{
    return ref++;
}

unsigned long StandardTemplateWizardInterface::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

Q_EXPORT_INTERFACE( StandardTemplateWizardInterface )
