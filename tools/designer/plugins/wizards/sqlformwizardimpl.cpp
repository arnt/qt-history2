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
    autoPopulate( TRUE );
}

void SqlFormWizard::autoPopulate( bool populate )
{
    listBoxSelectedField->clear();
    if ( populate ) {
	//## prime listBoxSelectedField with selected table fields
    }
}

void SqlFormWizard::fieldDown()
{
    if ( listBoxSelectedField->currentItem() == -1 ||
	 listBoxSelectedField->currentItem() == (int)listBoxSelectedField->count() - 1 ||
	 listBoxSelectedField->count() < 2 )
	return;
    int index = listBoxSelectedField->currentItem() + 1;
    QListBoxItem *i = listBoxSelectedField->item( listBoxSelectedField->currentItem() );
    listBoxSelectedField->takeItem( i );
    listBoxSelectedField->insertItem( i, index );
    listBoxSelectedField->setCurrentItem( i );
}

void SqlFormWizard::fieldUp()
{
    if ( listBoxSelectedField->currentItem() <= 0 ||
	 listBoxSelectedField->count() < 2 )
	return;
    int index = listBoxSelectedField->currentItem() - 1;
    QListBoxItem *i = listBoxSelectedField->item( listBoxSelectedField->currentItem() );
    listBoxSelectedField->takeItem( i );
    listBoxSelectedField->insertItem( i, index );
    listBoxSelectedField->setCurrentItem( i );
}

void SqlFormWizard::removeField()
{
    int i = listBoxSelectedField->currentItem();
    if ( i != -1 ) {
	listBoxField->insertItem( listBoxSelectedField->currentText() );
	listBoxSelectedField->removeItem( i );
    }
}

void SqlFormWizard::addField()
{
    int i = listBoxField->currentItem();
    if ( i == -1 )
	return;
    QString f = listBoxField->currentText();
    if ( !f.isEmpty() )
	listBoxSelectedField->insertItem( f );
    listBoxField->removeItem( i );
}
