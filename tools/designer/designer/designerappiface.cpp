#include <qinitguid.h>
#include <qcomponentinterface.h>
#include "designerappiface.h"
#include "mainwindow.h"
#include "formwindow.h"
#include "formlist.h"
#include "propertyeditor.h"
#include "hierarchyview.h"
#include <qapplication.h>
#include <qobjectlist.h>
#include <qstatusbar.h>

/*
 * Application Interface
*/
DesignerApplicationInterfaceImpl::DesignerApplicationInterfaceImpl()
: ref( 0 )
{
}

QUnknownInterface *DesignerApplicationInterfaceImpl::queryInterface( const QGuid &guid )
{
    QUnknownInterface *iface = 0;
    if ( guid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)(QComponentInterface*)this;
    else if ( guid == IID_QComponentInterface )
	iface = (QComponentInterface*)this;
    else if ( guid == IID_DesignerStatusBarInterface )
	iface = new DesignerStatusBarInterfaceImpl( this );
    else if ( guid == IID_DesignerFormListInterface )
	iface = new DesignerFormListInterfaceImpl( this );
    else if ( guid == IID_DesignerFormInterface )
	iface = new DesignerFormInterfaceImpl( 0, this );
    else if ( guid == IID_DesignerWidgetListInterface )
	iface = new DesignerWidgetListInterfaceImpl( this );
    else if ( guid == IID_DesignerWidgetInterface )
	iface = new DesignerWidgetInterfaceImpl( 0, this );
    else if ( guid == IID_DesignerMainWindowInterface )
	iface = new DesignerMainWindowInterfaceImpl( this );

    if ( iface )
	iface->addRef();
    return iface;
}

unsigned long DesignerApplicationInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerApplicationInterfaceImpl::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QString DesignerApplicationInterfaceImpl::name() const
{
    return "Qt Designer";
}

QString DesignerApplicationInterfaceImpl::description() const
{
    return "GUI Editor for the Qt Toolkit";
}

QString DesignerApplicationInterfaceImpl::version() const
{
    return "1.1";
}

QString DesignerApplicationInterfaceImpl::author() const
{
    return "Trolltech";
}





DesignerMainWindowInterfaceImpl::DesignerMainWindowInterfaceImpl( QUnknownInterface *ai )
    : DesignerMainWindowInterface(), appIface( ai )
{
    QWidget *mw = qApp ? qApp->mainWidget() : 0;
    if ( mw && mw->inherits( "MainWindow" ) )
	mainWindow = (MainWindow*)mw;
    else
	mainWindow = 0;

    appIface->addRef();
}

DesignerMainWindowInterfaceImpl::~DesignerMainWindowInterfaceImpl()
{
}

QUnknownInterface *DesignerMainWindowInterfaceImpl::queryInterface( const QGuid &guid )
{
    return appIface->queryInterface( guid );
}

unsigned long DesignerMainWindowInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerMainWindowInterfaceImpl::release()
{
    if ( !--ref ) {
	appIface->release();
	delete this;
	return 0;
    }

    return ref;
}

void DesignerMainWindowInterfaceImpl::fileNew()
{
    if ( !mainWindow )
	return;
    mainWindow->fileNew();
}

void DesignerMainWindowInterfaceImpl::fileNewProject()
{
    if ( !mainWindow )
	return;
    mainWindow->fileNewProject();
}

void DesignerMainWindowInterfaceImpl::fileCloseProject()
{
    if ( !mainWindow )
	return;
    mainWindow->fileCloseProject();
}

void DesignerMainWindowInterfaceImpl::fileOpen()
{
    if ( !mainWindow )
	return;
    mainWindow->fileOpen();
}

bool DesignerMainWindowInterfaceImpl::fileSave()
{
    if ( !mainWindow )
	return FALSE;
    return mainWindow->fileSave();
}

bool DesignerMainWindowInterfaceImpl::fileSaveAs()
{
    if ( !mainWindow )
	return FALSE;
    return mainWindow->fileSaveAs();
}

void DesignerMainWindowInterfaceImpl::fileSaveAll()
{
    if ( !mainWindow )
	return;
    mainWindow->fileSaveAll();
}

void DesignerMainWindowInterfaceImpl::fileCreateTemplate()
{
    if ( !mainWindow )
	return;
    mainWindow->fileCreateTemplate();
}

void DesignerMainWindowInterfaceImpl::editUndo()
{
    if ( !mainWindow )
	return;
    mainWindow->editUndo();
}

