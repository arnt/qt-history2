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
#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qlistview.h>

#include "../designerinterface.h"

SqlFormWizard::SqlFormWizard( QComponentInterface *aIface, QWidget *w,
			      QWidget* parent,  const char* name, bool modal, WFlags fl )
    : SqlFormWizardBase( parent, name, modal, fl ), widget( w ), appIface( aIface )
{
    setFinishEnabled( databasePage, FALSE );
    setFinishEnabled( sqlPage, TRUE );

    if ( widget->inherits( "QSqlTable" ) )
	setAppropriate( navigPage, FALSE );

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

void SqlFormWizard::tableSelected( const QString & )
{
    autoPopulate( TRUE );
}

void SqlFormWizard::autoPopulate( bool populate )
{
    listBoxField->clear();
    listBoxSortField->clear();
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
	listBoxSortField->insertStringList( lst );
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

void SqlFormWizard::addSortField()
{
    int i = listBoxSortField->currentItem();
    if ( i == -1 )
	return;
    QString f = listBoxSortField->currentText();
    if ( !f.isEmpty() )
	listBoxSortedField->insertItem( f + " ASC" );
}

void SqlFormWizard::reSortSortField()
{
    int i = listBoxSortedField->currentItem();
    if ( i != -1 ) {
	QString text = listBoxSortedField->currentText();
	if ( text.mid( text.length() - 3 ) == "ASC" )
	    text = text.mid( 0, text.length() - 3 ) + "DESC";
	else if ( text.mid( text.length() - 4 ) == "DESC" )
	    text = text.mid( 0, text.length() - 4 ) + "ASC";
	listBoxSortedField->removeItem( i );
	listBoxSortedField->insertItem( text, i );
	listBoxSortedField->setCurrentItem( i );
    }
}

void SqlFormWizard::removeSortField()
{
    int i = listBoxSortedField->currentItem();
    if ( i != -1 ) {
	listBoxSortedField->removeItem( i );
    }
}

void SqlFormWizard::sortFieldUp()
{
    if ( listBoxSortedField->currentItem() <= 0 ||
	 listBoxSortedField->count() < 2 )
	return;
    int index = listBoxSortedField->currentItem() - 1;
    QListBoxItem *i = listBoxSortedField->item( listBoxSortedField->currentItem() );
    listBoxSortedField->takeItem( i );
    listBoxSortedField->insertItem( i, index );
    listBoxSortedField->setCurrentItem( i );
}

void SqlFormWizard::sortFieldDown()
{
    if ( listBoxSortedField->currentItem() == -1 ||
	 listBoxSortedField->currentItem() == (int)listBoxSortedField->count() - 1 ||
	 listBoxSortedField->count() < 2 )
	return;
    int index = listBoxSortedField->currentItem() + 1;
    QListBoxItem *i = listBoxSortedField->item( listBoxSortedField->currentItem() );
    listBoxSortedField->takeItem( i );
    listBoxSortedField->insertItem( i, index );
    listBoxSortedField->setCurrentItem( i );
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

static QPushButton *create_widget( QWidget *parent, const char *name,
				   const QString &txt, const QRect &r, DesignerMetaDatabaseInterface *mdbIface,
				   DesignerFormInterface *fIface, DesignerWidgetFactoryInterface *wfIface )
{
    QPushButton *pb = (QPushButton*)wfIface->create( "QPushButton", parent, name );
    pb->setText( txt );
    pb->setGeometry( r );
    mdbIface->setPropertyChanged( pb, "text", TRUE );
    mdbIface->setPropertyChanged( pb, "geometry", TRUE );
    fIface->addWidget( pb );
    return pb;
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

    if ( !editFilter->text().isEmpty() ) {
	widget->setProperty( "filter", editFilter->text() );
	mdbIface->setPropertyChanged( widget, "filter", TRUE );
    }

    if ( listBoxSortedField->count() ) {
	QStringList lst;
	for ( uint i = 0; i < listBoxSortedField->count(); ++i )
	    lst << listBoxSortedField->text( i );
	widget->setProperty( "sort", lst );
	mdbIface->setPropertyChanged( widget, "sort", TRUE );
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

    int row = 0;
    const int SPACING = 25;

    for( uint j = 0; j < listBoxSelectedField->count(); j++ ){

	QSqlField* field = tab.field( listBoxSelectedField->text( j ) );
	if ( !field )
	    continue;

	QString labelName = field->name();
	labelName = labelName.mid(0,1).upper() + labelName.mid(1);
	labelName.replace( QRegExp("_"), " " );
	label = (QLabel*)wfIface->create( "QLabel", widget, QString( "label" + labelName ) );
	label->setText( labelName );
	label->setGeometry( SPACING, row+SPACING, SPACING*3, SPACING );
	mdbIface->setPropertyChanged( label, "text", TRUE );
	mdbIface->setPropertyChanged( label, "geometry", TRUE );
	fIface->addWidget( label );

	editorDummy = f->createEditor( widget, field );
	editor = wfIface->create( editorDummy->className(), widget, QString( QString( editorDummy->className() ) + labelName) );
	delete editorDummy;
	editor->setGeometry( SPACING * 5, row+SPACING, SPACING*3, SPACING );
	mdbIface->setPropertyChanged( editor, "geometry", TRUE );
	fIface->addWidget( editor );

	QStringList lst;
	lst << conn << table << field->name();
	mdbIface->setFakeProperty( editor, "database", lst );
	mdbIface->setPropertyChanged( editor, "database", TRUE );

	row += SPACING + 5;

    }

    if ( checkBoxNavig->isChecked() ) {
	if ( checkBoxFirst->isChecked() ) {
	    QPushButton *pb = create_widget( widget, "PushButtonFirst", "|< &First",
					     QRect( SPACING * 10, SPACING, SPACING * 3, SPACING ), mdbIface, fIface, wfIface );
	    mdbIface->addConnection( widget->parentWidget(), pb, "clicked()", widget, "firstRecord()" );
	}
	if ( checkBoxPrev->isChecked() ) {
	    QPushButton *pb = create_widget( widget, "PushButtonPrev", "<< &Prev",
					     QRect( SPACING * 13, SPACING, SPACING * 3, SPACING ), mdbIface, fIface, wfIface );
	    mdbIface->addConnection( widget->parentWidget(), pb, "clicked()", widget, "prevRecord()" );
	}
	if ( checkBoxNext->isChecked() ) {
	    QPushButton *pb = create_widget( widget, "PushButtonNext", "&Next >>",
					     QRect( SPACING * 16, SPACING, SPACING * 3, SPACING ), mdbIface, fIface, wfIface );
	    mdbIface->addConnection( widget->parentWidget(), pb, "clicked()", widget, "nextRecord()" );
	}
	if ( checkBoxLast->isChecked() ) {
	    QPushButton *pb = create_widget( widget, "PushButtonLast", "&Last >|",
					     QRect( SPACING * 19, SPACING, SPACING * 3, SPACING ), mdbIface, fIface, wfIface );
	    mdbIface->addConnection( widget->parentWidget(), pb, "clicked()", widget, "lastRecord()" );
	}
    }

    if ( checkBoxEdit->isChecked() ) {
	if ( checkBoxInsert->isChecked() ) {
	    QPushButton *pb = create_widget( widget, "PushButtonInsert", "&Insert",
					     QRect( SPACING * 10, SPACING *3, SPACING * 3, SPACING ), mdbIface, fIface, wfIface );
	    mdbIface->addConnection( widget->parentWidget(), pb, "clicked()", widget, "insertRecord()" );
	}
	if ( checkBoxUpdate->isChecked() ) {
	    QPushButton *pb = create_widget( widget, "PushButtonUpdate", "&Update",
					     QRect( SPACING * 13, SPACING *3, SPACING * 3, SPACING ), mdbIface, fIface, wfIface );
	    mdbIface->addConnection( widget->parentWidget(), pb, "clicked()", widget, "updateRecord()" );
	}
	if ( checkBoxDelete->isChecked() ) {
	    QPushButton *pb = create_widget( widget, "PushButtonDelete", "&Delete",
					     QRect( SPACING * 16, SPACING *3, SPACING * 3, SPACING ), mdbIface, fIface, wfIface );
	    mdbIface->addConnection( widget->parentWidget(), pb, "clicked()", widget, "deleteRecord()" );
	}
    }

    proIface->closeDatabase( editConnection->text() );

    SqlFormWizardBase::accept();
}
