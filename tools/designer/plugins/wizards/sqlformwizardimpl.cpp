#include "sqlformwizardimpl.h"

#include <qlistbox.h>
#include <qwidget.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qlistview.h>
#include <qfeatures.h>

#ifndef QT_NO_SQL
#include <qsqltable.h>
#include <qsqleditorfactory.h>
#include <qsqlindex.h>
#include <qsqlcursor.h>
#endif

SqlFormWizard::SqlFormWizard( QUnknownInterface *aIface, QWidget *w,
			      QWidget* parent, DesignerFormWindow *fw, const char* name, bool modal, WFlags fl )
    : SqlFormWizardBase( parent, name, modal, fl ), widget( w ), appIface( aIface ),
      mode( None )
{
    formWindow = fw;
    setFinishEnabled( databasePage, FALSE );
    setFinishEnabled( sqlPage, TRUE );

    /* set mode of operation */
    if ( widget->inherits( "QSqlTable" ) ) {
	setCaption( "SQL Table Wizard" );
	mode = Table;
	setAppropriate( navigPage, FALSE );
    } else {
	setCaption( "SQL Form Wizard" );
	setAppropriate( tablePropertiesPage, FALSE );
	mode = Form;
    }

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

    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;

    listBoxTable->clear();
    editTable->clear();
    QList<DesignerDatabase> databases = proIface->databaseConnections();
    databases.setAutoDelete( TRUE );
    for ( DesignerDatabase *d = databases.first(); d; d = databases.next() ) {
	if ( d->name() == c  || ( d->name() == "(default)" || d->name().isEmpty() ) && c == "(default)")
	    listBoxTable->insertStringList( d->tables() );
    }
    delete proIface;
}

void SqlFormWizard::tableSelected( const QString & )
{
    autoPopulate( TRUE );
}