void DesignerMainWindowInterfaceImpl::editRedo()
{
    if ( !mainWindow )
	return;
    mainWindow->editRedo();
}

void DesignerMainWindowInterfaceImpl::editCut()
{
    if ( !mainWindow )
	return;
    mainWindow->editCut();
}

void DesignerMainWindowInterfaceImpl::editCopy()
{
    if ( !mainWindow )
	return;
    mainWindow->editCopy();
}

void DesignerMainWindowInterfaceImpl::editPaste()
{
    if ( !mainWindow )
	return;
    mainWindow->editPaste();
}

void DesignerMainWindowInterfaceImpl::editDelete()
{
    if ( !mainWindow )
	return;
    mainWindow->editDelete();
}

void DesignerMainWindowInterfaceImpl::editSelectAll()
{
    if ( !mainWindow )
	return;
    mainWindow->editSelectAll();
}

void DesignerMainWindowInterfaceImpl::editLower()
{
    if ( !mainWindow )
	return;
    mainWindow->editLower();
}

void DesignerMainWindowInterfaceImpl::editRaise()
{
    if ( !mainWindow )
	return;
    mainWindow->editRaise();
}

void DesignerMainWindowInterfaceImpl::editAdjustSize()
{
    if ( !mainWindow )
	return;
    mainWindow->editAdjustSize();
}

void DesignerMainWindowInterfaceImpl::editLayoutHorizontal()
{
    if ( !mainWindow )
	return;
    mainWindow->editLayoutHorizontal();
}

void DesignerMainWindowInterfaceImpl::editLayoutVertical()
{
    if ( !mainWindow )
	return;
    mainWindow->editLayoutVertical();
}

void DesignerMainWindowInterfaceImpl::editLayoutHorizontalSplit()
{
    if ( !mainWindow )
	return;
    mainWindow->editLayoutHorizontalSplit();
}

void DesignerMainWindowInterfaceImpl::editLayoutVerticalSplit()
{
    if ( !mainWindow )
	return;
    mainWindow->editLayoutVerticalSplit();
}

void DesignerMainWindowInterfaceImpl::editLayoutGrid()
{
    if ( !mainWindow )
	return;
    mainWindow->editLayoutGrid();
}

void DesignerMainWindowInterfaceImpl::editLayoutContainerHorizontal()
{
    if ( !mainWindow )
	return;
    mainWindow->editLayoutContainerHorizontal();
}

void DesignerMainWindowInterfaceImpl::editLayoutContainerVertical()
{
    if ( !mainWindow )
	return;
    mainWindow->editLayoutContainerVertical();
}

void DesignerMainWindowInterfaceImpl::editLayoutContainerGrid()
{
    if ( !mainWindow )
	return;
    mainWindow->editLayoutContainerGrid();
}

void DesignerMainWindowInterfaceImpl::editBreakLayout()
{
    if ( !mainWindow )
	return;
    mainWindow->editBreakLayout();
}

void DesignerMainWindowInterfaceImpl::editAccels()
{
    if ( !mainWindow )
	return;
    mainWindow->editAccels();
}

void DesignerMainWindowInterfaceImpl::editSlots()
{
    if ( !mainWindow )
	return;
    mainWindow->editSlots();
}

void DesignerMainWindowInterfaceImpl::editConnections()
{
    if ( !mainWindow )
	return;
    mainWindow->editConnections();
}

void DesignerMainWindowInterfaceImpl::editFormSettings()
{
    if ( !mainWindow )
	return;
    mainWindow->editFormSettings();
}

void DesignerMainWindowInterfaceImpl::editProjectSettings()
{
    if ( !mainWindow )
	return;
    mainWindow->editProjectSettings();
}

void DesignerMainWindowInterfaceImpl::editDatabaseConnections()
{
    if ( !mainWindow )
	return;
    mainWindow->editDatabaseConnections();
}

void DesignerMainWindowInterfaceImpl::editPreferences()
{
    if ( !mainWindow )
	return;
    mainWindow->editPreferences();
}

void DesignerMainWindowInterfaceImpl::previewForm()
{
    if ( !mainWindow )
	return;
    mainWindow->previewForm();
}

void DesignerMainWindowInterfaceImpl::previewForm( const QString &s )
{
    if ( !mainWindow )
	return;
    mainWindow->previewForm( s );
}

void DesignerMainWindowInterfaceImpl::windowPropertyEditor( bool b )
{
    if ( !mainWindow )
	return;
    mainWindow->windowPropertyEditor( b );
}

