#include "tableeditorimpl.h"
#include <qtable.h>
#include "formwindow.h"
#include <qlabel.h>
#include <qcombobox.h>
#include <qheader.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include "pixmapchooser.h"
#include "command.h"
#include <qvaluelist.h>
#include <qtabwidget.h>
#include "project.h"
#include "metadatabase.h"

TableEditor::TableEditor( QWidget* parent,  QWidget *editWidget, FormWindow *fw, const char* name, bool modal, WFlags fl )
    : TableEditorBase( parent, name, modal, fl ), editTable( (QTable*)editWidget ), formWindow( fw )
{
    if ( !editTable->inherits( "QSqlTable" ) ) {
	labelFields->hide();
	comboFields->hide();
	labelTable->hide();
	labelTableValue->hide();
    }
    if ( editTable->inherits( "QSqlTable" ) ) {
	// ## why does this behave weird?
	//	TabWidget->removePage( rows_tab );
	//	rows_tab->hide();
	// ## do this in the meantime...
	TabWidget->setTabEnabled( rows_tab, FALSE );
    }

    labelColumnPixmap->setText( "" );
    labelRowPixmap->setText( "" );

    if ( formWindow->project() && editTable->inherits( "QSqlTable" ) ) {
	QStringList lst = MetaDataBase::fakeProperty( editTable, "database" ).toStringList();
	if ( lst.count() == 2 && !lst[ 0 ].isEmpty() && !lst[ 1 ].isEmpty() ) {
	    QStringList fields;
	    fields << "<no field>";
	    fields += formWindow->project()->databaseFieldList( lst[ 0 ], lst[ 1 ] );
	    comboFields->insertStringList( fields );
	}
	if ( !lst[ 1 ].isEmpty() )
	    labelTableValue->setText( lst[ 1 ] );
    }

    readFromTable();
}

TableEditor::~TableEditor()
{
}

void TableEditor::columnDownClicked()
{
    if ( listColumns->currentItem() == -1 ||
	 listColumns->currentItem() == (int)listColumns->count() - 1 ||
	 listColumns->count() < 2 )
	return;
    saveFieldMap();
    int index = listColumns->currentItem() + 1;
    QListBoxItem *i = listColumns->item( listColumns->currentItem() );
    listColumns->takeItem( i );
    listColumns->insertItem( i, index );
    listColumns->setCurrentItem( i );
    readColumns();
    restoreFieldMap();
    currentColumnChanged( i );
}

void TableEditor::columnTextChanged( const QString &s )
{
    if ( listColumns->currentItem() == -1 )
	return;
    listColumns->changeItem( s, listColumns->currentItem() );
    if ( table->horizontalHeader()->iconSet( listColumns->currentItem() ) )
	table->horizontalHeader()->setLabel( listColumns->currentItem(),
					     *table->horizontalHeader()->iconSet( listColumns->currentItem() ), s );
    else
	table->horizontalHeader()->setLabel( listColumns->currentItem(), s );
}

void TableEditor::columnUpClicked()
{
    if ( listColumns->currentItem() <= 0 ||
	 listColumns->count() < 2 )
	return;
    saveFieldMap();
    int index = listColumns->currentItem() - 1;
    QListBoxItem *i = listColumns->item( listColumns->currentItem() );
    listColumns->takeItem( i );
    listColumns->insertItem( i, index );
    listColumns->setCurrentItem( i );
    readColumns();
    restoreFieldMap();
    currentColumnChanged( i );
}

void TableEditor::currentColumnChanged( QListBoxItem *i )
{
    if ( !i )
	return;
    editColumnText->blockSignals( TRUE );
    editColumnText->setText( i->text() );
    if ( i->pixmap() )
	labelColumnPixmap->setPixmap( *i->pixmap() );
    else
	labelColumnPixmap->setText( "" );
    editColumnText->blockSignals( FALSE );

    if ( editTable->inherits( "QSqlTable" ) ) {
	QString s = *fieldMap.find( listColumns->index( i ) );
	if ( s.isEmpty() )
	    comboFields->setCurrentItem( 0 );
	else if ( comboFields->listBox()->findItem( s ) )
	    comboFields->setCurrentItem( comboFields->listBox()->index( comboFields->listBox()->findItem( s ) ) );
	else
	    comboFields->lineEdit()->setText( s );
    }
}

void TableEditor::currentFieldChanged( const QString &s )
{
    if ( listColumns->currentItem() == -1 )
	return;
    fieldMap.remove( listColumns->currentItem() );
    fieldMap.insert( listColumns->currentItem(), s );
}

void TableEditor::currentRowChanged( QListBoxItem *i )
{
    if ( !i )
	return;
    editRowText->blockSignals( TRUE );
    editRowText->setText( i->text() );
    if ( i->pixmap() )
	labelRowPixmap->setPixmap( *i->pixmap() );
    else
	labelRowPixmap->setText( "" );
    editRowText->blockSignals( FALSE );
}

