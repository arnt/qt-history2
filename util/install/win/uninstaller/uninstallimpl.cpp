#include "uninstallimpl.h"
#include <qsettings.h>
#include <qmessagebox.h>

UninstallDlgImpl::UninstallDlgImpl( QWidget* parent, const char* name, bool modal, WFlags f )
    : UninstallDlg( parent, name, modal, f )
{
}

UninstallDlgImpl::~UninstallDlgImpl()
{
}

void UninstallDlgImpl::cleanRegistry()
{
    cleanRegistryHelper( "/Trolltech/Qt" );
    cleanRegistryHelper( "/Trolltech/Qt Designer" );
    cleanRegistryHelper( "/Trolltech/Qt Assistant" );
    cleanRegistryHelper( "/Trolltech/Qt Linguist" );
}

void UninstallDlgImpl::cleanRegistryHelper( const QString& key )
{
    QSettings settings;
    QStringList::Iterator it;
    QStringList keys = settings.subkeyList( key );
    for ( it = keys.begin(); it != keys.end(); ++it ) {
        cleanRegistryHelper( key + "/" + *it );
    }
    QStringList entries = settings.entryList( key );
    for ( it = entries.begin(); it != entries.end(); ++it ) {
        settings.removeEntry( key + "/" + *it );
    }
    settings.removeEntry( key + "/." );
}