void DesignerMainWindowInterfaceImpl::windowHierarchyView( bool b )
{
    if ( !mainWindow )
	return;
    mainWindow->windowHierarchyView( b );
}

void DesignerMainWindowInterfaceImpl::windowFormList( bool b )
{
    if ( !mainWindow )
	return;
    mainWindow->windowFormList( b );
}

void DesignerMainWindowInterfaceImpl::windowActionEditor( bool b )
{
    if ( !mainWindow )
	return;
    mainWindow->windowActionEditor( b );
}

void DesignerMainWindowInterfaceImpl::toolsCustomWidget()
{
    if ( !mainWindow )
	return;
    mainWindow->toolsCustomWidget();
}

void DesignerMainWindowInterfaceImpl::helpContents()
{
    if ( !mainWindow )
	return;
    mainWindow->helpContents();
}

void DesignerMainWindowInterfaceImpl::helpManual()
{
    if ( !mainWindow )
	return;
    mainWindow->helpManual();
}

void DesignerMainWindowInterfaceImpl::helpAbout()
{
    if ( !mainWindow )
	return;
    mainWindow->helpAbout();
}

void DesignerMainWindowInterfaceImpl::helpAboutQt()
{
    if ( !mainWindow )
	return;
    mainWindow->helpAboutQt();
}






/*
 * StatusBar Interface
*/
DesignerStatusBarInterfaceImpl::DesignerStatusBarInterfaceImpl( QUnknownInterface *ai )
: DesignerStatusBarInterface(), appIface( ai ), statusBar( 0 ), ref( 0 )
{
    QWidget *mw = qApp ? qApp->mainWidget() : 0;
    if ( mw && mw->inherits( "QMainWindow" ) )
	statusBar = ((QMainWindow*)mw)->statusBar();

    appIface->addRef();
}

QUnknownInterface *DesignerStatusBarInterfaceImpl::queryInterface( const QGuid& guid )
{
    return appIface->queryInterface( guid );
}

unsigned long DesignerStatusBarInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerStatusBarInterfaceImpl::release()
{
    if ( !--ref ) {
	appIface->release();
	delete this;
	return 0;
    }

    return ref;
}

void DesignerStatusBarInterfaceImpl::setMessage( const QString &text, int ms )
{
    if ( statusBar )
	statusBar->message( text, ms );
}

void DesignerStatusBarInterfaceImpl::clear()
{
    if ( statusBar )
	statusBar->clear();
}

/*
 * FormList Interface
*/
DesignerFormListInterfaceImpl::DesignerFormListInterfaceImpl( QUnknownInterface *ai )
: DesignerFormListInterface(), appIface( ai ), formList( 0 ), listIterator( 0 ), ref( 0 )
{
    QWidget *mw = qApp ? qApp->mainWidget() : 0;
    if ( mw && mw->inherits( "MainWindow" ) )
	formList = ((MainWindow*)mw)->formlist();

    appIface->addRef();
}

DesignerFormListInterfaceImpl::~DesignerFormListInterfaceImpl()
{
    delete listIterator;
}

QUnknownInterface *DesignerFormListInterfaceImpl::queryInterface( const QGuid& guid )
{
    return appIface->queryInterface( guid );
}

unsigned long DesignerFormListInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerFormListInterfaceImpl::release()
{
    if ( !--ref ) {
	appIface->release();
	delete this;
	return 0;
    }
    return ref;;
}

QString DesignerFormListInterfaceImpl::text( DesignerFormInterface *form, int col ) const
{
    QString formname = form->property( "name" ).toString();
    QListViewItemIterator it( formList );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 0 ) == formname )
	    return item->text( col );
    }
    return QString::null;
}

void DesignerFormListInterfaceImpl::setText( DesignerFormInterface *form, int col, const QString& s )
{
    QString formname = form->property( "name" ).toString();
    QListViewItemIterator it( formList );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 0 ) == formname ) {
	    item->setText( col, s );
	    return;
	}
    }
}

const QPixmap* DesignerFormListInterfaceImpl::pixmap( DesignerFormInterface *form, int col ) const
{
    QString formname = form->property( "name" ).toString();
    QListViewItemIterator it( formList );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 0 ) == formname )
	    return item->pixmap( col );
    }
    return 0;
}

void DesignerFormListInterfaceImpl::setPixmap( DesignerFormInterface *form, int col, const QPixmap& pix )
{
    QString formname = form->property( "name" ).toString();
    QListViewItemIterator it( formList );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 0 ) == formname ) {
	    item->setPixmap( col, pix );
	    return;
	}
    }
}

