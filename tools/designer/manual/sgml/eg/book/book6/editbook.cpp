/****************************************************************************
** Form implementation generated from reading ui file '/home/mark/p4/qt/tools/designer/manual/sgml/eg/book/book6/editbook.ui'
**
** Created: Fri Mar 2 11:56:54 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "/home/mark/p4/qt/tools/designer/manual/sgml/eg/book/book6/editbook.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qdatabrowser.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qsqlcursor.h>
#include <qsqldatabase.h>
#include <qsqlform.h>
#include <qsqlrecord.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a EditBookForm which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
EditBookForm::EditBookForm( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "EditBookForm" );
    resize( 540, 287 ); 
    setCaption( tr( "Edit Books" ) );
    EditBookFormLayout = new QVBoxLayout( this ); 
    EditBookFormLayout->setSpacing( 6 );
    EditBookFormLayout->setMargin( 11 );

    BookDataBrowser = new QDataBrowser( this, "BookDataBrowser" );
    QStringList BookDataBrowserSort;
    BookDataBrowserSort << "title ASC";
    BookDataBrowser->setSort( BookDataBrowserSort );
    BookDataBrowserLayout = new QGridLayout( BookDataBrowser ); 
    BookDataBrowserLayout->setSpacing( 6 );
    BookDataBrowserLayout->setMargin( 11 );

    Layout2 = new QGridLayout; 
    Layout2->setSpacing( 6 );
    Layout2->setMargin( 0 );

    labelPrice = new QLabel( BookDataBrowser, "labelPrice" );
    labelPrice->setText( tr( "Price" ) );

    Layout2->addWidget( labelPrice, 1, 0 );

    labelTitle = new QLabel( BookDataBrowser, "labelTitle" );
    labelTitle->setText( tr( "Title" ) );

    Layout2->addWidget( labelTitle, 0, 0 );

    QLineEditTitle = new QLineEdit( BookDataBrowser, "QLineEditTitle" );

    Layout2->addWidget( QLineEditTitle, 0, 1 );

    QLineEditPrice = new QLineEdit( BookDataBrowser, "QLineEditPrice" );

    Layout2->addWidget( QLineEditPrice, 1, 1 );

    BookDataBrowserLayout->addLayout( Layout2, 0, 0 );

    Layout3 = new QHBoxLayout; 
    Layout3->setSpacing( 6 );
    Layout3->setMargin( 0 );

    PushButtonFirst = new QPushButton( BookDataBrowser, "PushButtonFirst" );
    PushButtonFirst->setText( tr( "|< &First" ) );
    Layout3->addWidget( PushButtonFirst );

    PushButtonPrev = new QPushButton( BookDataBrowser, "PushButtonPrev" );
    PushButtonPrev->setText( tr( "<< &Prev" ) );
    Layout3->addWidget( PushButtonPrev );

    PushButtonNext = new QPushButton( BookDataBrowser, "PushButtonNext" );
    PushButtonNext->setText( tr( "&Next >>" ) );
    Layout3->addWidget( PushButtonNext );

    PushButtonLast = new QPushButton( BookDataBrowser, "PushButtonLast" );
    PushButtonLast->setText( tr( "&Last >|" ) );
    Layout3->addWidget( PushButtonLast );

    BookDataBrowserLayout->addLayout( Layout3, 1, 0 );

    Layout6 = new QHBoxLayout; 
    Layout6->setSpacing( 6 );
    Layout6->setMargin( 0 );

    PushButtonInsert = new QPushButton( BookDataBrowser, "PushButtonInsert" );
    PushButtonInsert->setText( tr( "&Insert" ) );
    Layout6->addWidget( PushButtonInsert );

    PushButtonUpdate = new QPushButton( BookDataBrowser, "PushButtonUpdate" );
    PushButtonUpdate->setText( tr( "&Update" ) );
    PushButtonUpdate->setDefault( TRUE );
    Layout6->addWidget( PushButtonUpdate );

    PushButtonDelete = new QPushButton( BookDataBrowser, "PushButtonDelete" );
    PushButtonDelete->setText( tr( "&Delete" ) );
    Layout6->addWidget( PushButtonDelete );

    PushButtonClose = new QPushButton( BookDataBrowser, "PushButtonClose" );
    PushButtonClose->setText( tr( "&Close" ) );
    Layout6->addWidget( PushButtonClose );

    BookDataBrowserLayout->addLayout( Layout6, 2, 0 );
    EditBookFormLayout->addWidget( BookDataBrowser );

    // database support
    QSqlForm* BookDataBrowserForm =  new QSqlForm( this, "BookDataBrowserForm" );
    BookDataBrowserForm->insert( QLineEditTitle, "title" );
    BookDataBrowserForm->insert( QLineEditPrice, "price" );
    BookDataBrowser->setForm( BookDataBrowserForm );





    // signals and slots connections
    connect( PushButtonFirst, SIGNAL( clicked() ), BookDataBrowser, SLOT( first() ) );
    connect( BookDataBrowser, SIGNAL( firstRecordAvailable( bool ) ), PushButtonFirst, SLOT( setEnabled(bool) ) );
    connect( PushButtonPrev, SIGNAL( clicked() ), BookDataBrowser, SLOT( prev() ) );
    connect( BookDataBrowser, SIGNAL( prevRecordAvailable( bool ) ), PushButtonPrev, SLOT( setEnabled(bool) ) );
    connect( PushButtonNext, SIGNAL( clicked() ), BookDataBrowser, SLOT( next() ) );
    connect( BookDataBrowser, SIGNAL( nextRecordAvailable( bool ) ), PushButtonNext, SLOT( setEnabled(bool) ) );
    connect( PushButtonLast, SIGNAL( clicked() ), BookDataBrowser, SLOT( last() ) );
    connect( BookDataBrowser, SIGNAL( lastRecordAvailable( bool ) ), PushButtonLast, SLOT( setEnabled(bool) ) );
    connect( PushButtonInsert, SIGNAL( clicked() ), BookDataBrowser, SLOT( insert() ) );
    connect( PushButtonUpdate, SIGNAL( clicked() ), BookDataBrowser, SLOT( update() ) );
    connect( PushButtonDelete, SIGNAL( clicked() ), BookDataBrowser, SLOT( del() ) );
    connect( BookDataBrowser, SIGNAL( primeInsert(QSqlRecord*) ), this, SLOT( primeInsertBook(QSqlRecord*) ) );
    connect( PushButtonClose, SIGNAL( clicked() ), this, SLOT( accept() ) );
    init();
}

/*  
 *  Destroys the object and frees any allocated resources
 */
EditBookForm::~EditBookForm()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

void EditBookForm::primeInsertBook( QSqlRecord * buffer )
{
    QSqlQuery query;  
    query.exec( "UPDATE sequence SET sequence = sequence + 1 WHERE tablename='book';" );  
    query.exec( "SELECT sequence FROM sequence WHERE tablename='book';" );  
    if ( query.next() ) {  
	buffer->setValue( "id", query.value( 0 ) );  
    }     
}

void EditBookForm::init()
{
}

void EditBookForm::destroy()
{
}

