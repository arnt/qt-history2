#include "projectsettingsinterfaceimpl.h"
#include "projectsettings.h"

ProjectSettingsInterfaceImpl::ProjectSettingsInterfaceImpl( QUnknownInterface *outer )
    : parent( outer ),
      ref( 0 ),
      settingsTab( 0 )
{
}

ulong ProjectSettingsInterfaceImpl::addRef()
{
    return parent ? parent->addRef() : ref++;
}

ulong ProjectSettingsInterfaceImpl::release()
{
    if ( parent )
	return parent->release();
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

ProjectSettingsInterface::ProjectSettings *ProjectSettingsInterfaceImpl::projectSetting()
{
    if ( !settingsTab ) {
	settingsTab = new CppProjectSettings( 0 );
	settingsTab->hide();
    }
    ProjectSettings *pf = 0;
    pf = new ProjectSettings;
    pf->tab = settingsTab;
    pf->title = "C++";
    pf->receiver = pf->tab;
    pf->init_slot = SLOT( reInit( QUnknownInterface * ) );
    pf->accept_slot = SLOT( save( QUnknownInterface * ) );
    return pf;
}

QStringList ProjectSettingsInterfaceImpl::projectSettings() const
{
    return QStringList();
}

void ProjectSettingsInterfaceImpl::connectTo( QUnknownInterface * )
{
}

void ProjectSettingsInterfaceImpl::deleteProjectSettingsObject( ProjectSettings *pf )
{
    delete pf;
}

QRESULT ProjectSettingsInterfaceImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    if ( parent )
	return parent->queryInterface( uuid, iface );

    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_ProjectSettings )
	*iface = (ProjectSettingsInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

