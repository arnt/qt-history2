#include "sqlformwizardimpl.h"
#include <qlistbox.h>
#include <qwidget.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include <qeditorfactory.h> //###remove
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qsqleditorfactory.h>

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

    connect( checkBoxAutoPopulate, SIGNAL( toggled(bool) ), this, SLOT( autoPopulate(bool) ) );

    listBoxField->insertItem("test1"); //##
    listBoxField->insertItem("test2"); //##
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
    //## prime listBoxField with selected table fields
    autoPopulate( TRUE );
}

void SqlFormWizard::autoPopulate( bool populate )
{
    listBoxSelectedField->clear();
    if ( populate ) {
	for ( uint i = 0; i < listBoxField->count(); ++i )
	    listBoxSelectedField->insertItem( listBoxField->item(i)->text() );
    }
}

void SqlFormWizard::fieldDown()
{
}

void SqlFormWizard::fieldUp()
{
}

void SqlFormWizard::removeField()
{
    int i = listBoxSelectedField->currentItem();
    if ( i != -1 )
	listBoxSelectedField->removeItem( i );
}

void SqlFormWizard::addField()
{
    QString f = listBoxField->currentText();
    if ( !f.isEmpty() )
	listBoxSelectedField->insertItem( f );
}
