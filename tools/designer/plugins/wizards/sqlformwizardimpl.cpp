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

#include "../designerinterface.h"

SqlFormWizard::SqlFormWizard( QComponentInterface *aIface, QWidget *w,
			      QWidget* parent,  const char* name, bool modal, WFlags fl )
    : SqlFormWizardBase( parent, name, modal, fl ), widget( w ), appIface( aIface )
{
    setFinishEnabled( databasePage, FALSE );
    setFinishEnabled( populatePage, TRUE );

    connect( checkBoxAutoPopulate, SIGNAL( toggled(bool) ), this, SLOT( autoPopulate(bool) ) );
    setupPage1();
}

SqlFormWizard::~SqlFormWizard()
{
    // no need to delete child widgets, Qt does it all for us
}

void SqlFormWizard::connectionSelected( const QString &c )
{
    if ( !appIface )
	return;

    DesignerProjectInterface *proIface = (DesignerProjectInterface*)appIface->queryInterface( IID_DesignerProjectInterface );
    if ( !proIface )
	return;

    listBoxTable->clear();
    editTable->clear();
    listBoxTable->insertStringList( proIface->databaseTableList( c ) );
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

void SqlFormWizard::setupDatabaseConnections()
{
    if ( !appIface )
	return;

    DesignerMainWindowInterface *mwIface = (DesignerMainWindowInterface*)appIface->queryInterface( IID_DesignerMainWindowInterface );
    if ( !mwIface )
	return;
    mwIface->editDatabaseConnections();
    setupPage1();
}

void SqlFormWizard::setupPage1()
{
    if ( !appIface )
	return;

    DesignerProjectInterface *proIface = (DesignerProjectInterface*)appIface->queryInterface( IID_DesignerProjectInterface );
    if ( !proIface )
	return;

    listBoxTable->clear();
    listBoxConnection->clear();
    editTable->clear();
    editConnection->clear();

    QStringList lst = proIface->databaseConnectionList();
    listBoxConnection->insertStringList( lst );
}
