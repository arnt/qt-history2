/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/


void MainWindow::changeProperties()
{
    if ( !dlgProperties )
	dlgProperties = new ChangeProperties( this, 0, FALSE );
    dlgProperties->setControl( container );    
    dlgProperties->show();
}

void MainWindow::clearControl()
{
    container->clear();
    logSignals->clear();
    logProperties->clear();
    bool hasControl = !container->isNull();
    actionContainerClear->setEnabled( hasControl );
    actionControlProperties->setEnabled( hasControl );
    actionControlMethods->setEnabled( hasControl );
    actionControlInfo->setEnabled( hasControl );
    actionContainerSet->setEnabled( !hasControl );
    if ( dlgInvoke && !hasControl )
	dlgInvoke->setControl( 0 );
    if ( dlgProperties && !hasControl )
	dlgProperties->setControl( 0 );
}

void MainWindow::containerProperties()
{
    if ( !dlgAmbient ) {
	dlgAmbient = new AmbientProperties( this, 0, FALSE );
	dlgAmbient->setControl( container );
    }
    dlgAmbient->show();
}

void MainWindow::invokeMethods()
{
    if ( !dlgInvoke )
	dlgInvoke = new InvokeMethod( this, 0, FALSE );
    dlgInvoke->setControl( container );
    dlgInvoke->show();    
}

void MainWindow::logPropertyChanged( const QString &prop )
{
    logProperties->append( container->caption() + ": Property Change: " + prop );    
}

void MainWindow::logSignal( const QString &signal, int, void * )
{
    logSignals->append( container->caption() + ": " + signal );
}

void MainWindow::setControl()
{
    QActiveXSelect select( this, 0, TRUE );
    if ( select.exec() ) {
	container->setControl( select.selectedControl() );
	logSignals->clear();
	logProperties->clear();
    }
    bool hasControl = !container->isNull();
    actionControlProperties->setEnabled( hasControl );
    actionControlMethods->setEnabled( hasControl );
    actionControlInfo->setEnabled( hasControl );
 
    actionContainerSet->setEnabled( !hasControl );
    actionContainerClear->setEnabled( hasControl );    
    if ( hasControl ) {
	if ( dlgInvoke )
	    dlgInvoke->setControl( container );
	if ( dlgProperties )
	    dlgProperties->setControl( container );
    }
    connect( container, SIGNAL(textChanged(const QString&)), logDebug, SLOT(append(const QString&)) );
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
}

void MainWindow::destroy()
{
    qInstallMsgHandler( oldDebugHandler );
    debuglog = 0;    
}

void MainWindow::controlInfo()
{
    ControlInfo info( this, 0, TRUE );
    info.setControl( container );
    info.exec();
}