uint DesignerFormListInterfaceImpl::count() const
{
    uint c = 0;
    QListViewItemIterator it( formList );
    while ( it.current() ) {
	++it;
	++c;
    }
    return c;
}

DesignerFormInterface *DesignerFormListInterfaceImpl::reset()
{
    delete listIterator;
    listIterator = new QListViewItemIterator( formList );

    DesignerFormInterface *iface = new DesignerFormInterfaceImpl( (FormListItem*)listIterator->current(), appIface );
    iface->addRef();

    return iface;
}

DesignerFormInterface *DesignerFormListInterfaceImpl::current()
{
    if ( !listIterator )
	return reset();

    DesignerFormInterface *iface = new DesignerFormInterfaceImpl( (FormListItem*)listIterator->current(), appIface );
    iface->addRef();

    return iface;
}

DesignerFormInterface *DesignerFormListInterfaceImpl::next()
{
    if ( !listIterator )
	reset();

    ++(*listIterator);
    return current();
}

DesignerFormInterface *DesignerFormListInterfaceImpl::prev()
{
    if ( !listIterator )
	reset();

    --(*listIterator);
    return current();
}

bool DesignerFormListInterfaceImpl::newForm()
{
    MainWindow *mw = (MainWindow*)qApp->mainWidget();
    if ( !mw )
	return FALSE;

    qDebug( "Not yet implemented" );

    return TRUE;
}

bool DesignerFormListInterfaceImpl::loadForm()
{
    MainWindow *mw = (MainWindow*)qApp->mainWidget();
    if ( !mw )
	return FALSE;

    qDebug( "Not yet implemented" );

    return TRUE;
}

bool DesignerFormListInterfaceImpl::saveAll() const
{
    QListViewItemIterator it( formList );
    while ( it.current() ) {
	FormWindow* fw = ((FormListItem*)it.current())->formWindow();
	++it;
	fw->save( "Dudeldi" );
    }

    return TRUE;
}

void DesignerFormListInterfaceImpl::closeAll() const
{
    QListViewItemIterator it( formList );
    while ( it.current() ) {
	FormWindow* fw = ((FormListItem*)it.current())->formWindow();
	++it;
	fw->close( TRUE );
    }
}

bool DesignerFormListInterfaceImpl::connect( const char * signal, QObject *receiver, const char *slot )
{
    return formList ? formList->connect( formList, signal, receiver, slot ) : FALSE;
}

/*
 * DesignerFormWindowInterface
*/
DesignerFormInterfaceImpl::DesignerFormInterfaceImpl( FormListItem *fw, QUnknownInterface *ai )
    : DesignerFormInterface(), item( fw ), appIface( ai ), ref( 0 )
{
    appIface->addRef();

    if ( !item ) {
	FormList *fl = 0;
	QWidget *mw = qApp ? qApp->mainWidget() : 0;
	if ( mw && mw->inherits( "MainWindow" ) )
	    fl = ((MainWindow*)mw)->formlist();
	item = fl ? (FormListItem*)fl->currentItem() : 0;
    }
}

QUnknownInterface *DesignerFormInterfaceImpl::queryInterface( const QGuid& guid )
{
    return appIface->queryInterface( guid );
}

unsigned long DesignerFormInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerFormInterfaceImpl::release()
{
    if ( !--ref ) {
	appIface->release();
	delete this;
	return 0;
    }

    return ref;
}

QVariant DesignerFormInterfaceImpl::property( const QCString& p )
{
    return ( item && item->formWindow() ) ? item->formWindow()->property( p ) : QVariant();
}

bool DesignerFormInterfaceImpl::setProperty( const QCString& p, const QVariant& v )
{
    return ( item && item->formWindow() ) ? item->formWindow()->setProperty( p, v ) : FALSE;
}

void DesignerFormInterfaceImpl::save() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->save( "Dudeldi" );
}

void DesignerFormInterfaceImpl::close() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->close( TRUE );
}

void DesignerFormInterfaceImpl::undo() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->undo();
}

void DesignerFormInterfaceImpl::redo() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->redo();
}

bool DesignerFormInterfaceImpl::connect( const char *signal, QObject *receiver, const char *slot )
{
    FormWindow *fw = 0;
    if ( item )
	fw = item->formWindow();

    return fw ? fw->connect( fw, signal, receiver, slot ) : FALSE;
}

/*
 * DesignerWidgetListInterface implementation
 */
