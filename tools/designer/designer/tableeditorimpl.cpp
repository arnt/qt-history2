#include "tableeditorimpl.h"
#include <qtable.h>
#include "formwindow.h"
#include <qlabel.h>
#include <qcombobox.h>

/*
 *  Constructs a TableEditor which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
TableEditor::TableEditor( QWidget* parent,  QWidget *editWidget, FormWindow *fw, const char* name, bool modal, WFlags fl )
    : TableEditorBase( parent, name, modal, fl ), editTable( (QTable*)editWidget ), formWindow( fw )
{
    if ( !editTable->inherits( "QSqlTable" ) ) {
	labelFields->hide();
	comboFields->hide();
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
TableEditor::~TableEditor()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * protected slot
 */
void TableEditor::columnDownClicked()
{
    qWarning( "TableEditor::columnDownClicked() not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::columnTextChanged( const QString & )
{
    qWarning( "TableEditor::columnTextChanged( const QString & ) not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::columnUpClicked()
{
    qWarning( "TableEditor::columnUpClicked() not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::currentColumnChanged( QListBoxItem * )
{
    qWarning( "TableEditor::currentColumnChanged( QListBoxItem * ) not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::currentFieldChanged( const QString & )
{
    qWarning( "TableEditor::currentFieldChanged( const QString & ) not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::currentRowChanged( QListBoxItem * )
{
    qWarning( "TableEditor::currentRowChanged( QListBoxItem * ) not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::deleteColumnClicked()
{
    qWarning( "TableEditor::deleteColumnClicked() not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::deleteRowClicked()
{
    qWarning( "TableEditor::deleteRowClicked() not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::newColumnClicked()
{
    qWarning( "TableEditor::newColumnClicked() not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::newRowClicked()
{
    qWarning( "TableEditor::newRowClicked() not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::okClicked()
{
    qWarning( "TableEditor::okClicked() not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::rowDownClicked()
{
    qWarning( "TableEditor::rowDownClicked() not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::rowTextChanged( const QString & )
{
    qWarning( "TableEditor::rowTextChanged( const QString & ) not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::rowUpClicked()
{
    qWarning( "TableEditor::rowUpClicked() not yet implemented!" );
}
/*
 * protected slot
 */
void TableEditor::applyClicked()
{
    qWarning( "TableEditor::applyClicked() not yet implemented!" );
}

void TableEditor::chooseRowPixmapClicked()
{
}

void TableEditor::deleteRowPixmapClicked()
{
}

void TableEditor::chooseColPixmapClicked()
{
}

void TableEditor::deleteColPixmapClicked()
{
}
