#include "tableeditorimpl.h"
#include <qtable.h>
#include "formwindow.h"
#include <qlabel.h>
#include <qcombobox.h>
#include <qheader.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include "pixmapchooser.h"

TableEditor::TableEditor( QWidget* parent,  QWidget *editWidget, FormWindow *fw, const char* name, bool modal, WFlags fl )
    : TableEditorBase( parent, name, modal, fl ), editTable( (QTable*)editWidget ), formWindow( fw )
{
    if ( !editTable->inherits( "QSqlTable" ) ) {
	labelFields->hide();
	comboFields->hide();
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
    int index = listColumns->currentItem() + 1;
    QListBoxItem *i = listColumns->item( listColumns->currentItem() );
    listColumns->takeItem( i );
    listColumns->insertItem( i, index );
    listColumns->setCurrentItem( i );
    readColumns();
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
    int index = listColumns->currentItem() - 1;
    QListBoxItem *i = listColumns->item( listColumns->currentItem() );
    listColumns->takeItem( i );
    listColumns->insertItem( i, index );
    listColumns->setCurrentItem( i );
    readColumns();
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

    // ### field stuff
}

void TableEditor::currentFieldChanged( const QString & )
{
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
    }

    if ( listColumns->firstItem() ) {
	listColumns->setCurrentItem( listColumns->firstItem() );
	listColumns->setSelected( listColumns->firstItem(), TRUE );
    }

    QHeader *rows = editTable->verticalHeader();
    table->setNumRows( rows->count() );
    for ( int i = 0; i < rows->count(); ++i ) {
	if ( editTable->verticalHeader()->iconSet( i ) ) {
	    table->verticalHeader()->setLabel( i, *editTable->verticalHeader()->iconSet( i ),
					       editTable->verticalHeader()->label( i ) );
	    listRows->insertItem( editTable->verticalHeader()->iconSet( i )->pixmap(),
				  editTable->verticalHeader()->label( i ) );
	} else {
	    table->verticalHeader()->setLabel( i, editTable->verticalHeader()->label( i ) );
	    listRows->insertItem( editTable->verticalHeader()->label( i ) );
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
