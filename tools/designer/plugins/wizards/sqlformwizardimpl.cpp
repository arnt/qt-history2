#include "sqlformwizardimpl.h"
#include <qlistbox.h>
#include <qlineedit.h>

SqlFormWizard::SqlFormWizard( QWidget *w, const QValueList<TemplateWizardInterface::DatabaseConnection> &conns,
			      QWidget* parent,  const char* name, bool modal, WFlags fl )
    : SqlFormWizardBase( parent, name, modal, fl ), widget( w ), dbConnections( conns )
{
    for ( QValueList<TemplateWizardInterface::DatabaseConnection>::Iterator it = dbConnections.begin();
	  it != dbConnections.end(); ++it )
	listBoxConnection->insertItem( (*it).connection );
    //setNextEnabled( databasePage, FALSE );
    setFinishEnabled( databasePage, FALSE );
    setFinishEnabled( populatePage, TRUE );
}

SqlFormWizard::~SqlFormWizard()
{
    // no need to delete child widgets, Qt does it all for us
}

void SqlFormWizard::connectionSelected( const QString &c )
{
    for ( QValueList<TemplateWizardInterface::DatabaseConnection>::Iterator it = dbConnections.begin();
	  it != dbConnections.end(); ++it ) {
	if ( (*it).connection == c ) {
	    listBoxTable->clear();
	    editTable->clear();
	    listBoxTable->insertStringList( (*it).tables );
	}
    }
}

void SqlFormWizard::tableSelected( const QString &t )
{
    //setNextEnabled( databasePage, !t.isEmpty() );
}

