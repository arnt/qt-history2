#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qiconview.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qtabwidget.h>
#include <qpoint.h>
#include <qobjectlist.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qfileinfo.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qapplication.h>

#include "mainwindow.h"
#include "startdialogimpl.h"

FileDialog::FileDialog( QWidget *parent )
    : QFileDialog( parent )
{
}

void FileDialog::accept()
{
    emit fileSelected();
}


StartDialog::StartDialog( QWidget *parent, const QStringList& projects,
			  const QString& currentProject,
			  const QString &templatePath )
    : StartDialogBase( parent, 0 )
{
    projectCombo->insertStringList( projects );
    projectCombo->setCurrentText( currentProject );
    newForm = new NewForm();
    newForm->insertTemplates( templateView, templatePath );

    recentFiles.clear();

    initFileOpen();

    connect( recentView, SIGNAL( doubleClicked(QIconViewItem*) ),
	     this, SLOT( accept() ) );
    connect( recentView, SIGNAL( returnPressed(QIconViewItem*) ),
	     this, SLOT( accept() ) );
    connect( fd, SIGNAL( fileSelected() ), this, SLOT( accept() ) );
}

void StartDialog::accept()
{
    hide();
    int tabindex = tabWidget->currentPageIndex();
    QString filename;
    if( !tabindex ) {
	if ( !templateView->currentItem() )
	    return;
	Project *pro = MainWindow::self->findProject( projectCombo->currentText() );
	if ( !pro )
	    return;
	MainWindow::self->setCurrentProject( pro );
	( (NewItem*)templateView->currentItem() )->insert( pro );
    } else if ( tabindex == 1 ) {
	filename = fd->selectedFile();	

    } else if ( tabindex == 2 ) {    	
	filename = recentFiles[recentView->currentItem()->index()];
    }
    if ( tabindex ) {    	
	if ( !filename.isEmpty() ) {
	    QFileInfo fi( filename );
	    if ( fi.extension() == "pro" )	
		MainWindow::self->openProject( filename );
	    else
		MainWindow::self->fileOpen( "", "", filename );
	}
    }
    done( Accepted );
}

void StartDialog::reject()
{
    done( Rejected );
}

void StartDialog::projectChanged( const QString &project )
{
    Project *pro = MainWindow::self->findProject( project );
    if ( !pro )
	return;
    QPtrList<QIconViewItem> allItems = newForm->allViewItems();
    QIconViewItem *i;
    for ( i = allItems.first(); i; i = allItems.next() )
	( (NewItem*)i )->setProject( pro );
    templateView->setCurrentItem( templateView->firstItem() );
    templateView->arrangeItemsInGrid( TRUE );
}

void StartDialog::itemChanged( QIconViewItem *item )
{
    projectLabel->setEnabled( item->rtti() != NewItem::ProjectType );
    projectCombo->setEnabled( item->rtti() != NewItem::ProjectType );
}

void StartDialog::recentItemChanged( QIconViewItem *item )
{
    QString msg( recentFiles[item->index()] );
    QFileInfo f( recentFiles[item->index()] );
    uint s = f.size();
    QString unit( "B" );
    if( s > 1024 ) {
	s /= 1024;
	unit = "KB";
    }
    if ( s > 1024 ) {
	s /= 1024;
	unit = "MB";
    }
    QDateTime dt( f.lastModified() );
    QString date( dt.toString( "MMMM dd hh:mm" ));
    msg = QString( "%1 (%2 %3)  %4" ).arg(msg).arg(s).arg(unit).arg(date);
    fileInfoLabel->setText( msg );
}

void StartDialog::clearFileInfo()
{
    fileInfoLabel->clear();
}

void StartDialog::setRecentlyFiles( QStringList &files )
{
    insertRecentItems( files, FALSE );
}

void StartDialog::setRecentlyProjects( QStringList &projects )
{
    insertRecentItems( projects, TRUE );
}

void StartDialog::insertRecentItems( QStringList &files, bool isProject )
{
    QString iconName = "newform.xpm";
    if ( isProject )
	iconName = "project.xpm";
    QIconViewItem *item;
    QStringList::iterator it = files.begin();
    for( ; it != files.end(); ++it ) {
	QFileInfo fi( *it );
	item = new QIconViewItem( recentView, fi.fileName() );
	recentFiles[recentView->index( item )] = *it;	
	item->setPixmap( PixmapChooser::loadPixmap( iconName ) );
	item->setDragEnabled( FALSE );
    }
}

void StartDialog::initFileOpen()
{
    fd = new FileDialog( this );
    QPoint point( 0, 0 );
    fd->reparent( tab, point );

    QObjectList *l = fd->queryList( "QPushButton" );
    QObjectListIt it( *l );
    QObject *obj;
    while ( (obj = it.current()) != 0 ) {
        ++it;
	((QPushButton*)obj)->hide();
    }
    delete l;

    fd->setSizeGripEnabled ( FALSE );
    tabLayout->addWidget( fd );

    QPluginManager<ImportFilterInterface> manager( IID_ImportFilter, QApplication::libraryPaths(), "/designer" );
    QStringList filterlist;
    filterlist << tr( "Designer Files (*.ui *.pro)" );
    filterlist << tr( "Qt User-Interface Files (*.ui)" );
    filterlist << tr( "QMAKE Project Files (*.pro)" );

    QStringList list = manager.featureList();
    for ( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 )
	    filterlist << *it2;

    filterlist << tr( "All Files (*)" );
    QString filters = filterlist.join( ";;" );
    fd->setFilters( filters );

    fd->show();
}