void SqlFormWizard::autoPopulate( bool populate )
{
    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;
    QList<DesignerDatabase> databases = proIface->databaseConnections();
    listBoxField->clear();
    listBoxSortField->clear();
    listBoxSelectedField->clear();
    if ( populate ) {
	databases.setAutoDelete( TRUE );
	for ( DesignerDatabase *d = databases.first(); d; d = databases.next() ) {
	    if ( d->name() == editConnection->text() || ( d->name() == "(default)" || d->name().isEmpty() ) && editConnection->text() == "(default)" ) {
		QStringList lst = *d->fields().find( editTable->text() );
		// remove primary index fields, if any
		d->open();
#ifndef QT_NO_SQL
		QSqlCursor tab( editTable->text() );
		QSqlIndex pIdx = tab.primaryIndex();
		for ( uint i = 0; i < pIdx.count(); i++ ) {
		    listBoxField->insertItem( pIdx.field( i )->name() );
		    lst.remove( pIdx.field( i )->name() );
		}
#endif
		d->close();
		listBoxSelectedField->insertStringList( lst );
		listBoxSortField->insertStringList( lst );
	    }
	}
	delete proIface;
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

    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;
    proIface->setupDatabases();
    setupPage1();
    delete proIface;
}

void SqlFormWizard::setupPage1()
{
    if ( !appIface )
	return;

    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;

    listBoxTable->clear();
    listBoxConnection->clear();
    editTable->clear();
    editConnection->clear();
    if ( !widget->parentWidget()->inherits( "FormWindow" ) ) {
	checkBoxEdit->setChecked( FALSE );
	checkBoxNavig->setChecked( FALSE );
    }
    QList<DesignerDatabase> databases = proIface->databaseConnections();
    databases.setAutoDelete( TRUE );
    QStringList lst;
    for ( DesignerDatabase *d = databases.first(); d; d = databases.next() )
	lst << d->name();
    listBoxConnection->insertStringList( lst );
    delete proIface;
}

static QPushButton *create_widget( QWidget *parent, const char *name,
				   const QString &txt, const QRect &r, DesignerFormWindow *fw )
{
    QPushButton *pb = (QPushButton*)fw->create( "QPushButton", parent, name );
    pb->setText( txt );
    pb->setGeometry( r );
    fw->setPropertyChanged( pb, "text", TRUE );
    fw->setPropertyChanged( pb, "geometry", TRUE );
    return pb;
}

void SqlFormWizard::accept()
{
    if ( !appIface )
	return;

#ifndef QT_NO_SQL
    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !widget || !proIface ) {
	SqlFormWizardBase::accept();
	return;
    }

    QString conn = editConnection->text();
    QString table = editTable->text();
    QStringList lst;
    lst << conn << table;

    if ( !conn.isEmpty() && !table.isEmpty() ) {
	formWindow->setProperty( widget, "database", lst );
	formWindow->setPropertyChanged( widget, "database", TRUE );
    }

    if ( !editFilter->text().isEmpty() ) {
	widget->setProperty( "filter", editFilter->text() );
	formWindow->setPropertyChanged( widget, "filter", TRUE );
    }

    if ( listBoxSortedField->count() ) {
	QStringList lst;
	for ( uint i = 0; i < listBoxSortedField->count(); ++i )
	    lst << listBoxSortedField->text( i );
	widget->setProperty( "sort", lst );
	formWindow->setPropertyChanged( widget, "sort", TRUE );
    }

    QList<DesignerDatabase> databases = proIface->databaseConnections();
    databases.setAutoDelete( TRUE );
    DesignerDatabase *database = 0;
    for ( DesignerDatabase *d = databases.first(); d; d = databases.next() ) {
	if ( d->name() == editConnection->text() || ( d->name() == "(default)" || d->name().isEmpty() ) && editConnection->text() == "(default)" ) {
	    database = d;
	    d->open();
	    break;
	}
    }
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

    uint j;
    switch ( mode ) {
    case None:
	break;
    case Form:
	for( j = 0; j < listBoxSelectedField->count(); j++ ){

	    QSqlField* field = tab.field( listBoxSelectedField->text( j ) );
	    if ( !field )
		continue;

	    QString labelName = field->name();
	    labelName = labelName.mid(0,1).upper() + labelName.mid(1);
	    labelName.replace( QRegExp("_"), " " );
	    label = (QLabel*)formWindow->create( "QLabel", widget, QString( "label" + labelName ) );
	    label->setText( labelName );
	    label->setGeometry( SPACING, row+SPACING, SPACING*3, SPACING );
	    formWindow->setPropertyChanged( label, "text", TRUE );
	    formWindow->setPropertyChanged( label, "geometry", TRUE );

	    editorDummy = f->createEditor( widget, field );
	    editor = formWindow->create( editorDummy->className(), widget, QString( QString( editorDummy->className() ) + labelName) );
	    delete editorDummy;
	    editor->setGeometry( SPACING * 5, row+SPACING, SPACING*3, SPACING );
	    formWindow->setPropertyChanged( editor, "geometry", TRUE );

	    QStringList lst;
	    lst << conn << table << field->name();
	    formWindow->setProperty( editor, "database", lst );
	    formWindow->setPropertyChanged( editor, "database", TRUE );

	    row += SPACING + 5;

	}

	if ( checkBoxNavig->isChecked() ) {
	    if ( checkBoxFirst->isChecked() ) {
		QPushButton *pb = create_widget( widget, "PushButtonFirst", "|< &First",
						 QRect( SPACING * 10, SPACING, SPACING * 3, SPACING ), formWindow );
		formWindow->addConnection( pb, "clicked()", widget, "firstRecord()" );
		formWindow->addConnection( widget, "firstRecordAvailable( bool )", pb, "setEnabled( bool )" );
	    }
	    if ( checkBoxPrev->isChecked() ) {
		QPushButton *pb = create_widget( widget, "PushButtonPrev", "<< &Prev",
						 QRect( SPACING * 13, SPACING, SPACING * 3, SPACING ), formWindow );
		formWindow->addConnection( pb, "clicked()", widget, "prevRecord()" );
		formWindow->addConnection( widget, "prevRecordAvailable( bool )", pb, "setEnabled( bool )" );
	    }
	    if ( checkBoxNext->isChecked() ) {
		QPushButton *pb = create_widget( widget, "PushButtonNext", "&Next >>",
						 QRect( SPACING * 16, SPACING, SPACING * 3, SPACING ), formWindow );
		formWindow->addConnection( pb, "clicked()", widget, "nextRecord()" );
		formWindow->addConnection( widget, "nextRecordAvailable( bool )", pb, "setEnabled( bool )" );
	    }
	    if ( checkBoxLast->isChecked() ) {
		QPushButton *pb = create_widget( widget, "PushButtonLast", "&Last >|",
						 QRect( SPACING * 19, SPACING, SPACING * 3, SPACING ), formWindow );
		formWindow->addConnection( pb, "clicked()", widget, "lastRecord()" );
		formWindow->addConnection( widget, "lastRecordAvailable( bool )", pb, "setEnabled( bool )" );
	    }
	}

	if ( checkBoxEdit->isChecked() ) {
	    if ( checkBoxInsert->isChecked() ) {
		QPushButton *pb = create_widget( widget, "PushButtonInsert", "&Insert",
						 QRect( SPACING * 10, SPACING *3, SPACING * 3, SPACING ), formWindow );
		formWindow->addConnection( pb, "clicked()", widget, "insertRecord()" );
	    }
	    if ( checkBoxUpdate->isChecked() ) {
		QPushButton *pb = create_widget( widget, "PushButtonUpdate", "&Update",
						 QRect( SPACING * 13, SPACING *3, SPACING * 3, SPACING ), formWindow );
		formWindow->addConnection( pb, "clicked()", widget, "updateRecord()" );
	    }
	    if ( checkBoxDelete->isChecked() ) {
		QPushButton *pb = create_widget( widget, "PushButtonDelete", "&Delete",
						 QRect( SPACING * 16, SPACING *3, SPACING * 3, SPACING ), formWindow );
		formWindow->addConnection( pb, "clicked()", widget, "deleteRecord()" );
	    }
	}
	break;
    case Table:
	{
	    QSqlTable* sqlTable = ((QSqlTable*)widget);
	    if ( checkBoxReadOnly->isChecked() ) {
		sqlTable->setReadOnly( TRUE );
		formWindow->setPropertyChanged( sqlTable, "readOnly", TRUE );
	    } else {
		if ( checkBoxConfirmInserts->isChecked() ) {
		    sqlTable->setConfirmInsert( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmInsert", TRUE );
		}
		if ( checkBoxConfirmUpdates->isChecked() ) {
		    sqlTable->setConfirmUpdate( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmUpdate", TRUE );
		}
		if ( checkBoxConfirmDeletes->isChecked() ) {
		    sqlTable->setConfirmDelete( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmDelete", TRUE );
		}
		if ( checkBoxConfirmCancels->isChecked() ) {
		    sqlTable->setConfirmCancels( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmCancels", TRUE );
		}
	    }
	    if ( checkBoxSorting->isChecked() ) {
		sqlTable->setSorting( TRUE );
		formWindow->setPropertyChanged( sqlTable, "sorting", TRUE );
	    }

	    QMap<QString, QString> columnFields;
	    sqlTable->setNumCols( listBoxSelectedField->count() ); // no need to change property through mdbIface here, since QSqlTable doesn't offer that through Designer
	    for( j = 0; j < listBoxSelectedField->count(); j++ ){

		QSqlField* field = tab.field( listBoxSelectedField->text( j ) );
		if ( !field )
		    continue;

		QString labelName = field->name();
		labelName = labelName.mid(0,1).upper() + labelName.mid(1);
		labelName.replace( QRegExp("_"), " " );

		((QTable*)widget)->horizontalHeader()->setLabel( j, labelName );

		columnFields.insert( labelName, field->name() );
	    }
	    formWindow->setColumnFields( widget, columnFields );
	    break;
	}
    }

    database->close();
#endif

    SqlFormWizardBase::accept();
    delete formWindow;
    delete proIface;
}
