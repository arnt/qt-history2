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
#include <qregexp.h>

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
    listBoxField->clear();
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
    if ( !appIface )
	return;

    DesignerProjectInterface *proIface = (DesignerProjectInterface*)appIface->queryInterface( IID_DesignerProjectInterface );
    DesignerMetaDatabaseInterface *mdbIface = (DesignerMetaDatabaseInterface*)appIface->queryInterface( IID_DesignerMetaDatabaseInterface );
    DesignerWidgetFactoryInterface *wfIface = (DesignerWidgetFactoryInterface*)appIface->queryInterface( IID_DesignerWidgetFactoryInteface );
    DesignerFormInterface *fIface = (DesignerFormInterface*)appIface->queryInterface( IID_DesignerFormInterface );
    if ( !widget || !proIface || !mdbIface || !wfIface || !fIface ) {
	SqlFormWizardBase::accept();
	return;
    }

    QString conn = editConnection->text();
    QString table = editTable->text();
    QStringList lst;
    lst << conn << table;

    if ( !conn.isEmpty() && !table.isEmpty() ) {
	mdbIface->setFakeProperty( widget, "database", lst );
	mdbIface->setPropertyChanged( widget, "database", TRUE );
    }

    proIface->openDatabase( editConnection->text() );
    QSqlCursor tab( editTable->text() );
    int columns = 2;

    QSqlEditorFactory * f = QSqlEditorFactory::defaultFactory();

    QWidget * editorDummy;
    QWidget * editor;
    QLabel * label;

    int visibleFields = listBoxSelectedField->count();
    int numPerColumn = visibleFields / columns;
    if( (visibleFields % columns) > 0)
	numPerColumn++;

    int row = 0, col = 0;
    const int SPACING = 25;

    for( uint j = 0; j < listBoxSelectedField->count(); j++ ){

	QSqlField* field = tab.field( listBoxSelectedField->text( j ) );
	if ( !field )
	    continue;

	QString labelName = field->name();
	labelName = labelName.mid(0,1).upper() + labelName.mid(1);
	labelName.replace( QRegExp("_"), " " );
	label = (QLabel*)wfIface->create( "QLabel", widget, QString( "label" + field->name() ) );
	label->setText( labelName );
	label->setGeometry( col+SPACING, row+SPACING, SPACING*3, SPACING );
	mdbIface->setPropertyChanged( label, "text", TRUE );
	mdbIface->setPropertyChanged( label, "geometry", TRUE );
	fIface->addWidget( label );

	editorDummy = f->createEditor( widget, field );
	editor = wfIface->create( editorDummy->className(), widget, QString( QString( editorDummy->className() ) + field->name()) );
	delete editorDummy;
	editor->setGeometry( col+ SPACING * 5, row+SPACING, SPACING*3, SPACING );
	mdbIface->setPropertyChanged( editor, "geometry", TRUE );
	fIface->addWidget( editor );

	QStringList lst;
	lst << conn << table << field->name();
	mdbIface->setFakeProperty( editor, "database", lst );
	mdbIface->setPropertyChanged( editor, "database", TRUE );
	
	row += 25;

    }


    proIface->closeDatabase( editConnection->text() );

    SqlFormWizardBase::accept();
}