void TableEditor::deleteColumnClicked()
{
    if ( listColumns->currentItem() == -1 )
	return;
    table->setNumCols( table->numCols() - 1 );
    delete listColumns->item( listColumns->currentItem() );
    readColumns();
    if ( listColumns->firstItem() ) {
	listColumns->setCurrentItem( listColumns->firstItem() );
	listColumns->setSelected( listColumns->firstItem(), TRUE );
    }
}

void TableEditor::deleteRowClicked()
{
    if ( listRows->currentItem() == -1 )
	return;
    table->setNumRows( table->numRows() - 1 );
    delete listRows->item( listRows->currentItem() );
    readRows();
    if ( listRows->firstItem() ) {
	listRows->setCurrentItem( listRows->firstItem() );
	listRows->setSelected( listRows->firstItem(), TRUE );
    }
}

void TableEditor::newColumnClicked()
{
    table->setNumCols( table->numCols() + 1 );
    table->horizontalHeader()->setLabel( table->numCols() - 1, QString::number( table->numCols() ) );
    listColumns->insertItem( QString::number( table->numCols() ) );
    QListBoxItem *i = listColumns->item( listColumns->count() - 1 );
    listColumns->setCurrentItem( i );
    listColumns->setSelected( i, TRUE );
    editColumnText->setFocus();
    editColumnText->selectAll();
}

void TableEditor::newRowClicked()
{
    table->setNumRows( table->numRows() + 1 );
    table->verticalHeader()->setLabel( table->numRows() - 1, QString::number( table->numRows() ) );
    listRows->insertItem( QString::number( table->numRows() ) );
    QListBoxItem *i = listRows->item( listRows->count() - 1 );
    listRows->setCurrentItem( i );
    listRows->setSelected( i, TRUE );
}

void TableEditor::okClicked()
{
    applyClicked();
    accept();
}

void TableEditor::rowDownClicked()
{
    if ( listRows->currentItem() == -1 ||
	 listRows->currentItem() == (int)listRows->count() - 1 ||
	 listRows->count() < 2 )
	return;
    int index = listRows->currentItem() + 1;
    QListBoxItem *i = listRows->item( listRows->currentItem() );
    listRows->takeItem( i );
    listRows->insertItem( i, index );
    listRows->setCurrentItem( i );
    readRows();
}

void TableEditor::rowTextChanged( const QString &s )
{
    if ( listRows->currentItem() == -1 )
	return;
    listRows->changeItem( s, listRows->currentItem() );
    if ( table->verticalHeader()->iconSet( listRows->currentItem() ) )
	table->verticalHeader()->setLabel( listRows->currentItem(),
					     *table->verticalHeader()->iconSet( listRows->currentItem() ), s );
    else
	table->verticalHeader()->setLabel( listRows->currentItem(), s );
}

void TableEditor::rowUpClicked()
{
    if ( listRows->currentItem() <= 0 ||
	 listRows->count() < 2 )
	return;
    int index = listRows->currentItem() - 1;
    QListBoxItem *i = listRows->item( listRows->currentItem() );
    listRows->takeItem( i );
    listRows->insertItem( i, index );
    listRows->setCurrentItem( i );
    readRows();
}

void TableEditor::applyClicked()
{
    QValueList<PopulateTableCommand::Row> rows;
    QValueList<PopulateTableCommand::Column> cols;

    int i = 0;
    for ( i = 0; i < table->horizontalHeader()->count(); ++i ) {
	PopulateTableCommand::Column col;
	col.text = table->horizontalHeader()->label( i );
	if ( table->horizontalHeader()->iconSet( i ) )
	    col.pix = table->horizontalHeader()->iconSet( i )->pixmap();
	col.field = *fieldMap.find( i );
	cols.append( col );
    }
    for ( i = 0; i < table->verticalHeader()->count(); ++i ) {
	PopulateTableCommand::Row row;
	row.text = table->verticalHeader()->label( i );
	if ( table->verticalHeader()->iconSet( i ) )
	    row.pix = table->verticalHeader()->iconSet( i )->pixmap();
	rows.append( row );
    }

    PopulateTableCommand *cmd = new PopulateTableCommand( tr( "Edit Rows and Columns of '%1' " ).arg( editTable->name() ),
							  formWindow, editTable, rows, cols );
    cmd->execute();
    formWindow->commandHistory()->addCommand( cmd );
}

void TableEditor::chooseRowPixmapClicked()
{
    if ( listRows->currentItem() == -1 )
	return;
    QPixmap pix;
    if ( listRows->item( listRows->currentItem() )->pixmap() )
	pix = qChoosePixmap( this, formWindow, *listRows->item( listRows->currentItem() )->pixmap() );
    else
	pix = qChoosePixmap( this, formWindow, QPixmap() );

    if ( pix.isNull() )
	return;

    table->verticalHeader()->setLabel( listRows->currentItem(), pix, table->verticalHeader()->label( listRows->currentItem() ) );
    listRows->changeItem( pix, listRows->currentText(), listRows->currentItem() );
}

