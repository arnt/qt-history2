/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <oaidl.h>
#include "../../shared/types.h"

void MainWindow::changeProperties()
{
    QActiveX *container = 0;
    if ( workspace->activeWindow() )
	container = (QActiveX*)workspace->activeWindow()->qt_cast("QActiveX");

    if ( !dlgProperties )
	dlgProperties = new ChangeProperties( this, 0, FALSE );
    dlgProperties->setControl( container );
    dlgProperties->show();
}

void MainWindow::clearControl()
{
    QActiveX *container = 0;
    if ( workspace->activeWindow() )
	container = (QActiveX*)workspace->activeWindow()->qt_cast("QActiveX");
    if ( container )
	container->clear();
    updateGUI();
}

void MainWindow::containerProperties()
{
    if ( !dlgAmbient ) {
	dlgAmbient = new AmbientProperties( this, 0, FALSE );
	dlgAmbient->setControl( workspace );
    }
    dlgAmbient->show();
}

void MainWindow::invokeMethods()
{
    QActiveX *container = 0;
    if ( workspace->activeWindow() )
	container = (QActiveX*)workspace->activeWindow()->qt_cast("QActiveX");

    if ( !dlgInvoke )
	dlgInvoke = new InvokeMethod( this, 0, FALSE );
    dlgInvoke->setControl( container );
    dlgInvoke->show();    
}

void MainWindow::logPropertyChanged( const QString &prop )
{
    QActiveX *container = (QActiveX*)((QObject*)sender())->qt_cast("QActiveX");
    if ( !container )
	return;

    logProperties->append( container->caption() + ": Property Change: " + prop );    
}

void MainWindow::logSignal( const QString &signal, int argc, void *argv )
{
    QActiveX *container = (QActiveX*)((QObject*)sender())->qt_cast("QActiveX");
    if ( !container )
	return;

    QString paramlist;
    VARIANT *params = (VARIANT*)argv;
    for ( int a = argc-1; a >= 0; --a ) {
	if ( a == argc-1 )
	    paramlist = " - {";
	QVariant qvar = VARIANTToQVariant( params[a] );
	paramlist += " " + qvar.toString();
	if ( a > 0 )
	    paramlist += ",";
	else
	    paramlist += " ";
    }
    if ( argc )
	paramlist += "}";
    logSignals->append( container->caption() + ": " + signal + paramlist );
}

void MainWindow::setControl()
{
    QActiveX *container = 0;
    if ( workspace->activeWindow() )
	container = (QActiveX*)workspace->activeWindow()->qt_cast("QActiveX");
    if ( !container )
	return;

    QActiveXSelect select( this, 0, TRUE );
    if ( select.exec() ) {
	container->setControl( select.selectedControl() );
    }
    updateGUI();
}

static QTextEdit *debuglog = 0;

static void redirectDebugOutput( QtMsgType type, const char*msg )
{
    debuglog->append( msg );
}

void MainWindow::init()
{
    dlgInvoke = 0;
    dlgProperties = 0;
    dlgAmbient = 0;
    debuglog = logDebug;
    oldDebugHandler = qInstallMsgHandler( redirectDebugOutput );
    QHBoxLayout *layout = new QHBoxLayout( Workbase );
    workspace = new QWorkspace( Workbase );
    layout->addWidget( workspace );
    connect( workspace, SIGNAL(windowActivated(QWidget*)), this, SLOT(windowActivated(QWidget*)) );
}

void MainWindow::destroy()
{
    qInstallMsgHandler( oldDebugHandler );
    debuglog = 0;    
}

void MainWindow::controlInfo()
{
    QActiveX *container = 0;
    if ( workspace->activeWindow() )
	container = (QActiveX*)workspace->activeWindow()->qt_cast("QActiveX");
    if ( !container )
	return;

    ControlInfo info( this, 0, TRUE );
    info.setControl( container );
    info.exec();
}

void MainWindow::fileLoad()
{
    QString fname = QFileDialog::getOpenFileName( QString::null, "*.qax", this );
    if ( fname.isEmpty() )
	return;

    QFile file( fname );
    if ( !file.open( IO_ReadOnly ) ) {
	QMessageBox::information( this, "Error Loading File", QString("The file could not be opened for reading.\n%1").arg(fname) );
	return;
    }

    QActiveX *container = new QActiveX( workspace );
    
    QDataStream d( &file );
    d >> *container;

    container->show();

    updateGUI();
}

void MainWindow::fileSave()
{
    QActiveX *container = 0;
    if ( workspace->activeWindow() )
	container = (QActiveX*)workspace->activeWindow()->qt_cast("QActiveX");
    if ( !container )
	return;

    QString fname = QFileDialog::getSaveFileName( QString::null, "*.qax", this );
    if ( fname.isEmpty() )
	return;

    QFile file( fname );
    if ( !file.open( IO_WriteOnly ) ) {
	QMessageBox::information( this, "Error Saving File", QString("The file could not be opened for writing.\n%1").arg(fname) );
	return;
    }
    QDataStream d( &file );
    d << *container;
}

void MainWindow::updateGUI()
{
    QActiveX *container = 0;
    if ( workspace->activeWindow() )
	container = (QActiveX*)workspace->activeWindow()->qt_cast("QActiveX");

    bool hasControl = container && !container->isNull();
    actionFileNew->setEnabled( TRUE );
    actionFileLoad->setEnabled( TRUE );
    actionFileSave->setEnabled( hasControl );
    actionContainerSet->setEnabled( container != 0 );
    actionContainerClear->setEnabled( hasControl );
    actionControlProperties->setEnabled( hasControl );
    actionControlMethods->setEnabled( hasControl );
    actionControlInfo->setEnabled( hasControl );
    if ( dlgInvoke )
	dlgInvoke->setControl( hasControl ? container : 0 );
    if ( dlgProperties )
	dlgProperties->setControl( hasControl ? container : 0 );

    QWidgetList list = workspace->windowList();
    QWidgetListIt it( list );
    while ( it.current() ) {
	QWidget *container = it.current();

	container->disconnect( SIGNAL(signal(const QString&, int, void*)) );
	if ( actionLogSignals->isOn() )
	    connect( container, SIGNAL(signal(const QString&, int, void*)), this, SLOT(logSignal(const QString&, int, void*)) );

	container->disconnect( SIGNAL(propertyChanged(const QString&)) );
	if ( actionLogProperties->isOn() ) 
	    connect( container, SIGNAL(propertyChanged(const QString&)), this, SLOT(logPropertyChanged(const QString&)) );
	container->blockSignals( actionFreezeEvents->isOn() );

	++it;
    }
}


void MainWindow::fileNew()
{
    QActiveXSelect select( this, 0, TRUE );
    if ( select.exec() ) {
	QActiveX *container = new QActiveX( select.selectedControl(), workspace, 0, WDestructiveClose );
	container->show();	
    }
    updateGUI();
}


void MainWindow::windowActivated( QWidget *window )
{
    updateGUI();
}
