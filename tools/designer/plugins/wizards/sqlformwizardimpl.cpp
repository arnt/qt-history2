#include "sqlformwizardimpl.h"

/*
 *  Constructs a SqlFormWizard which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The wizard will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal wizard.
 */
SqlFormWizard::SqlFormWizard( QWidget *w, const QValueList<TemplateWizardInterface::DatabaseConnection> &conns,
			      QWidget* parent,  const char* name, bool modal, WFlags fl )
    : SqlFormWizardBase( parent, name, modal, fl ), widget( w ), dbConnections( conns )
{
}

/*
 *  Destroys the object and frees any allocated resources
 */
SqlFormWizard::~SqlFormWizard()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * protected slot
 */
void SqlFormWizard::databaseSelected( const QString & )
{
    qWarning( "SqlFormWizard::databaseSelected( const QString & ) not yet implemented!" );
}
/*
 * protected slot
 */
void SqlFormWizard::tableSelected( const QString & )
{
    qWarning( "SqlFormWizard::tableSelected( const QString & ) not yet implemented!" );
}