void TableEditor::deleteRowPixmapClicked()
{
    if ( listRows->currentItem() == -1 )
	return;
    table->verticalHeader()->setLabel( listRows->currentItem(), QPixmap(), table->verticalHeader()->label( listRows->currentItem() ) );
    listRows->changeItem( listRows->currentText(), listRows->currentItem() );
}

void TableEditor::chooseColPixmapClicked()
{
    if ( listColumns->currentItem() == -1 )
	return;
    QPixmap pix;
    if ( listColumns->item( listColumns->currentItem() )->pixmap() )
	pix = qChoosePixmap( this, formWindow, *listColumns->item( listColumns->currentItem() )->pixmap() );
    else
	pix = qChoosePixmap( this, formWindow, QPixmap() );

    if ( pix.isNull() )
	return;

    table->horizontalHeader()->setLabel( listColumns->currentItem(), pix, table->horizontalHeader()->label( listColumns->currentItem() ) );
    listColumns->changeItem( pix, listColumns->currentText(), listColumns->currentItem() );
}

void TableEditor::deleteColPixmapClicked()
{
    if ( listColumns->currentItem() == -1 )
	return;
    table->horizontalHeader()->setLabel( listColumns->currentItem(), QPixmap(), table->horizontalHeader()->label( listColumns->currentItem() ) );
    listColumns->changeItem( listColumns->currentText(), listColumns->currentItem() );
}

void TableEditor::readFromTable()
{
    QHeader *cols = editTable->horizontalHeader();
    table->setNumCols( cols->count() );
    QMap<QString, QString> columnFields = MetaDataBase::columnFields( editTable );
    for ( int i = 0; i < cols->count(); ++i ) {
	if ( editTable->horizontalHeader()->iconSet( i ) ) {
	    table->horizontalHeader()->setLabel( i, *editTable->horizontalHeader()->iconSet( i ),
						 editTable->horizontalHeader()->label( i ) );
	    listColumns->insertItem( editTable->horizontalHeader()->iconSet( i )->pixmap(),
				     editTable->horizontalHeader()->label( i ) );
	} else {
	    table->horizontalHeader()->setLabel( i, editTable->horizontalHeader()->label( i ) );
	    listColumns->insertItem( editTable->horizontalHeader()->label( i ) );
	}
	QString cf = *columnFields.find( editTable->horizontalHeader()->label( i ) );
	fieldMap.insert( i, cf );
    }

    if ( listColumns->firstItem() ) {
	listColumns->setCurrentItem( listColumns->firstItem() );
	listColumns->setSelected( listColumns->firstItem(), TRUE );
    }

    QHeader *rows = editTable->verticalHeader();
    table->setNumRows( rows->count() );
    for ( int j = 0; j < rows->count(); ++j ) {
	if ( editTable->verticalHeader()->iconSet( j ) ) {
	    table->verticalHeader()->setLabel( j, *editTable->verticalHeader()->iconSet( j ),
					       editTable->verticalHeader()->label( j ) );
	    listRows->insertItem( editTable->verticalHeader()->iconSet( j )->pixmap(),
				  editTable->verticalHeader()->label( j ) );
	} else {
	    table->verticalHeader()->setLabel( j, editTable->verticalHeader()->label( j ) );
	    listRows->insertItem( editTable->verticalHeader()->label( j ) );
	}
    }

    if ( listRows->firstItem() ) {
	listRows->setCurrentItem( listRows->firstItem() );
	listRows->setSelected( listRows->firstItem(), TRUE );
    }
}

void TableEditor::readColumns()
{
    int j = 0;
    for ( QListBoxItem *i = listColumns->firstItem(); i; i = i->next(), ++j ) {
	if ( i->pixmap() )
	    table->horizontalHeader()->setLabel( j, *i->pixmap(), i->text() );
	else
	    table->horizontalHeader()->setLabel( j, i->text() );
    }
}

void TableEditor::readRows()
{
    int j = 0;
    for ( QListBoxItem *i = listRows->firstItem(); i; i = i->next(), ++j ) {
	if ( i->pixmap() )
	    table->verticalHeader()->setLabel( j, *i->pixmap(), i->text() );
	else
	    table->verticalHeader()->setLabel( j, i->text() );
    }
}

void TableEditor::saveFieldMap()
{
    tmpFieldMap.clear();
    for ( QMap<int, QString>::Iterator it = fieldMap.begin(); it != fieldMap.end(); ++it )
	tmpFieldMap.insert( listColumns->item( it.key() ), *it );
}

void TableEditor::restoreFieldMap()
{
    fieldMap.clear();
    for ( QMap<QListBoxItem*, QString>::Iterator it = tmpFieldMap.begin(); it != tmpFieldMap.end(); ++it )
	fieldMap.insert( listColumns->index( it.key() ), *it );
}
