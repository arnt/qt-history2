#include "sqlformwizardimpl.h"
#include <qlistbox.h>
#include <qwidget.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qsqleditorfactory.h>
#include <qsqlindex.h>
#include <qsqlcursor.h>

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
	DesignerProjectInterface *proIface = (DesignerProjectInterface*)appIface->queryInterface( IID_DesignerProjectInterface );
	if ( !proIface )
	    return;
	QStringList lst = proIface->databaseFieldList( editConnection->text(), editTable->text() );
	// remove primary index fields, if any
	proIface->openDatabase( editConnection->text() );
	QSqlCursor tab( editTable->text() );
	QSqlIndex pIdx = tab.primaryIndex();
	for ( uint i = 0; i < pIdx.count(); i++ ) {
	    listBoxField->insertItem( pIdx.field( i )->name() );
	    lst.remove( pIdx.field( i )->name() );
	}
	proIface->closeDatabase( editConnection->text() );
	listBoxSelectedField->insertStringList( lst );
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

void SqlFormWizard::accept()
{
    DesignerProjectInterface *proIface = (DesignerProjectInterface*)appIface->queryInterface( IID_DesignerProjectInterface );
    if ( !widget || !proIface ) {
	SqlFormWizardBase::accept();
	return;
    }

    proIface->openDatabase( editConnection->text() );
    QSqlCursor tab( editTable->text() );
    int columns = 2;

    QSqlEditorFactory * f = QSqlEditorFactory::defaultFactory();

    QWidget * editor;
    QLabel * label;
    QVBoxLayout * vb = new QVBoxLayout( widget );
    QGridLayout * g  = new QGridLayout( vb );
    g->setMargin( 5 );
    g->setSpacing( 3 );
    int visibleFields = listBoxSelectedField->count();

    if( columns < 1 ) columns = 1;

    int numPerColumn = visibleFields / columns;
    if( (visibleFields % columns) > 0)
	numPerColumn++;

    int col = 0, currentCol = 0;

    for( uint j = 0; j < listBoxSelectedField->count(); j++ ){
	if( col >= numPerColumn ){
	    col = 0;
	    currentCol += 2;
	}

	QSqlField* field = tab.field( listBoxSelectedField->text( j ) );
	if ( !field )
	    continue;

	label = new QLabel( field->name(), widget ); //## make pretty?
	g->addWidget( label, col, currentCol );

	editor = f->createEditor( widget, field );
	g->addWidget( editor, col, currentCol + 1 );
	col++;
    }

    proIface->closeDatabase( editConnection->text() );

    SqlFormWizardBase::accept();
}