DesignerWidgetListInterfaceImpl::DesignerWidgetListInterfaceImpl( QUnknownInterface *ai )
: DesignerWidgetListInterface(), appIface( ai ), listIterator( 0 )
{
    appIface->addRef();

    QWidget *mw = qApp ? qApp->mainWidget() : 0;
    if ( mw && mw->inherits( "MainWindow" ) ) {
	QWidget *hv = ((MainWindow*)mw)->objectHierarchy();
	QObjectList *ol = hv->queryList( "HierarchyList" );
	hierarchy = (HierarchyList*)ol->at( 0 );
	delete ol;
    }
}

DesignerWidgetListInterfaceImpl::~DesignerWidgetListInterfaceImpl()
{
    delete listIterator;
}

QUnknownInterface *DesignerWidgetListInterfaceImpl::queryInterface( const QGuid& guid )
{
    return appIface->queryInterface( guid );
}

unsigned long DesignerWidgetListInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerWidgetListInterfaceImpl::release()
{
    if ( !--ref ) {
	appIface->release();
	delete this;
	return 0;
    }

    return ref;
}

uint DesignerWidgetListInterfaceImpl::count() const
{
    uint c = 0;
    QListViewItemIterator it( hierarchy );
    while ( it.current() ) {
	++it;
	++c;
    }

    return c;
}

DesignerWidgetInterface* DesignerWidgetListInterfaceImpl::reset()
{
    delete listIterator;
    listIterator = new QListViewItemIterator( hierarchy );

    DesignerWidgetInterface* iface = new DesignerWidgetInterfaceImpl( (HierarchyItem*)listIterator->current(), appIface );
    iface->addRef();

    return iface;
}

DesignerWidgetInterface* DesignerWidgetListInterfaceImpl::current()
{
    if ( !listIterator )
	return reset();

    DesignerWidgetInterface* iface = new DesignerWidgetInterfaceImpl( (HierarchyItem*)listIterator->current(), appIface );
    iface->addRef();

    return iface;
}

DesignerWidgetInterface* DesignerWidgetListInterfaceImpl::next()
{
    if ( !listIterator )
	reset();

    ++(*listIterator);
    return current();
}

DesignerWidgetInterface* DesignerWidgetListInterfaceImpl::prev()
{
    if ( !listIterator )
	reset();

    --(*listIterator);
    return current();
}

void DesignerWidgetListInterfaceImpl::selectAll() const
{
    qDebug( "TODO: DesignerWidgetListInterfaceImpl::selectAll" );
}

void DesignerWidgetListInterfaceImpl::removeAll() const
{
    qDebug( "TODO: DesignerWidgetListInterfaceImpl::removeAll" );
}

/*
 * DesignerWidgetInterface implementation
 */
DesignerWidgetInterfaceImpl::DesignerWidgetInterfaceImpl( HierarchyItem *i, QUnknownInterface *ai )
: DesignerWidgetInterface(), item( i ), appIface( ai )
{
    appIface->addRef();

    if ( !item ) {
	QWidget *mw = qApp ? qApp->mainWidget() : 0;
	HierarchyList *hl;
	if ( mw && mw->inherits( "MainWindow" ) ) {
	    QWidget *hv = ((MainWindow*)mw)->objectHierarchy();
	    QObjectList *ol = hv->queryList( "HierarchyList" );
	    hl = (HierarchyList*)ol->at( 0 );
	    delete ol;
	}
	if ( hl )
	    item = (HierarchyItem*)hl->currentItem();
    }
}

QUnknownInterface *DesignerWidgetInterfaceImpl::queryInterface( const QGuid& guid )
{
    return appIface->queryInterface( guid );
}

unsigned long DesignerWidgetInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerWidgetInterfaceImpl::release()
{
    if ( !--ref ) {
	appIface->release();
	delete this;
	return 0;
    }

    return ref;
}

bool DesignerWidgetInterfaceImpl::setProperty( const QCString &p , const QVariant &v )
{
    QWidget *w;
    if ( item && ( w = item->widget() ) )
	return w->setProperty( p, v );
    return FALSE;
}

QVariant DesignerWidgetInterfaceImpl::property( const QCString &p )
{
    QWidget *w;
    if ( item && ( w = item->widget() ) )
	return w->property( p );
    return QVariant();
}

void DesignerWidgetInterfaceImpl::setSelected( bool sel )
{
    if ( item )
	item->setSelected( sel );
}

bool DesignerWidgetInterfaceImpl::selected() const
{
    return item ? item->isSelected() : FALSE;
}

void DesignerWidgetInterfaceImpl::remove()
{
    qDebug( "TODO: DesignerWidgetInterfaceImpl::remove" );
}
