#include <qspinbox.h>

void BookForm::init()
{

}

void BookForm::destroy()
{

}

void BookForm::editClicked()
{
    EditBookForm *dialog = new EditBookForm( this, "Edit Book Form", TRUE );
    QSqlCursor cur( "book" );
    dialog->BookDataBrowser->setSqlCursor( &cur );
    dialog->BookDataBrowser->setFilter( BookDataTable->filter() );
    dialog->BookDataBrowser->setSort(QSqlIndex::fromStringList(
	BookDataTable->sort(), &cur ) );
    dialog->BookDataBrowser->refresh();
    int i = BookDataTable->currentRow();
    if ( i == -1 ) i = 0; // Always use the first row
    dialog->BookDataBrowser->seek( i );
    dialog->exec();
    delete dialog;
    BookDataTable->refresh();
}

void BookForm::connectClicked()
{
    bool ok = FALSE;
    ConnectDialog* dialog = new ConnectDialog( this, "Connect", TRUE );
    dialog->editDatabase->setText( "book" );
    if ( dialog->exec() == QDialog::Accepted ) {
	QSqlDatabase::removeDatabase( QSqlDatabase::defaultConnection );
	QSqlDatabase* db = QSqlDatabase::addDatabase( dialog->comboDriver->currentText() );
	db->setDatabaseName( dialog->editDatabase->text() );
	db->setUserName( dialog->editUsername->text() );
	db->setPassword( dialog->editPassword->text() );
	db->setHostName( dialog->editHostname->text() );
	db->setPort( dialog->portSpinBox->value() );
	if ( !db->open() ) {
	    //## warning?
	    ok= FALSE;
	} else
	    ok = TRUE;
    } 
    if ( !ok ) {
	editButton->setEnabled( FALSE ); 
	BookDataTable->setSqlCursor( 0 );
	AuthorDataTable->setSqlCursor( 0 );
    }  else {
	editButton->setEnabled( TRUE );
	QSqlCursor* authorCursor = new QSqlCursor( "author" );
	AuthorDataTable->setSqlCursor( authorCursor, FALSE, TRUE );
	QSqlCursor* bookCursor = new QSqlCursor( "book" );	
	BookDataTable->setSqlCursor( bookCursor, FALSE, TRUE );
	AuthorDataTable->refresh( QDataTable::RefreshAll );
	BookDataTable->refresh( QDataTable::RefreshAll );	
    }
    delete dialog;
}

void BookForm::newCurrentAuthor( QSqlRecord * author )
{
    BookDataTable->setFilter( "authorid=" + author->value( "id" ).toString() );
    BookDataTable->refresh();
}

void BookForm::primeInsertAuthor( QSqlRecord * buffer )
{
    QSqlQuery q;
    q.exec( "UPDATE sequence SET sequence = sequence + 1 WHERE tablename='author';" );
    q.exec( "SELECT sequence FROM sequence WHERE tablename='author';" );
    if ( q.next() ) {
	buffer->setValue( "id", q.value( 0 ) );
    }
}

