#include "resultwindow.h"
#include <qstringlist.h>
#include <qlistbox.h>
#include <qsqlcursor.h>
#include <qsqlfield.h>
#include <qsqlindex.h>
#include <qdatatable.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qmessagebox.h>

ResultWindow::ResultWindow ( QWidget * parent, const char * name, WFlags f )
    : SqlBrowseWindowBase(parent, name, f)
{
    db = QSqlDatabase::database();
    QStringList fil = db->tables();
    tableList->insertStringList( fil );
    connect( execButton,SIGNAL(clicked()), this, SLOT(slotExec()));
//     connect( dataGrid, SIGNAL( currentChanged(const QSqlRecord*)),
//	     SLOT( newSelection(const QSqlRecord*)));
    dataGrid->setReadOnly( FALSE );
    dataGrid->setSorting( TRUE );
    dataGrid->setConfirmEdits( TRUE );
    dataGrid->setConfirmCancels( TRUE );
}

ResultWindow::~ResultWindow()
{

}

void ResultWindow::slotExec()
{
    sql.setName( tableList->currentText() );
    sql.setSort( sql.primaryIndex() );
    dataGrid->setSqlCursor( &sql, TRUE );
    dataGrid->refresh( QDataTable::RefreshAll );
}

void ResultWindow::newSelection( const QSqlRecord* /*fields*/ )
{
//     QString cap;
//     for ( uint i = 0; i < fields->count(); ++i ) {
//	cap += fields->displayLabel( fields->fieldName(i) ).leftJustify(20) + ":" +
//	       fields->value(i).toString().rightJustify(30) + "\n";
//     }
//     currentRecordEdit->setText( cap );
}
